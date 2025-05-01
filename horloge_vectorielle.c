// client_vectoriel.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>

#define BASE_PORT 8080
#define CLIENT_COUNT 4
#define BUFFER_SIZE 1024
#define MAX_RETRIES 3
#define RETRY_DELAY 2

int horloge[CLIENT_COUNT] = {0};
int idProcessus = 0;
int running = 1;
pthread_mutex_t lock;

typedef struct {
    int id;
    struct sockaddr_in addr;
} Peer;

Peer peers[CLIENT_COUNT];

int max(int a, int b) {
    return (a > b) ? a : b;
}

void afficherHorloge() {
    printf("[Vectoriel %d] Horloge: [ ", idProcessus);
    for (int i = 0; i < CLIENT_COUNT; i++) {
        printf("%d ", horloge[i]);
    }
    printf("]\n");
}

void evenementLocal() {
    pthread_mutex_lock(&lock);
    horloge[idProcessus]++;
    printf("[Vectoriel %d] Événement local.\n", idProcessus);
    afficherHorloge();
    pthread_mutex_unlock(&lock);
}

// Prépare le message contenant l'horloge vectorielle
char* prepareMessage() {
    static char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int offset = 0;

    pthread_mutex_lock(&lock);
    horloge[idProcessus]++;
    
    for (int i = 0; i < CLIENT_COUNT; i++) {
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "%d,", horloge[i]);
    }
    
    // Debug output
    printf("[Vectoriel %d] Message préparé: %s\n", idProcessus, buffer);
    pthread_mutex_unlock(&lock);
    
    return buffer;
}

// Envoie un message à un pair spécifique
int envoyerMessagePair(int peerId, char* message) {
    int sock;
    struct sockaddr_in addr = peers[peerId].addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[Erreur] Création socket");
        return -1;
    }
    
    // Configuration timeout de connexion
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
    
    for (int tentative = 1; tentative <= MAX_RETRIES; tentative++) {
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("[Vectoriel %d] Tentative %d échouée vers %d. Réessai...\n", 
                   idProcessus, tentative, peerId);
            
            if (tentative == MAX_RETRIES) {
                close(sock);
                return -1;
            }
            sleep(RETRY_DELAY);
        } else {
            break;
        }
    }
    
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("[Erreur] Envoi");
        close(sock);
        return -1;
    }
    
    close(sock);
    return 0;
}

// Envoie un message à tous les pairs
void envoyerMessage() {
    char *message = prepareMessage();
    
    for (int i = 0; i < CLIENT_COUNT; i++) {
        if (envoyerMessagePair(i, message) == 0) {
            if (i == idProcessus) {
                printf("[Vectoriel %d] Message envoyé à soi-même\n", idProcessus);
            } else {
                printf("[Vectoriel %d] Message envoyé à %d\n", idProcessus, i);
            }
            afficherHorloge();
        }
    }
}

// Traite un message reçu
void traiterMessage(char *buffer) {
    // Debugging output
    printf("[Vectoriel %d] Message reçu: %s\n", idProcessus, buffer);
    
    int horloge_recue[CLIENT_COUNT] = {0};
    char *token = strtok(buffer, ",");
    int i = 0;
    
    while (token != NULL && i < CLIENT_COUNT) {
        horloge_recue[i] = atoi(token);
        token = strtok(NULL, ",");
        i++;
    }
    
    // Debugging: afficher l'horloge extraite
    printf("[Vectoriel %d] Horloge extraite: [ ", idProcessus);
    for (i = 0; i < CLIENT_COUNT; i++) {
        printf("%d ", horloge_recue[i]);
    }
    printf("]\n");
    
    pthread_mutex_lock(&lock);
    // Mise à jour de l'horloge vectorielle
    for (i = 0; i < CLIENT_COUNT; i++) {
        horloge[i] = max(horloge[i], horloge_recue[i]);
    }
    
    // Incrémenter sa propre position
    horloge[idProcessus]++;
    printf("[Vectoriel %d] Mise à jour horloge après réception\n", idProcessus);
    afficherHorloge();
    pthread_mutex_unlock(&lock);
}

// Thread d'écoute pour les connexions entrantes
void* ecouterConnexions(void *arg) {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(BASE_PORT + idProcessus);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, CLIENT_COUNT) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("[Vectoriel %d] En écoute sur le port %d\n", idProcessus, BASE_PORT + idProcessus);
    
    while(running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("select error");
            continue;
        }
        
        if (activity == 0) {
            // Timeout, pas de nouvelle connexion
            continue;
        }
        
        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                continue;
            }
            
            char buffer[BUFFER_SIZE] = {0};
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread > 0) {
                buffer[valread] = '\0';
                traiterMessage(buffer);
            }
            
            close(new_socket);
        }
    }
    
    close(server_fd);
    return NULL;
}

void sigintHandler(int sig_num) {
    running = 0;
    printf("\n[Vectoriel %d] Arrêt du programme...\n", idProcessus);
    exit(0);
}

// Interactive menu for user commands
void* userCommandsThread(void *arg) {
    char command[20];
    
    // Wait for automatic execution to complete
    int autoExecutionComplete = 0;
    pthread_mutex_lock(&lock);
    autoExecutionComplete = *((int*)arg);
    pthread_mutex_unlock(&lock);
    
    while (!autoExecutionComplete) {
        // Wait until automatic execution is complete
        sleep(1);
        pthread_mutex_lock(&lock);
        autoExecutionComplete = *((int*)arg);
        pthread_mutex_unlock(&lock);
    }
    
    // Add a delay to allow for all messages to be processed
    printf("\nAttente de la fin des communications en cours...\n");
    sleep(5);  // Wait 5 seconds for any remaining messages to be processed
    
    // Clear any leftover input
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    // Start interactive menu after automatic execution
    printf("\n========================================\n");
    printf("Exécution automatique terminée.\n");
    printf("Vous pouvez maintenant interagir avec le programme.\n");
    printf("========================================\n");
    
    while(running) {
        printf("\nCommandes disponibles:\n");
        printf("1. Événement local\n");
        printf("2. Envoyer message\n");
        printf("q. Quitter\n");
        printf("Choix: ");
        fflush(stdout);
        
        if (scanf("%s", command) != 1) {
            continue;
        }
        
        if (strcmp(command, "1") == 0) {
            evenementLocal();
        } else if (strcmp(command, "2") == 0) {
            envoyerMessage();
        } else if (strcmp(command, "q") == 0) {
            running = 0;
            printf("Arrêt du programme...\n");
            kill(getpid(), SIGINT);
            break;
        }
    }
    
    return NULL;
}

int main() {
    pthread_t listener_thread, commands_thread;
    
    // Flag for automatic execution completion
    int autoExecutionComplete = 0;
    
    // Initialiser le mutex
    pthread_mutex_init(&lock, NULL);
    
    // Configurer le gestionnaire de signal pour Ctrl+C
    signal(SIGINT, sigintHandler);
    
    printf("Entrez l'ID du processus (0 à %d) : ", CLIENT_COUNT - 1);
    scanf("%d", &idProcessus);
    
    if (idProcessus < 0 || idProcessus >= CLIENT_COUNT) {
        printf("ID de processus invalide, doit être entre 0 et %d\n", CLIENT_COUNT - 1);
        return EXIT_FAILURE;
    }
    
    // Initialiser les informations des pairs
    for (int i = 0; i < CLIENT_COUNT; i++) {
        peers[i].id = i;
        peers[i].addr.sin_family = AF_INET;
        peers[i].addr.sin_port = htons(BASE_PORT + i);
        inet_pton(AF_INET, "127.0.0.1", &peers[i].addr.sin_addr);
    }
    
    // Démarrer le thread d'écoute
    if (pthread_create(&listener_thread, NULL, ecouterConnexions, NULL) != 0) {
        perror("Erreur création thread d'écoute");
        return EXIT_FAILURE;
    }
    
    // Démarrer le thread de commandes utilisateur avec le flag
    if (pthread_create(&commands_thread, NULL, userCommandsThread, &autoExecutionComplete) != 0) {
        perror("Erreur création thread de commandes");
        return EXIT_FAILURE;
    }
    
    // Laisser le temps au thread d'écoute de démarrer
    sleep(1);
    
    // Mode normal: exécuter les 5 événements locaux
    printf("\nMode normal: exécution des 5 événements locaux automatiques...\n");
    for (int i = 0; i < 5; i++) {
        evenementLocal();
        sleep(1);
    }
    
    // Envoyer des messages à tous les pairs
    envoyerMessage();
    
    // Signal that automatic execution is complete
    pthread_mutex_lock(&lock);
    autoExecutionComplete = 1;
    pthread_mutex_unlock(&lock);
    
    // Attendre que les threads se terminent (ne se termineront que par Ctrl+C)
    pthread_join(commands_thread, NULL);
    pthread_join(listener_thread, NULL);
    
    // Libérer les ressources
    pthread_mutex_destroy(&lock);
    
    return 0;
}
