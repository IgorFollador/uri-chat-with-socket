#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 8000
#define SERVER_ADDRESS "localhost"
#define BUFFER_SIZE 1024

int keepRunning = 1;
int clientSocket;
char readBuffer[BUFFER_SIZE];

void *readThreadFunction(void *arg) {
    while (keepRunning) {
        int bytesRead = recv(clientSocket, readBuffer, sizeof(readBuffer) - 1, 0);
        if (bytesRead <= 0) {
            keepRunning = 0;
            break;
        }
        readBuffer[bytesRead] = '\0';
        printf("%s\n", readBuffer);
    }
    return NULL;
}

int main() {
    struct sockaddr_in serverAddr;
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    pthread_t readThread;
    pthread_create(&readThread, NULL, readThreadFunction, NULL);

    char inputBuffer[BUFFER_SIZE];

    printf("Digite uma mensagem (ou 'sair' para encerrar):\n");
    while (1) {
        fgets(inputBuffer, sizeof(inputBuffer), stdin);
        size_t len = strlen(inputBuffer);
        if (len > 0 && inputBuffer[len - 1] == '\n') {
            inputBuffer[len - 1] = '\0';
        }

        if (strcmp(inputBuffer, "sair") == 0) {
            keepRunning = 0;
            break;
        }
        send(clientSocket, inputBuffer, strlen(inputBuffer), 0);
    }

    pthread_join(readThread, NULL);
    close(clientSocket);

    return 0;
}