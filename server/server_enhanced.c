#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <semaphore.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define LOG_FILE "chat.log"
#define USERS_FILE "users.txt"
#define MAX_ROOMS 5
#define ROOM_NAME_LEN 30
#define SHM_SIZE 4096
#define MAX_RECENT_MESSAGES 20
#define MQ_NAME "/netchat_queue"
#define MAX_MQ_MESSAGES 10

/* ========= SHARED MEMORY STRUCTURE ========= */
typedef struct {
    char messages[MAX_RECENT_MESSAGES][BUFFER_SIZE];
    int message_count;
    int write_index;
    pthread_mutex_t shm_lock;
} SharedMessageBuffer;

/* ========= MESSAGE QUEUE STRUCTURE ========= */
typedef struct {
    char username[50];
    char message[BUFFER_SIZE];
    time_t timestamp;
    int priority;  // 0 = normal, 1 = urgent
} QueuedMessage;

typedef struct {
    int fd;
    char username[50];
    char password[50];
    int authenticated;
    char room[ROOM_NAME_LEN];
    pid_t process_id;  // For process forking
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t lock;
FILE *log_file;
int server_fd_global;
volatile sig_atomic_t server_running = 1;

/* Shared memory variables */
int shm_id;
SharedMessageBuffer *shm_buffer = NULL;

/* Message queue */
mqd_t message_queue;

/* Semaphore for connection control */
sem_t *connection_sem;

/* ========= SHARED MEMORY FUNCTIONS ========= */

/* Initialize shared memory */
void init_shared_memory() {
    key_t key = ftok("server.c", 65);
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    
    if (shm_id < 0) {
        perror("shmget failed");
        exit(1);
    }
    
    shm_buffer = (SharedMessageBuffer *)shmat(shm_id, NULL, 0);
    if (shm_buffer == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    /* Initialize shared memory structure */
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm_buffer->shm_lock, &attr);
    
    shm_buffer->message_count = 0;
    shm_buffer->write_index = 0;
    
    printf("[IPC]: Shared memory initialized (ID: %d)\n", shm_id);
}

/* Write message to shared memory */
void write_to_shared_memory(const char *message) {
    if (shm_buffer == NULL) return;
    
    pthread_mutex_lock(&shm_buffer->shm_lock);
    
    strncpy(shm_buffer->messages[shm_buffer->write_index], message, BUFFER_SIZE - 1);
    shm_buffer->write_index = (shm_buffer->write_index + 1) % MAX_RECENT_MESSAGES;
    
    if (shm_buffer->message_count < MAX_RECENT_MESSAGES) {
        shm_buffer->message_count++;
    }
    
    pthread_mutex_unlock(&shm_buffer->shm_lock);
}

/* Cleanup shared memory */
void cleanup_shared_memory() {
    if (shm_buffer != NULL) {
        pthread_mutex_destroy(&shm_buffer->shm_lock);
        shmdt(shm_buffer);
        shmctl(shm_id, IPC_RMID, NULL);
        printf("[IPC]: Shared memory cleaned up\n");
    }
}

/* ========= MESSAGE QUEUE FUNCTIONS ========= */

/* Initialize message queue */
void init_message_queue() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MQ_MESSAGES;
    attr.mq_msgsize = sizeof(QueuedMessage);
    attr.mq_curmsgs = 0;
    
    message_queue = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (message_queue == (mqd_t)-1) {
        perror("mq_open failed");
        exit(1);
    }
    
    printf("[IPC]: Message queue initialized\n");
}

/* Queue a message for offline delivery */
void queue_offline_message(const char *username, const char *message, int priority) {
    QueuedMessage qmsg;
    strncpy(qmsg.username, username, sizeof(qmsg.username) - 1);
    strncpy(qmsg.message, message, sizeof(qmsg.message) - 1);
    qmsg.timestamp = time(NULL);
    qmsg.priority = priority;
    
    if (mq_send(message_queue, (char *)&qmsg, sizeof(QueuedMessage), priority) == -1) {
        perror("mq_send failed");
    } else {
        printf("[MQ]: Queued message for %s (priority: %d)\n", username, priority);
    }
}

/* Deliver queued messages to user */
void deliver_queued_messages(int client_fd, const char *username) {
    QueuedMessage qmsg;
    unsigned int prio;
    struct mq_attr attr;
    
    mq_getattr(message_queue, &attr);
    
    while (attr.mq_curmsgs > 0) {
        ssize_t bytes_read = mq_receive(message_queue, (char *)&qmsg, sizeof(QueuedMessage), &prio);
        
        if (bytes_read >= 0 && strcmp(qmsg.username, username) == 0) {
            char delivery[BUFFER_SIZE + 100];
            snprintf(delivery, sizeof(delivery), "[Offline Message]: %s\n", qmsg.message);
            send(client_fd, delivery, strlen(delivery), 0);
        }
        
        mq_getattr(message_queue, &attr);
    }
}

/* Cleanup message queue */
void cleanup_message_queue() {
    mq_close(message_queue);
    mq_unlink(MQ_NAME);
    printf("[IPC]: Message queue cleaned up\n");
}

/* ========= SEMAPHORE FUNCTIONS ========= */

/* Initialize semaphore */
void init_semaphore() {
    connection_sem = sem_open("/netchat_sem", O_CREAT, 0666, MAX_CLIENTS);
    if (connection_sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
    printf("[SYNC]: Semaphore initialized (max connections: %d)\n", MAX_CLIENTS);
}

/* Cleanup semaphore */
void cleanup_semaphore() {
    sem_close(connection_sem);
    sem_unlink("/netchat_sem");
    printf("[SYNC]: Semaphore cleaned up\n");
}

/* ========= EXISTING FUNCTIONS (with enhancements) ========= */

void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", t);
}

void log_message(const char *message) {
    pthread_mutex_lock(&lock);
    if (log_file) {
        char timestamp[20];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(log_file, "%s %s", timestamp, message);
        fflush(log_file);
    }
    
    /* Also write to shared memory */
    write_to_shared_memory(message);
    
    pthread_mutex_unlock(&lock);
}

void broadcast(char *message, int sender_fd) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd != sender_fd) {
            send(clients[i].fd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

void broadcast_all(char *message) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        send(clients[i].fd, message, strlen(message), 0);
    }
    pthread_mutex_unlock(&lock);
}

void broadcast_room(char *message, int sender_fd, const char *room) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd != sender_fd && 
            strcmp(clients[i].room, room) == 0) {
            send(clients[i].fd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

int send_private_message(const char *target_username, const char *message, const char *sender) {
    pthread_mutex_lock(&lock);
    int found = 0;
    
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, target_username) == 0) {
            char pm[BUFFER_SIZE + 100];
            snprintf(pm, sizeof(pm), "[PM from %s]: %s", sender, message);
            send(clients[i].fd, pm, strlen(pm), 0);
            found = 1;
            break;
        }
    }
    
    /* If user not found, queue message for offline delivery */
    if (!found) {
        char offline_msg[BUFFER_SIZE];
        snprintf(offline_msg, sizeof(offline_msg), "From %s: %s", sender, message);
        queue_offline_message(target_username, offline_msg, 1);  // Priority = 1 for PMs
    }
    
    pthread_mutex_unlock(&lock);
    return found;
}

int register_user(const char *username, const char *password) {
    char clean_user[50];
    char clean_pass[50];
    
    strncpy(clean_user, username, sizeof(clean_user) - 1);
    clean_user[sizeof(clean_user) - 1] = '\0';
    clean_user[strcspn(clean_user, "\n\r:")] = '\0';
    
    strncpy(clean_pass, password, sizeof(clean_pass) - 1);
    clean_pass[sizeof(clean_pass) - 1] = '\0';
    clean_pass[strcspn(clean_pass, "\n\r:")] = '\0';
    
    if (strlen(clean_user) == 0 || strlen(clean_pass) == 0) {
        return 0;
    }
    
    FILE *file = fopen(USERS_FILE, "a");
    if (!file) {
        perror("Failed to open users file");
        return 0;
    }
    
    fprintf(file, "%s:%s\n", clean_user, clean_pass);
    fclose(file);
    
    char log_msg[BUFFER_SIZE];
    snprintf(log_msg, sizeof(log_msg), "[Server]: New user registered: %s\n", clean_user);
    log_message(log_msg);
    printf("%s", log_msg);
    
    return 1;
}

int authenticate_user(const char *username, const char *password) {
    char clean_user[50];
    char clean_pass[50];
    
    strncpy(clean_user, username, sizeof(clean_user) - 1);
    clean_user[sizeof(clean_user) - 1] = '\0';
    clean_user[strcspn(clean_user, "\n\r:")] = '\0';
    
    strncpy(clean_pass, password, sizeof(clean_pass) - 1);
    clean_pass[sizeof(clean_pass) - 1] = '\0';
    clean_pass[strcspn(clean_pass, "\n\r:")] = '\0';
    
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        return register_user(clean_user, clean_pass);
    }
    
    char line[256];
    char stored_user[50];
    char stored_pass[50];
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n\r")] = 0;
        
        if (sscanf(line, "%49[^:]:%49s", stored_user, stored_pass) == 2) {
            if (strcmp(stored_user, clean_user) == 0) {
                fclose(file);
                if (strcmp(stored_pass, clean_pass) == 0) {
                    return 1;
                } else {
                    return -1;
                }
            }
        }
    }
    
    fclose(file);
    return register_user(clean_user, clean_pass);
}

void handle_shutdown(int sig) {
    (void)sig;
    server_running = 0;
    
    char *msg = "\n[Server]: Server is shutting down. Goodbye!\n";
    broadcast_all(msg);
    log_message(msg);
    
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        close(clients[i].fd);
        if (clients[i].process_id > 0) {
            kill(clients[i].process_id, SIGTERM);
        }
    }
    pthread_mutex_unlock(&lock);
    
    if (log_file) {
        fclose(log_file);
    }
    
    /* Cleanup IPC resources */
    cleanup_shared_memory();
    cleanup_message_queue();
    cleanup_semaphore();
    
    close(server_fd_global);
    pthread_mutex_destroy(&lock);
    
    printf("\n[Server]: Shutdown complete. All IPC resources cleaned up.\n");
    exit(0);
}

/* ========= PROCESS FORKING - Handle client in separate process ========= */
void handle_client_process(int client_fd) {
    char buffer[BUFFER_SIZE];
    char username[50];
    char password[50];
    char message[BUFFER_SIZE + 100];
    int bytes_read;
    int client_index = -1;

    /* Receive username */
    int idx = 0;
    char c;
    while (idx < (int)sizeof(username) - 1) {
        bytes_read = recv(client_fd, &c, 1, 0);
        if (bytes_read <= 0) {
            close(client_fd);
            exit(0);
        }
        if (c == '\n') break;
        username[idx++] = c;
    }
    username[idx] = '\0';

    /* Receive password */
    idx = 0;
    while (idx < (int)sizeof(password) - 1) {
        bytes_read = recv(client_fd, &c, 1, 0);
        if (bytes_read <= 0) {
            close(client_fd);
            exit(0);
        }
        if (c == '\n') break;
        password[idx++] = c;
    }
    password[idx] = '\0';
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        char *err = "Error: Username and password cannot be empty.\n";
        send(client_fd, err, strlen(err), 0);
        close(client_fd);
        exit(0);
    }

    /* Authenticate */
    int auth_result = authenticate_user(username, password);
    if (auth_result != 1) {
        char *auth_fail = (auth_result == -1) ? 
            "ERROR: Wrong password. Disconnecting...\n" :
            "ERROR: Authentication failed. Disconnecting...\n";
        send(client_fd, auth_fail, strlen(auth_fail), 0);
        close(client_fd);
        exit(0);
    }

    /* Send welcome message */
    char welcome[BUFFER_SIZE * 2];
    snprintf(welcome, sizeof(welcome),
        "\n╔════════════════════════════════════════════════════════════════╗\n"
        "║           🎉 WELCOME TO NETCHAT (ENHANCED)! 🎉               ║\n"
        "╠════════════════════════════════════════════════════════════════╣\n"
        "║  ✅ Authentication successful!                                ║\n"
        "║  🔄 Running in separate process (PID: %d)                     ║\n"
        "║  💾 Shared memory enabled for message history                ║\n"
        "║  📨 Message queue active for offline delivery                ║\n"
        "║  🔐 Semaphore controlling concurrent connections             ║\n"
        "╚════════════════════════════════════════════════════════════════╝\n\n",
        getpid());
    send(client_fd, welcome, strlen(welcome), 0);

    /* Store user info */
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd == client_fd) {
            strncpy(clients[i].username, username, sizeof(clients[i].username) - 1);
            strncpy(clients[i].password, password, sizeof(clients[i].password) - 1);
            clients[i].authenticated = 1;
            strcpy(clients[i].room, "general");
            clients[i].process_id = getpid();
            client_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    /* Deliver queued messages */
    deliver_queued_messages(client_fd, username);

    /* Join notification */
    snprintf(message, sizeof(message), "[Server]: %s has joined #general (Process: %d)\n", username, getpid());
    printf("%s", message);
    log_message(message);
    broadcast_room(message, -1, "general");

    /* Message handling loop */
    while ((bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        
        /* Command handling */
        if (strncmp(buffer, "/pm ", 4) == 0) {
            char *cmd = buffer + 4;
            char *space = strchr(cmd, ' ');
            if (space) {
                *space = '\0';
                char *target_user = cmd;
                char *pm_msg = space + 1;
                pm_msg[strcspn(pm_msg, "\n")] = 0;
                
                if (send_private_message(target_user, pm_msg, username)) {
                    char confirm[BUFFER_SIZE];
                    snprintf(confirm, sizeof(confirm), "[PM to %s]: %s\n", target_user, pm_msg);
                    send(client_fd, confirm, strlen(confirm), 0);
                } else {
                    char *queued = "[Server]: User offline. Message queued for delivery.\n";
                    send(client_fd, queued, strlen(queued), 0);
                }
            }
        }
        else if (strncmp(buffer, "/recent", 7) == 0) {
            /* Show recent messages from shared memory */
            if (shm_buffer != NULL) {
                pthread_mutex_lock(&shm_buffer->shm_lock);
                char recent[BUFFER_SIZE * 2] = "\n[Recent Messages from Shared Memory]:\n";
                int start = (shm_buffer->write_index - shm_buffer->message_count + MAX_RECENT_MESSAGES) % MAX_RECENT_MESSAGES;
                for (int i = 0; i < shm_buffer->message_count; i++) {
                    int idx = (start + i) % MAX_RECENT_MESSAGES;
                    strcat(recent, shm_buffer->messages[idx]);
                }
                pthread_mutex_unlock(&shm_buffer->shm_lock);
                send(client_fd, recent, strlen(recent), 0);
            }
        }
        else {
            /* Regular message */
            char timestamp[20];
            get_timestamp(timestamp, sizeof(timestamp));
            
            pthread_mutex_lock(&lock);
            char current_room[ROOM_NAME_LEN];
            strcpy(current_room, clients[client_index].room);
            pthread_mutex_unlock(&lock);
            
            snprintf(message, sizeof(message), "%s [#%s] %s: %s", 
                     timestamp, current_room, username, buffer);
            
            printf("%s", message);
            log_message(message);
            broadcast_room(message, client_fd, current_room);
        }
    }

    /* Cleanup */
    pthread_mutex_lock(&lock);
    char leaving_user[50];
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd == client_fd) {
            strncpy(leaving_user, clients[i].username, sizeof(leaving_user) - 1);
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    snprintf(message, sizeof(message), "[Server]: %s has disconnected (Process: %d exiting)\n", leaving_user, getpid());
    printf("%s", message);
    log_message(message);
    broadcast_all(message);

    close(client_fd);
    exit(0);  // Exit child process
}

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    pid_t pid;

    /* Initialize mutex and log file */
    pthread_mutex_init(&lock, NULL);
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Failed to open log file");
    }

    /* Initialize IPC resources */
    init_shared_memory();
    init_message_queue();
    init_semaphore();

    /* Setup signal handler */
    signal(SIGINT, handle_shutdown);
    signal(SIGCHLD, SIG_IGN);  // Prevent zombie processes

    server_fd_global = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_global < 0) {
        perror("Socket failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd_global, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd_global, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_fd_global, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║          NETCHAT SERVER (ENHANCED) - RUNNING                  ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Port: %d                                                     ║\n", PORT);
    printf("║  Max Clients: %d                                              ║\n", MAX_CLIENTS);
    printf("║  💾 Shared Memory: ENABLED                                    ║\n");
    printf("║  📨 Message Queue: ENABLED                                    ║\n");
    printf("║  🔄 Process Forking: ENABLED                                  ║\n");
    printf("║  🚦 Semaphore Control: ENABLED                                ║\n");
    printf("║  Press Ctrl+C for graceful shutdown                          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    log_message("[Server]: Enhanced server started with IPC features\n");

    while (server_running) {
        /* Wait for semaphore (connection slot) */
        if (sem_wait(connection_sem) == -1) {
            perror("sem_wait failed");
            continue;
        }

        client_fd = accept(server_fd_global, NULL, NULL);
        
        if (client_fd < 0) {
            if (server_running) {
                perror("Accept failed");
            }
            sem_post(connection_sem);  // Release semaphore
            continue;
        }

        pthread_mutex_lock(&lock);
        
        if (client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&lock);
            char *full_msg = "Server full. Try again later.\n";
            send(client_fd, full_msg, strlen(full_msg), 0);
            close(client_fd);
            sem_post(connection_sem);
            continue;
        }
        
        clients[client_count].fd = client_fd;
        clients[client_count].authenticated = 0;
        clients[client_count].process_id = 0;
        strcpy(clients[client_count].room, "general");
        client_count++;
        
        pthread_mutex_unlock(&lock);

        /* Fork a child process to handle client */
        pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            close(client_fd);
            sem_post(connection_sem);
        }
        else if (pid == 0) {
            /* Child process */
            close(server_fd_global);  // Child doesn't need server socket
            handle_client_process(client_fd);
            /* Never reaches here - handle_client_process calls exit() */
        }
        else {
            /* Parent process */
            printf("[Server]: Forked child process %d for new client\n", pid);
            
            /* Update client's process ID */
            pthread_mutex_lock(&lock);
            for (int i = 0; i < client_count; i++) {
                if (clients[i].fd == client_fd) {
                    clients[i].process_id = pid;
                    break;
                }
            }
            pthread_mutex_unlock(&lock);
            
            /* Parent doesn't need client socket */
            close(client_fd);
            
            /* Release semaphore when child exits */
            sem_post(connection_sem);
        }
    }

    /* Wait for all child processes */
    while (wait(NULL) > 0);

    cleanup_shared_memory();
    cleanup_message_queue();
    cleanup_semaphore();
    
    close(server_fd_global);
    if (log_file) {
        fclose(log_file);
    }
    pthread_mutex_destroy(&lock);
    
    return 0;
}
