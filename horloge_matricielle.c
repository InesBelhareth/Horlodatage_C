#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define NUM_PROCESSES 4
#define BUFFER_SIZE 2048

int clock_matrix[NUM_PROCESSES][NUM_PROCESSES] = {0};
int my_id = 0;

void initialize_clock() {
    memset(clock_matrix, 0, sizeof(clock_matrix));
    clock_matrix[my_id][my_id] = 1;
}

void print_clock() {
    printf("\n[Process %d] Current Matrix Clock:\n", my_id);
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("P%d: [", i);
        for (int j = 0; j < NUM_PROCESSES; j++) {
            printf("%3d ", clock_matrix[i][j]);
        }
        printf("]\n");
    }
}

void local_event() {
    clock_matrix[my_id][my_id]++;
    printf("\n[Process %d] Local event occurred\n", my_id);
    print_clock();
}

void update_clock(int received[NUM_PROCESSES][NUM_PROCESSES]) {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        for (int j = 0; j < NUM_PROCESSES; j++) {
            if (received[i][j] > clock_matrix[i][j]) {
                clock_matrix[i][j] = received[i][j];
            }
        }
    }
    printf("\n[Process %d] Clock updated after receiving\n", my_id);
    print_clock();
}

void send_message(int sock) {
    // Increment before sending
    clock_matrix[my_id][my_id]++;
    
    // Prepare message
    char buffer[BUFFER_SIZE] = {0};
    char temp[20];
    
    strcat(buffer, "MATRIX|");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        for (int j = 0; j < NUM_PROCESSES; j++) {
            sprintf(temp, "%d,", clock_matrix[i][j]);
            strcat(buffer, temp);
        }
        strcat(buffer, ";");
    }
    
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    
    printf("\n[Process %d] Sent matrix clock\n", my_id);
    print_clock();
}

void receive_message(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(sock, buffer, BUFFER_SIZE-1, 0);
    
    if (bytes <= 0) {
        if (bytes == 0) {
            printf("Server disconnected\n");
        } else {
            perror("Receive error");
        }
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    buffer[bytes] = '\0';
    
    // Parse message
    if (strncmp(buffer, "MATRIX|", 7) != 0) {
        fprintf(stderr, "Invalid message format\n");
        return;
    }
    
    int received[NUM_PROCESSES][NUM_PROCESSES] = {0};
    char *ptr = buffer + 7; // Skip "MATRIX|"
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        char *line = strtok_r(ptr, ";", &ptr);
        if (!line) break;
        
        for (int j = 0; j < NUM_PROCESSES; j++) {
            char *val = strtok_r(line, ",", &line);
            if (!val) break;
            received[i][j] = atoi(val);
        }
    }
    
    update_clock(received);
}

int main() {
    printf("Enter process ID (0-%d): ", NUM_PROCESSES-1);
    scanf("%d", &my_id);
    getchar();
    
    if (my_id < 0 || my_id >= NUM_PROCESSES) {
        fprintf(stderr, "Invalid process ID\n");
        exit(EXIT_FAILURE);
    }
    
    initialize_clock();
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Process %d connected to server\n", my_id);
    
    // Main loop
    for (int i = 0; i < 3; i++) {
        local_event();
        sleep(1);
        
        send_message(sock);
        sleep(1);
        
        receive_message(sock);
        sleep(2);
    }
    
    // Final synchronization
    send_message(sock);
    sleep(1);
    receive_message(sock);
    
    printf("\n[Process %d] Final clock state:\n", my_id);
    print_clock();
    
    close(sock);
    return 0;
}