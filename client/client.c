#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sockfd;
char username[50];

/* Thread to receive messages */
void *receive_messages(void *arg) {
    (void)arg;  // Argument not used
    char buffer[BUFFER_SIZE];
    int bytes;

    while ((bytes = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char message[BUFFER_SIZE];
    char final_msg[BUFFER_SIZE];
    char password[50];

    printf("=== NetChat Client ===\n");
    printf("Enter your username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;   // remove newline

    printf("Enter password: ");
    fgets(password, 50, stdin);
    password[strcspn(password, "\n")] = 0;   // remove newline

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    printf("Connected to server...\n");
    
    /* Send username and password for authentication */
    char auth_username[BUFFER_SIZE];
    char auth_password[BUFFER_SIZE];
    snprintf(auth_username, sizeof(auth_username), "%s\n", username);
    snprintf(auth_password, sizeof(auth_password), "%s\n", password);
    send(sockfd, auth_username, strlen(auth_username), 0);
    send(sockfd, auth_password, strlen(auth_password), 0);

    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        
        /* Check if it's a command */
        if (message[0] == '/') {
            send(sockfd, message, strlen(message), 0);
        } else {
            snprintf(final_msg, BUFFER_SIZE, "%s: %s", username, message);
            send(sockfd, final_msg, strlen(final_msg), 0);
        }
    }

    close(sockfd);
    return 0;
}
