#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define LOG_FILE "chat.log"
#define USERS_FILE "users.txt"
#define MAX_ROOMS 5
#define ROOM_NAME_LEN 30

typedef struct {
    int fd;
    char username[50];
    char password[50];
    int authenticated;
    char room[ROOM_NAME_LEN];
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t lock;
FILE *log_file;
int server_fd_global;
volatile sig_atomic_t server_running = 1;

/* Get current timestamp */
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", t);
}

/* Log message to file with mutex protection */
void log_message(const char *message) {
    pthread_mutex_lock(&lock);
    if (log_file) {
        char timestamp[20];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(log_file, "%s %s", timestamp, message);
        fflush(log_file);
    }
    pthread_mutex_unlock(&lock);
}

/* Broadcast message to all clients except sender */
void broadcast(char *message, int sender_fd) {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd != sender_fd) {
            send(clients[i].fd, message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

/* Broadcast to all clients including sender */
void broadcast_all(char *message) {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++) {
        send(clients[i].fd, message, strlen(message), 0);
    }

    pthread_mutex_unlock(&lock);
}

/* Broadcast to all clients in the same room except sender */
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

/* Send private message to specific user */
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
    
    pthread_mutex_unlock(&lock);
    return found;
}

/* Register new user - add to users.txt */
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
        printf("[Server]: Invalid username or password format\n");
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

/* Validate user credentials - file-based authentication */
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
        line[strcspn(line, "\n\r")] = '\0';
        
        if (sscanf(line, "%49[^:]:%49s", stored_user, stored_pass) == 2) {
            if (strcmp(stored_user, clean_user) == 0) {
                fclose(file);
                if (strcmp(stored_pass, clean_pass) == 0) {
                    return 1;
                } else {
                    printf("[Server]: Wrong password for user: %s\n", clean_user);
                    return 0;
                }
            }
        }
    }
    
    fclose(file);
    return register_user(clean_user, clean_pass);
}

/* Signal handler for graceful shutdown */
void handle_shutdown(int sig) {
    (void)sig;  // Signal number not used in handler
    server_running = 0;
    
    char *msg = "\n[Server]: Server is shutting down. Goodbye!\n";
    broadcast_all(msg);
    log_message(msg);
    
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        close(clients[i].fd);
    }
    pthread_mutex_unlock(&lock);
    
    if (log_file) {
        fclose(log_file);
    }
    
    close(server_fd_global);
    pthread_mutex_destroy(&lock);
    
    printf("\nServer shutdown complete.\n");
    exit(0);
}

/* Handle individual client */
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char username[50];
    char password[50];
    char message[BUFFER_SIZE + 100];
    int bytes_read;
    int client_index = -1;

    /* Step 1: Receive username (read until newline) */
    int idx = 0;
    char c;
    while (idx < (int)sizeof(username) - 1) {
        bytes_read = recv(client_fd, &c, 1, 0);
        if (bytes_read <= 0) {
            close(client_fd);
            return NULL;
        }
        if (c == '\n') break;
        username[idx++] = c;
    }
    username[idx] = '\0';

    /* Step 2: Receive password (read until newline) */
    idx = 0;
    while (idx < (int)sizeof(password) - 1) {
        bytes_read = recv(client_fd, &c, 1, 0);
        if (bytes_read <= 0) {
            close(client_fd);
            return NULL;
        }
        if (c == '\n') break;
        password[idx++] = c;
    }
    password[idx] = '\0';
    
    /* Validate inputs */
    if (strlen(username) == 0 || strlen(password) == 0) {
        char *err = "Error: Username and password cannot be empty.\n";
        send(client_fd, err, strlen(err), 0);
        close(client_fd);
        
        pthread_mutex_lock(&lock);
        for (int i = 0; i < client_count; i++) {
            if (clients[i].fd == client_fd) {
                for (int j = i; j < client_count - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                client_count--;
                break;
            }
        }
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    /* Step 3: Authenticate */
    if (!authenticate_user(username, password)) {
        char *auth_fail = "Authentication failed. Disconnecting...\n";
        send(client_fd, auth_fail, strlen(auth_fail), 0);
        close(client_fd);
        
        /* Remove from client list */
        pthread_mutex_lock(&lock);
        for (int i = 0; i < client_count; i++) {
            if (clients[i].fd == client_fd) {
                for (int j = i; j < client_count - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                client_count--;
                break;
            }
        }
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    /* Authentication successful */
    char welcome_banner[BUFFER_SIZE * 2];
    snprintf(welcome_banner, sizeof(welcome_banner),
        "\n"
        "╔════════════════════════════════════════════════════════════════╗\n"
        "║                  🎉 WELCOME TO NETCHAT! 🎉                    ║\n"
        "╚════════════════════════════════════════════════════════════════╝\n"
        "\n"
        "✅ Authentication successful!\n"
        "📝 Your account has been saved for future logins.\n"
        "🏠 You are now in room: #general\n"
        "\n"
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
        "║                                                                ║\n"
        "║  👥 USERS:                                                     ║\n"
        "║     • /users                - List users in current room      ║\n"
        "║                                                                ║\n"
        "║  ℹ️  HELP:                                                      ║\n"
        "║     • /help                 - Show this menu again            ║\n"
        "║                                                                ║\n"
        "╚════════════════════════════════════════════════════════════════╝\n"
        "\n");
    send(client_fd, welcome_banner, strlen(welcome_banner), 0);

    /* Store user info in client structure */
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd == client_fd) {
            strncpy(clients[i].username, username, sizeof(clients[i].username) - 1);
            strncpy(clients[i].password, password, sizeof(clients[i].password) - 1);
            clients[i].authenticated = 1;
            strcpy(clients[i].room, "general");  // Default room
            client_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    /* Send join notification to room */
    snprintf(message, sizeof(message), "[Server]: %s has joined #general\n", username);
    printf("%s", message);
    log_message(message);
    broadcast_room(message, -1, "general");  // Send to all in general room

    /* Handle messages and commands */
    while ((bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        
        /* Check for /help command */
        if (strncmp(buffer, "/help", 5) == 0 && (buffer[5] == '\n' || buffer[5] == '\0')) {
            char help_menu[BUFFER_SIZE * 2];
            snprintf(help_menu, sizeof(help_menu),
                "\n"
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
                "║                                                                ║\n"
                "║  👥 USERS:                                                     ║\n"
                "║     • /users                - List users in current room      ║\n"
                "║                                                                ║\n"
                "║  ℹ️  HELP:                                                      ║\n"
                "║     • /help                 - Show this menu again            ║\n"
                "║                                                                ║\n"
                "╚════════════════════════════════════════════════════════════════╝\n"
                "\n");
            send(client_fd, help_menu, strlen(help_menu), 0);
            continue;
        }
        
        /* Parse commands */
        if (strncmp(buffer, "/pm ", 4) == 0) {
            /* Private message: /pm username message */
            char *cmd = buffer + 4;
            char *space = strchr(cmd, ' ');
            if (space) {
                *space = '\0';
                char *target_user = cmd;
                char *pm_msg = space + 1;
                pm_msg[strcspn(pm_msg, "\n")] = 0;  // Remove newline
                
                if (send_private_message(target_user, pm_msg, username)) {
                    char confirm[BUFFER_SIZE];
                    snprintf(confirm, sizeof(confirm), "[PM to %s]: %s\n", target_user, pm_msg);
                    send(client_fd, confirm, strlen(confirm), 0);
                    
                    char log_msg[BUFFER_SIZE];
                    snprintf(log_msg, sizeof(log_msg), "[PM] %s -> %s: %s\n", username, target_user, pm_msg);
                    log_message(log_msg);
                } else {
                    char *not_found = "[Server]: User not found\n";
                    send(client_fd, not_found, strlen(not_found), 0);
                }
            } else {
                char *usage = "[Server]: Usage: /pm <username> <message>\n";
                send(client_fd, usage, strlen(usage), 0);
            }
        }
        else if (strncmp(buffer, "/room", 5) == 0 && (buffer[5] == '\n' || buffer[5] == '\0')) {
            /* Show current room */
            pthread_mutex_lock(&lock);
            char room_msg[BUFFER_SIZE];
            snprintf(room_msg, sizeof(room_msg), "[Server]: You are in #%s\n", clients[client_index].room);
            pthread_mutex_unlock(&lock);
            send(client_fd, room_msg, strlen(room_msg), 0);
        }
        else if (strncmp(buffer, "/join ", 6) == 0) {
            /* Join room: /join roomname */
            char *new_room = buffer + 6;
            new_room[strcspn(new_room, "\n")] = 0;
            
            pthread_mutex_lock(&lock);
            char old_room[ROOM_NAME_LEN];
            strcpy(old_room, clients[client_index].room);
            strncpy(clients[client_index].room, new_room, ROOM_NAME_LEN - 1);
            pthread_mutex_unlock(&lock);
            
            /* Notify old room */
            snprintf(message, sizeof(message), "[Server]: %s has left #%s\n", username, old_room);
            broadcast_room(message, -1, old_room);
            log_message(message);
            
            /* Notify new room */
            snprintf(message, sizeof(message), "[Server]: %s has joined #%s\n", username, new_room);
            broadcast_room(message, -1, new_room);
            log_message(message);
            
            snprintf(message, sizeof(message), "[Server]: You joined #%s\n", new_room);
            send(client_fd, message, strlen(message), 0);
        }
        else if (strncmp(buffer, "/users", 6) == 0) {
            /* List users in current room */
            pthread_mutex_lock(&lock);
            char user_list[BUFFER_SIZE] = "[Server]: Users in this room: ";
            char current_room[ROOM_NAME_LEN];
            strcpy(current_room, clients[client_index].room);
            
            for (int i = 0; i < client_count; i++) {
                if (strcmp(clients[i].room, current_room) == 0) {
                    strcat(user_list, clients[i].username);
                    strcat(user_list, " ");
                }
            }
            pthread_mutex_unlock(&lock);
            strcat(user_list, "\n");
            send(client_fd, user_list, strlen(user_list), 0);
        }
        else if (strncmp(buffer, "/rooms", 6) == 0) {
            /* List all active rooms */
            pthread_mutex_lock(&lock);
            char room_list[BUFFER_SIZE] = "[Server]: Active rooms: ";
            char rooms[MAX_ROOMS][ROOM_NAME_LEN];
            int room_count = 0;
            
            for (int i = 0; i < client_count; i++) {
                int exists = 0;
                for (int j = 0; j < room_count; j++) {
                    if (strcmp(rooms[j], clients[i].room) == 0) {
                        exists = 1;
                        break;
                    }
                }
                if (!exists && room_count < MAX_ROOMS) {
                    strcpy(rooms[room_count++], clients[i].room);
                }
            }
            
            for (int i = 0; i < room_count; i++) {
                strcat(room_list, "#");
                strcat(room_list, rooms[i]);
                strcat(room_list, " ");
            }
            pthread_mutex_unlock(&lock);
            strcat(room_list, "\n");
            send(client_fd, room_list, strlen(room_list), 0);
        }
        else {
            /* Regular message - broadcast to room with timestamp */
            char timestamp[20];
            get_timestamp(timestamp, sizeof(timestamp));
            
            pthread_mutex_lock(&lock);
            char current_room[ROOM_NAME_LEN];
            strcpy(current_room, clients[client_index].room);
            pthread_mutex_unlock(&lock);
            
            snprintf(message, sizeof(message), "%s [#%s] %s", timestamp, current_room, buffer);
            
            printf("%s", message);
            log_message(message);
            broadcast_room(message, client_fd, current_room);
        }
    }

    /* Client disconnected */
    pthread_mutex_lock(&lock);
    char leaving_user[50];
    char leaving_room[ROOM_NAME_LEN];
    for (int i = 0; i < client_count; i++) {
        if (clients[i].fd == client_fd) {
            strncpy(leaving_user, clients[i].username, sizeof(leaving_user) - 1);
            strncpy(leaving_room, clients[i].room, sizeof(leaving_room) - 1);
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    snprintf(message, sizeof(message), "[Server]: %s has left #%s\n", leaving_user, leaving_room);
    printf("%s", message);
    log_message(message);
    broadcast_room(message, -1, leaving_room);

    close(client_fd);
    return NULL;
}

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    pthread_t tid;

    /* Initialize mutex and open log file */
    pthread_mutex_init(&lock, NULL);
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Failed to open log file");
    }

    /* Setup signal handler for graceful shutdown (Ctrl+C) */
    signal(SIGINT, handle_shutdown);

    server_fd_global = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_global < 0) {
        perror("Socket failed");
        exit(1);
    }

    /* Allow socket reuse */
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

    printf("Server running on port %d...\n", PORT);
    printf("Maximum clients: %d\n", MAX_CLIENTS);
    printf("Press Ctrl+C for graceful shutdown\n\n");
    
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "[Server]: Server started\n");
    log_message(log_msg);

    while (server_running) {
        client_fd = accept(server_fd_global, NULL, NULL);
        
        if (client_fd < 0) {
            if (server_running) {
                perror("Accept failed");
            }
            continue;
        }

        pthread_mutex_lock(&lock);
        
        /* Check if server is full */
        if (client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&lock);
            char *full_msg = "Server full. Try again later.\n";
            send(client_fd, full_msg, strlen(full_msg), 0);
            close(client_fd);
            printf("[Server]: Rejected client - server full\n");
            continue;
        }
        
        clients[client_count].fd = client_fd;
        memset(clients[client_count].username, 0, sizeof(clients[client_count].username));
        memset(clients[client_count].password, 0, sizeof(clients[client_count].password));
        clients[client_count].authenticated = 0;
        strcpy(clients[client_count].room, "general");
        client_count++;
        
        pthread_mutex_unlock(&lock);

        pthread_create(&tid, NULL, handle_client, &client_fd);
        pthread_detach(tid);  // Auto cleanup thread resources
    }

    close(server_fd_global);
    if (log_file) {
        fclose(log_file);
    }
    pthread_mutex_destroy(&lock);
    return 0;
}
