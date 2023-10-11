#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT 8000

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Cria socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(serverAddr.sin_zero), 8);

    // Conecta ao servidor
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
        perror("Connect");
        exit(1);
    }

    printf("Conectado ao servidor de chat.\n");
    printf("Digite 'q' e pressione Enter para sair.\n");

    char message[1024];

    while (1) {
        fgets(message, sizeof(message), stdin);
        if (message[0] == 'q' && message[1] == '\n') {
            break;
        }
        send(clientSocket, message, strlen(message), 0);
    }

    close(clientSocket);

    return 0;
}
