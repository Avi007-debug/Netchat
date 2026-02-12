#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <mqueue.h>
#include <semaphore.h>

#define PORT 5555
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define LOG_FILE "chat.log"
#define USERS_FILE "users.txt"
#define MAX_ROOMS 5
#define ROOM_NAME_LEN 30
#define SHM_SIZE 131072  // 128KB - increased for broadcast queue
#define MAX_RECENT_MESSAGES 20
#define MQ_NAME "/netchat_queue"
#define MAX_MQ_MESSAGES 10
#define MAX_BROADCAST_QUEUE 50

/* ========= BROADCAST MESSAGE STRUCTURE ========= */
typedef struct {
    char message[BUFFER_SIZE];
    int sender_fd;
    char room[ROOM_NAME_LEN];
    int broadcast_type;  // 0=room, 1=all, 2=none
} BroadcastMessage;

/* ========= SHARED MEMORY STRUCTURE ========= */
typedef struct {
    int fd;
    char username[50];
    char password[50];
    int authenticated;
    char room[ROOM_NAME_LEN];
    pid_t process_id;
} SharedClient;

typedef struct {
    char messages[MAX_RECENT_MESSAGES][BUFFER_SIZE];
    int message_count;
    int write_index;
    pthread_mutex_t shm_lock;
    SharedClient clients[MAX_CLIENTS];
    int client_count;
    BroadcastMessage broadcast_queue[MAX_BROADCAST_QUEUE];
    int broadcast_read_idx;
    int broadcast_write_idx;
    int broadcast_count;
    pid_t parent_pid;
} SharedMessageBuffer;

/* ========= MESSAGE QUEUE STRUCTURE ========= */
typedef struct {
    char username[50];
    char message[BUFFER_SIZE];
    time_t timestamp;
    int priority;  // 0 = normal, 1 = urgent
} QueuedMessage;

pthread_mutex_t lock;
FILE *log_file;
int server_fd_global;
volatile sig_atomic_t server_running = 1;
volatile sig_atomic_t broadcast_pending = 0;
pid_t parent_pid_global = 0;

/* Forward declarations */
void handle_broadcast_signal(int sig);

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
    /* Use absolute path to ensure ftok works regardless of CWD */
    key_t key = ftok("/tmp/netchat_key", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    
    printf("[DEBUG] ftok returned key=%d\n", (int)key);
    fflush(stdout);
    
    /* Try to create new, if exists just get it */
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    int is_new = (shm_id >= 0);
    
    printf("[DEBUG] First shmget: shm_id=%d, is_new=%d\n", shm_id, is_new);
    fflush(stdout);
    
    /* If already exists, just get it */
    if (!is_new) {
        shm_id = shmget(key, SHM_SIZE, 0666);
        printf("[DEBUG] Second shmget: shm_id=%d\n", shm_id);
        fflush(stdout);
    }
    
    if (shm_id < 0) {
        perror("shmget failed");
        exit(1);
    }
    
    shm_buffer = (SharedMessageBuffer *)shmat(shm_id, NULL, 0);
    if (shm_buffer == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    printf("[DEBUG] shmat succeeded, shm_buffer=%p\n", (void*)shm_buffer);
    fflush(stdout);
    
    /* Only initialize mutex if newly created */
    if (is_new) {
        printf("[DEBUG] Zeroing shared memory\n");
        fflush(stdout);
        
        /* Zero out the structure first */
        memset(shm_buffer, 0, sizeof(SharedMessageBuffer));
        
        printf("[DEBUG] Initializing mutex attr\n");
        fflush(stdout);
        
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
            perror("setpshared failed");
            exit(1);
        }
        
        printf("[DEBUG] Calling pthread_mutex_init\n");
        fflush(stdout);
        
        if (pthread_mutex_init(&shm_buffer->shm_lock, &attr) != 0) {
            perror("mutex_init failed");
            exit(1);
        }
        pthread_mutexattr_destroy(&attr);
        
        shm_buffer->message_count = 0;
        shm_buffer->write_index = 0;
        shm_buffer->broadcast_read_idx = 0;
        shm_buffer->broadcast_write_idx = 0;
        shm_buffer->broadcast_count = 0;
        shm_buffer->client_count = 0;
        printf("[IPC]: New shared memory created (ID: %d)\n", shm_id);
    } else {
        printf("[IPC]: Using existing shared memory (ID: %d)\n", shm_id);
    }
}

/* Queue a message for broadcasting by parent process */
void queue_broadcast(const char *message, int sender_fd, const char *room, int broadcast_type) {
    pthread_mutex_lock(&shm_buffer->shm_lock);
    
    if (shm_buffer->broadcast_count < MAX_BROADCAST_QUEUE) {
        BroadcastMessage *msg = &shm_buffer->broadcast_queue[shm_buffer->broadcast_write_idx];
        strncpy(msg->message, message, BUFFER_SIZE - 1);
        msg->message[BUFFER_SIZE - 1] = '\0';
        msg->sender_fd = sender_fd;
        strncpy(msg->room, room, ROOM_NAME_LEN - 1);
        msg->room[ROOM_NAME_LEN - 1] = '\0';
        msg->broadcast_type = broadcast_type;
        
        shm_buffer->broadcast_write_idx = (shm_buffer->broadcast_write_idx + 1) % MAX_BROADCAST_QUEUE;
        shm_buffer->broadcast_count++;
        
        pthread_mutex_unlock(&shm_buffer->shm_lock);
        
        /* Signal parent to process broadcasts */
        if (shm_buffer->parent_pid > 0) {
            kill(shm_buffer->parent_pid, SIGUSR1);
        }
    } else {
        pthread_mutex_unlock(&shm_buffer->shm_lock);
        fprintf(stderr, "[WARNING]: Broadcast queue full, message dropped\n");
    }
}

/* Signal handler in parent to process broadcast queue */
void handle_broadcast_signal(int sig) {
    (void)sig;  // Unused
    broadcast_pending = 1;  // Set flag for main loop
}

/* Process all pending broadcasts (called by parent only) */
void process_broadcasts() {
    if (!shm_buffer) return;  // Safety check
    
    broadcast_pending = 0;  // Clear flag
    
    pthread_mutex_lock(&shm_buffer->shm_lock);
    
    while (shm_buffer->broadcast_count > 0) {
        BroadcastMessage *msg = &shm_buffer->broadcast_queue[shm_buffer->broadcast_read_idx];
        
        /* Broadcast based on type */
        for (int i = 0; i < shm_buffer->client_count; i++) {
            int should_send = 0;
            
            if (msg->broadcast_type == 1) {
                /* Broadcast to all */
                should_send = 1;
            } else if (msg->broadcast_type == 0) {
                /* Broadcast to room (excluding sender) */
                if (shm_buffer->clients[i].fd != msg->sender_fd &&
                    strcmp(shm_buffer->clients[i].room, msg->room) == 0) {
                    should_send = 1;
                }
            }
            
            if (should_send) {
                send(shm_buffer->clients[i].fd, msg->message, strlen(msg->message), 0);
            }
        }
        
        shm_buffer->broadcast_read_idx = (shm_buffer->broadcast_read_idx + 1) % MAX_BROADCAST_QUEUE;
        shm_buffer->broadcast_count--;
    }
    
    pthread_mutex_unlock(&shm_buffer->shm_lock);
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
    
    /* Try to unlink first in case it exists from crashed previous run */
    mq_unlink(MQ_NAME);
    
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
    /* Unlink first in case it exists from crashed previous run */
    sem_unlink("/netchat_sem");
    
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
    if (!log_file) return;  // Safety check
    
    pthread_mutex_lock(&lock);
    if (log_file) {
        char timestamp[20];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(log_file, "%s %s", timestamp, message);
        fflush(log_file);
    }
    pthread_mutex_unlock(&lock);
    
    /* Also write to shared memory (disabled for stability) */
    // write_to_shared_memory(message);
}

void broadcast(char *message, int sender_fd) {
    /* Child process: queue for parent to broadcast */
    if (getpid() != shm_buffer->parent_pid) {
        queue_broadcast(message, sender_fd, "general", 0);
    } else {
        /* Parent process: broadcast directly */
        pthread_mutex_lock(&shm_buffer->shm_lock);
        for (int i = 0; i < shm_buffer->client_count; i++) {
            if (shm_buffer->clients[i].fd != sender_fd) {
                send(shm_buffer->clients[i].fd, message, strlen(message), 0);
            }
        }
        pthread_mutex_unlock(&shm_buffer->shm_lock);
    }
}

void broadcast_all(char *message) {
    /* Child process: queue for parent to broadcast */
    if (getpid() != shm_buffer->parent_pid) {
        queue_broadcast(message, -1, "", 1);
    } else {
        /* Parent process: broadcast directly */
        pthread_mutex_lock(&shm_buffer->shm_lock);
        for (int i = 0; i < shm_buffer->client_count; i++) {
            send(shm_buffer->clients[i].fd, message, strlen(message), 0);
        }
        pthread_mutex_unlock(&shm_buffer->shm_lock);
    }
}

void broadcast_room(char *message, int sender_fd, const char *room) {
    /* Child process: queue for parent to broadcast */
    if (getpid() != shm_buffer->parent_pid) {
        queue_broadcast(message, sender_fd, room, 0);
    } else {
        /* Parent process: broadcast directly */
        pthread_mutex_lock(&shm_buffer->shm_lock);
        for (int i = 0; i < shm_buffer->client_count; i++) {
            if (shm_buffer->clients[i].fd != sender_fd && 
                strcmp(shm_buffer->clients[i].room, room) == 0) {
                send(shm_buffer->clients[i].fd, message, strlen(message), 0);
            }
        }
        pthread_mutex_unlock(&shm_buffer->shm_lock);
    }
}

int send_private_message(const char *target_username, const char *message, const char *sender) {
    pthread_mutex_lock(&shm_buffer->shm_lock);
    int found = 0;
    
    for (int i = 0; i < shm_buffer->client_count; i++) {
        if (strcmp(shm_buffer->clients[i].username, target_username) == 0) {
            char pm[BUFFER_SIZE + 100];
            snprintf(pm, sizeof(pm), "[PM from %s]: %s", sender, message);
            send(shm_buffer->clients[i].fd, pm, strlen(pm), 0);
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
    
    pthread_mutex_unlock(&shm_buffer->shm_lock);
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
    
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║              GRACEFUL SHUTDOWN IN PROGRESS...                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    char *msg = "\n[Server]: Server is shutting down. Goodbye!\n";
    broadcast_all(msg);
    log_message(msg);
    
    printf("[Shutdown]: Broadcasting shutdown message to all clients...\n");
    
    pthread_mutex_lock(&shm_buffer->shm_lock);
    int child_count = 0;
    for (int i = 0; i < shm_buffer->client_count; i++) {
        close(shm_buffer->clients[i].fd);
        if (shm_buffer->clients[i].process_id > 0) {
            kill(shm_buffer->clients[i].process_id, SIGTERM);
            child_count++;
        }
    }
    pthread_mutex_unlock(&shm_buffer->shm_lock);
    
    printf("[Shutdown]: Waiting for %d child processes to exit...\n", child_count);
    
    /* Wait for all child processes */
    while (child_count > 0) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            child_count--;
        } else {
            usleep(100000);  // 100ms
        }
    }
    
    printf("[Shutdown]: All child processes terminated\n");
    
    if (log_file) {
        fclose(log_file);
    }
    
    /* Cleanup IPC resources */
    printf("[Shutdown]: Cleaning up IPC resources...\n");
    cleanup_shared_memory();
    cleanup_message_queue();
    cleanup_semaphore();
    
    close(server_fd_global);
    pthread_mutex_destroy(&lock);
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║         SHUTDOWN COMPLETE - ALL RESOURCES CLEANED UP         ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    exit(0);
}

/* Handle child process termination */
void handle_sigchld(int sig) {
    (void)sig;
    int status;
    pid_t pid;
    
    /* Reap all terminated children */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /* Find and remove client from shared memory */
        pthread_mutex_lock(&shm_buffer->shm_lock);
        for (int i = 0; i < shm_buffer->client_count; i++) {
            if (shm_buffer->clients[i].process_id == pid) {
                /* Close the socket in parent */
                close(shm_buffer->clients[i].fd);
                
                /* Remove this client by shifting others */
                for (int j = i; j < shm_buffer->client_count - 1; j++) {
                    shm_buffer->clients[j] = shm_buffer->clients[j + 1];
                }
                shm_buffer->client_count--;
                break;
            }
        }
        pthread_mutex_unlock(&shm_buffer->shm_lock);
    }
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
    char welcome[BUFFER_SIZE * 3];
    snprintf(welcome, sizeof(welcome),
        "\n╔════════════════════════════════════════════════════════════════╗\n"
        "║           🎉 WELCOME TO NETCHAT (ENHANCED)! 🎉               ║\n"
        "╠════════════════════════════════════════════════════════════════╣\n"
        "║  ✅ Authentication successful!                                ║\n"
        "║  🔄 Running in separate process (PID: %d)                     ║\n"
        "║  💾 Shared memory enabled for message history                ║\n"
        "║  📨 Message queue active for offline delivery                ║\n"
        "║  🔐 Semaphore controlling concurrent connections             ║\n"
        "╚════════════════════════════════════════════════════════════════╝\n\n"
        "╔════════════════════════════════════════════════════════════════╗\n"
        "║                     AVAILABLE COMMANDS                         ║\n"
        "╠════════════════════════════════════════════════════════════════╣\n"
        "║                                                                ║\n"
        "║  💬 MESSAGING:                                                 ║\n"
        "║     • Type normally to send message to current room           ║\n"
        "║     • /pm <user> <message>  - Send private message            ║\n"
        "║                                                                ║\n"
        "║  🏢 ROOMS:                                                     ║\n"
        "║     • /room                 - Show current room               ║\n"
        "║     • /join <roomname>      - Join/create a room              ║\n"
        "║     • /rooms                - List all active rooms           ║\n"
        "║     • /recent               - Show recent messages from memory ║\n"
        "║                                                                ║\n"
        "║  👥 USERS:                                                     ║\n"
        "║     • /users                - List users in current room      ║\n"
        "║                                                                ║\n"
        "║  ℹ️  HELP:                                                      ║\n"
        "║     • /help                 - Show this menu again            ║\n"
        "║                                                                ║\n"
        "╚════════════════════════════════════════════════════════════════╝\n\n",
        getpid());
    send(client_fd, welcome, strlen(welcome), 0);

    /* Store user info */
    pthread_mutex_lock(&shm_buffer->shm_lock);
    for (int i = 0; i < shm_buffer->client_count; i++) {
        if (shm_buffer->clients[i].fd == client_fd) {
            strncpy(shm_buffer->clients[i].username, username, sizeof(shm_buffer->clients[i].username) - 1);
            strncpy(shm_buffer->clients[i].password, password, sizeof(shm_buffer->clients[i].password) - 1);
            shm_buffer->clients[i].authenticated = 1;
            strcpy(shm_buffer->clients[i].room, "general");
            shm_buffer->clients[i].process_id = getpid();
            client_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&shm_buffer->shm_lock);

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
        else if (strncmp(buffer, "/help", 5) == 0 && (buffer[5] == '\n' || buffer[5] == '\0')) {
            /* Show help menu */
            char help_menu[BUFFER_SIZE * 2];
            snprintf(help_menu, sizeof(help_menu),
                "\n╔════════════════════════════════════════════════════════════════╗\n"
                "║                     AVAILABLE COMMANDS                         ║\n"
                "╠════════════════════════════════════════════════════════════════╣\n"
                "║                                                                ║\n"
                "║  💬 MESSAGING:                                                 ║\n"
                "║     • Type normally to send message to current room           ║\n"
                "║     • /pm <user> <message>  - Send private message            ║\n"
                "║                                                                ║\n"
                "║  🏢 ROOMS:                                                     ║\n"
                "║     • /room                 - Show current room               ║\n"
                "║     • /join <roomname>      - Join/create a room              ║\n"
                "║     • /rooms                - List all active rooms           ║\n"
                "║     • /recent               - Show recent messages from memory ║\n"
                "║                                                                ║\n"
                "║  👥 USERS:                                                     ║\n"
                "║     • /users                - List users in current room      ║\n"
                "║                                                                ║\n"
                "║  ℹ️  HELP:                                                      ║\n"
                "║     • /help                 - Show this menu again            ║\n"
                "║                                                                ║\n"
                "╚════════════════════════════════════════════════════════════════╝\n\n");
            send(client_fd, help_menu, strlen(help_menu), 0);
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
        else if (strncmp(buffer, "/join ", 6) == 0) {
            /* Join/create a room */
            char room_name[ROOM_NAME_LEN];
            char *room_str = buffer + 6;
            room_str[strcspn(room_str, "\n")] = 0;
            
            if (strlen(room_str) > 0) {
                pthread_mutex_lock(&shm_buffer->shm_lock);
                if (client_index >= 0) {
                    char old_room[ROOM_NAME_LEN];
                    strcpy(old_room, shm_buffer->clients[client_index].room);
                    strncpy(shm_buffer->clients[client_index].room, room_str, ROOM_NAME_LEN - 1);
                    pthread_mutex_unlock(&shm_buffer->shm_lock);
                    
                    /* Notify room left */
                    char leaving_msg[BUFFER_SIZE];
                    snprintf(leaving_msg, sizeof(leaving_msg), 
                        "[Server]: %s has left #%s\n", username, old_room);
                    broadcast_room(leaving_msg, -1, old_room);
                    
                    /* Notify room joined */
                    char joining_msg[BUFFER_SIZE];
                    snprintf(joining_msg, sizeof(joining_msg), 
                        "[Server]: %s has joined #%s\n", username, room_str);
                    broadcast_room(joining_msg, -1, room_str);
                    
                    /* Confirm to user */
                    char confirm[BUFFER_SIZE];
                    snprintf(confirm, sizeof(confirm), 
                        "[Server]: You are now in room #%s\n", room_str);
                    send(client_fd, confirm, strlen(confirm), 0);
                } else {
                    pthread_mutex_unlock(&shm_buffer->shm_lock);
                }
            } else {
                char *err = "[Server]: Room name cannot be empty.\n";
                send(client_fd, err, strlen(err), 0);
            }
        }
        else if (strncmp(buffer, "/room", 5) == 0 && (buffer[5] == '\n' || buffer[5] == '\0')) {
            /* Show current room */
            pthread_mutex_lock(&shm_buffer->shm_lock);
            char current_room[ROOM_NAME_LEN];
            if (client_index >= 0) {
                strcpy(current_room, shm_buffer->clients[client_index].room);
            } else {
                strcpy(current_room, "unknown");
            }
            pthread_mutex_unlock(&shm_buffer->shm_lock);
            
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), 
                "[Server]: You are currently in room #%s\n", current_room);
            send(client_fd, response, strlen(response), 0);
        }
        else if (strncmp(buffer, "/rooms", 6) == 0 && (buffer[6] == '\n' || buffer[6] == '\0')) {
            /* List all active rooms */
            pthread_mutex_lock(&shm_buffer->shm_lock);
            char rooms_list[BUFFER_SIZE * 2];
            strcpy(rooms_list, "\n[Active Rooms]:\n");
            
            /* Count users per room */
            char room_names[MAX_ROOMS][ROOM_NAME_LEN];
            int room_counts[MAX_ROOMS] = {0};
            int room_count = 0;
            
            for (int i = 0; i < shm_buffer->client_count; i++) {
                int found = 0;
                for (int j = 0; j < room_count; j++) {
                    if (strcmp(room_names[j], shm_buffer->clients[i].room) == 0) {
                        room_counts[j]++;
                        found = 1;
                        break;
                    }
                }
                if (!found && room_count < MAX_ROOMS) {
                    strcpy(room_names[room_count], shm_buffer->clients[i].room);
                    room_counts[room_count]++;
                    room_count++;
                }
            }
            
            pthread_mutex_unlock(&shm_buffer->shm_lock);
            
            /* Display rooms */
            for (int i = 0; i < room_count; i++) {
                char room_info[BUFFER_SIZE];
                snprintf(room_info, sizeof(room_info), 
                    "  • #%s (%d user%s)\n", 
                    room_names[i], 
                    room_counts[i],
                    room_counts[i] != 1 ? "s" : "");
                strcat(rooms_list, room_info);
            }
            
            strcat(rooms_list, "\n");
            send(client_fd, rooms_list, strlen(rooms_list), 0);
        }
        else if (strncmp(buffer, "/users", 6) == 0 && (buffer[6] == '\n' || buffer[6] == '\0')) {
            /* List users in current room */
            pthread_mutex_lock(&shm_buffer->shm_lock);
            char current_room[ROOM_NAME_LEN];
            if (client_index >= 0) {
                strcpy(current_room, shm_buffer->clients[client_index].room);
            } else {
                strcpy(current_room, "general");
            }
            
            char users_list[BUFFER_SIZE * 2];
            snprintf(users_list, sizeof(users_list), 
                "\n[Users in #%s]:\n", current_room);
            
            for (int i = 0; i < shm_buffer->client_count; i++) {
                if (strcmp(shm_buffer->clients[i].room, current_room) == 0) {
                    char user_info[BUFFER_SIZE];
                    snprintf(user_info, sizeof(user_info), 
                        "  • %s\n", shm_buffer->clients[i].username);
                    strcat(users_list, user_info);
                }
            }
            
            pthread_mutex_unlock(&shm_buffer->shm_lock);
            
            strcat(users_list, "\n");
            send(client_fd, users_list, strlen(users_list), 0);
        }
        else {
            /* Regular message */
            char timestamp[20];
            get_timestamp(timestamp, sizeof(timestamp));
            
            pthread_mutex_lock(&shm_buffer->shm_lock);
            char current_room[ROOM_NAME_LEN];
            strcpy(current_room, shm_buffer->clients[client_index].room);
            pthread_mutex_unlock(&shm_buffer->shm_lock);
            
            snprintf(message, sizeof(message), "%s [#%s] %s: %s", 
                     timestamp, current_room, username, buffer);
            
            printf("%s", message);
            log_message(message);
            broadcast_room(message, client_fd, current_room);
        }
    }

    /* Cleanup */
    pthread_mutex_lock(&shm_buffer->shm_lock);
    char leaving_user[50];
    for (int i = 0; i < shm_buffer->client_count; i++) {
        if (shm_buffer->clients[i].fd == client_fd) {
            strncpy(leaving_user, shm_buffer->clients[i].username, sizeof(leaving_user) - 1);
            for (int j = i; j < shm_buffer->client_count - 1; j++) {
                shm_buffer->clients[j] = shm_buffer->clients[j + 1];
            }
            shm_buffer->client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&shm_buffer->shm_lock);

    snprintf(message, sizeof(message), "[Server]: %s has disconnected (Process: %d exiting)\n", leaving_user, getpid());
    printf("%s", message);
    log_message(message);
    broadcast_all(message);

    close(client_fd);
    exit(0);  // Exit child process
}

int main() {
    printf("[DEBUG] Starting main()\n");
    fflush(stdout);
    
    int client_fd;
    struct sockaddr_in server_addr;
    pid_t pid;

    /* Initialize mutex and log file */
    printf("[DEBUG] Initializing pthread mutex\n");
    fflush(stdout);
    
    pthread_mutex_init(&lock, NULL);
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Failed to open log file");
    }

    /* Initialize IPC resources */
    printf("[DEBUG] Calling init_shared_memory()\n");
    fflush(stdout);
    init_shared_memory();
    
    /* Store parent PID in shared memory */
    parent_pid_global = getpid();
    pthread_mutex_lock(&shm_buffer->shm_lock);
    shm_buffer->parent_pid = parent_pid_global;
    pthread_mutex_unlock(&shm_buffer->shm_lock);
    
    printf("[DEBUG] Calling init_message_queue()\n");
    fflush(stdout);
    init_message_queue();
    
    printf("[DEBUG] Calling init_semaphore()\n");
    fflush(stdout);
    init_semaphore();

    /* Setup signal handlers */
    signal(SIGINT, handle_shutdown);
    signal(SIGCHLD, handle_sigchld);  // Handle child termination
    signal(SIGUSR1, handle_broadcast_signal);  // Handle broadcast requests from children

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
    printf("[DEBUG] Entering accept loop\n");
    fflush(stdout);

    /* Set up signal mask for pselect */
    sigset_t empty_mask, block_mask;
    sigemptyset(&empty_mask);
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);  // Block SIGUSR1 except during pselect
    sigprocmask(SIG_BLOCK, &block_mask, NULL);

    while (server_running) {
        /* Process any pending broadcasts */
        if (broadcast_pending) {
            process_broadcasts();
        }
        
        /* Use pselect() with timeout - atomically unblocks signals */
        fd_set read_fds;
        struct timespec timeout;
        
        FD_ZERO(&read_fds);
        FD_SET(server_fd_global, &read_fds);
        
        timeout.tv_sec = 0;
        timeout.tv_nsec = 100000000;  // 100ms timeout
        
        /* pselect atomically unblocks signals during wait */
        int select_result = pselect(server_fd_global + 1, &read_fds, NULL, NULL, &timeout, &empty_mask);
        
        if (select_result < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal - check if shutdown or broadcast */
                if (!server_running) break;
                continue;  // Process broadcasts on next iteration
            }
            perror("pselect failed");
            continue;
        }
        
        /* Timeout occurred - process broadcasts and continue */
        if (select_result == 0) {
            continue;
        }
        
        /* Socket is ready - try to get semaphore */
        printf("[DEBUG] Waiting for semaphore...\n");
        fflush(stdout);
        
        /* Wait for semaphore (connection slot) */
        if (sem_wait(connection_sem) == -1) {
            perror("sem_wait failed");
            continue;
        }
        
        printf("[DEBUG] Got semaphore, calling accept...\n");
        fflush(stdout);

        client_fd = accept(server_fd_global, NULL, NULL);
        
        printf("[DEBUG] Accept returned fd=%d\n", client_fd);
        fflush(stdout);
        
        if (client_fd < 0) {
            if (server_running) {
                perror("Accept failed");
            }
            sem_post(connection_sem);  // Release semaphore
            continue;
        }

        pthread_mutex_lock(&shm_buffer->shm_lock);
        
        if (shm_buffer->client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&shm_buffer->shm_lock);
            char *full_msg = "Server full. Try again later.\n";
            send(client_fd, full_msg, strlen(full_msg), 0);
            close(client_fd);
            sem_post(connection_sem);
            continue;
        }
        
        shm_buffer->clients[shm_buffer->client_count].fd = client_fd;
        shm_buffer->clients[shm_buffer->client_count].authenticated = 0;
        shm_buffer->clients[shm_buffer->client_count].process_id = 0;
        strcpy(shm_buffer->clients[shm_buffer->client_count].room, "general");
        shm_buffer->client_count++;
        
        pthread_mutex_unlock(&shm_buffer->shm_lock);

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
            pthread_mutex_lock(&shm_buffer->shm_lock);
            for (int i = 0; i < shm_buffer->client_count; i++) {
                if (shm_buffer->clients[i].fd == client_fd) {
                    shm_buffer->clients[i].process_id = pid;
                    break;
                }
            }
            pthread_mutex_unlock(&shm_buffer->shm_lock);
            
            /* Parent keeps the socket open for broadcasting - child will close it when done */
            
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
