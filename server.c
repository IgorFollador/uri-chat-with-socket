#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define MAX_CLIENTS 10 // Defina o número máximo de clientes
#define PORT 8000

int clientSockets[MAX_CLIENTS];
int clientCount = 0;
pthread_t threads[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *clientHandler(void *arg) {
    int clientSocket = *(int *)arg;
    char message[1024];
    int bytesRead;

    int clientNumber;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clientCount; i++) {
        if (clientSockets[i] == clientSocket) {
            clientNumber = i + 1; // Número do cliente (começando em 1)
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    while (1) {
        bytesRead = recv(clientSocket, message, sizeof(message), 0);
        if (bytesRead <= 0) {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < clientCount; i++) {
                if (clientSockets[i] == clientSocket) {
                    // Remove o cliente desconectado da lista
                    for (int j = i; j < clientCount - 1; j++) {
                        clientSockets[j] = clientSockets[j + 1];
                    }
                    clientCount--;
                }
            }
            pthread_mutex_unlock(&mutex);
            close(clientSocket);
            break;
        }
        message[bytesRead] = '\0';
        
        // Adiciona o prefixo com o número do cliente
        char messageWithPrefix[2048]; // Aumentar o tamanho do buffer
        snprintf(messageWithPrefix, sizeof(messageWithPrefix), "Cliente %d: %s", clientNumber, message);
        
        // Envia a mensagem com o prefixo para todos os outros clientes
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clientCount; i++) {
            if (clientSockets[i] != clientSocket) {
                send(clientSockets[i], messageWithPrefix, strlen(messageWithPrefix), 0);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    // Inicialize a matriz de sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clientSockets[i] = -1;
    }

    // Cria socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    // Define e configura os parâmetros de endereçamento
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddr.sin_zero), 8);

    // Associa o socket com o endereço local
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }

    // Aguarda solicitações de conexões
    if (listen(serverSocket, 5) == -1) {
        perror("Listen");
        exit(1);
    }

    printf("Servidor de chat está ouvindo na porta %d...\n", PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSocket == -1) {
            perror("Accept");
            continue;
        }

        pthread_mutex_lock(&mutex);
        if (clientCount < MAX_CLIENTS) {
            // Adicione o novo cliente à lista de clientes
            clientSockets[clientCount] = clientSocket;
            clientCount++;
            // Inicie uma nova thread para lidar com o cliente
            pthread_create(&threads[clientCount - 1], NULL, clientHandler, &clientSocket);
        } else {
            printf("Número máximo de clientes atingido. Rejeitando nova conexão.\n");
            close(clientSocket);
        }
        pthread_mutex_unlock(&mutex);
    }

    close(serverSocket);

    return 0;
}
