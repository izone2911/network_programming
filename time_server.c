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
#include <time.h>

#define PORT 8888
#define MAX_CLIENTS 10
#define BUFFER_SIZE 256

void get_current_time(char* format, char* response, size_t response_size) {
    time_t current_time = time(NULL);
    struct tm* timeinfo = localtime(&current_time);
    strftime(response, response_size, format, timeinfo);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    memset(response, 0, BUFFER_SIZE);

    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Error in receiving data");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        } else {
            char command[BUFFER_SIZE];
            sscanf(buffer, "%s", command);

            if (strcmp(command, "GET_TIME") == 0) {
                char format[BUFFER_SIZE];
                sscanf(buffer, "%*s %s", format);

                if (strcmp(format, "dd/mm/yyyy") == 0 ||
                    strcmp(format, "dd/mm/yy") == 0 ||
                    strcmp(format, "mm/dd/yyyy") == 0 ||
                    strcmp(format, "mm/dd/yy") == 0) {

                    get_current_time(format, response, BUFFER_SIZE);

                    send(client_socket, response, strlen(response), 0);
                } else {
                    strcpy(response, "Invalid date format");
                    send(client_socket, response, strlen(response), 0);
                }
            } else {
                strcpy(response, "Invalid command");
                send(client_socket, response, strlen(response), 0);
            }
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, BUFFER_SIZE);
    }

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

    if (listen(listener, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections...\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int client_socket = accept(listener, (struct sockaddr*)&client_addr, &client_addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            printf("Handling client %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            handle_client(client_socket);
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
            close(client_socket);
            continue;
        }

        close(client_socket);
    }

    return 0;
}