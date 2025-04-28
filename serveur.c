// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 4
#define BUFFER_SIZE 4096

int main() {
    int server_fd, client_fds[MAX_CLIENTS], max_clients = MAX_CLIENTS;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 4) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[Serveur] En attente de connexions...\n");

    for (int i = 0; i < MAX_CLIENTS; i++) client_fds[i] = 0;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < max_clients; i++) {
            int sd = client_fds[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0)) {
            perror("select error");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("[Serveur] Nouveau client connecté.\n");

            for (int i = 0; i < max_clients; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < max_clients; i++) {
            int sd = client_fds[i];
            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    printf("[Serveur] Client déconnecté.\n");
                    close(sd);
                    client_fds[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("[Serveur] Message reçu : %s\n", buffer);

                    // Rebalance à tous les autres clients
                    for (int j = 0; j < max_clients; j++) {
                        if (client_fds[j] != 0 && client_fds[j] != sd) {
                            send(client_fds[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }
}