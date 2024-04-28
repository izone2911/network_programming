#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 8000
#define MAX_CHILDREN 5

void handle_client(int client_socket) {
    char buf[256];
    int ret = recv(client_socket, buf, sizeof(buf), 0);
    buf[ret] = '\0';
    puts(buf);

    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello Client</h1></body></html>";
    send(client_socket, msg, strlen(msg), 0);

    close(client_socket);
}

int main() {
    int listener;
    struct sockaddr_in address;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections...\n");

    for (int i = 0; i < MAX_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            printf("Child process %d created\n", getpid());
            while (1) {
                int client_socket = accept(listener, NULL, NULL);
                printf("Child process %d handling client %d\n", getpid(), client_socket);
                printf("--------------------------------\n");
                handle_client(client_socket);
            }
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    while (wait(NULL) > 0);

    return 0;
}