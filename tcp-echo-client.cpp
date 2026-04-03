#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>

#define MAXMESSAGE 4096

int main(int argc, char **argv) {
    int client_socket;
    struct sockaddr_in client_info;
    char sent_message[MAXMESSAGE];
    char received_message[MAXMESSAGE];
    long n;

    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        exit(1);
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("erro ao criar socket\n");
        exit(2);
    }

    bzero(&client_info, sizeof(client_info));
    client_info.sin_family = AF_INET;
    client_info.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    client_info.sin_port = htons(atoi(argv[1]));
    if ((connect(client_socket, (struct sockaddr *)&client_info, sizeof(client_info))) == -1) {
        perror("erro no connect\n");
        exit(3);
    } else {
        printf("[Conectado com sucesso]\n");
    }

    bzero(sent_message, sizeof(sent_message));
    printf("Escreva sua mensagem: ");
    n = 0;
    while ((sent_message[n++] = getchar()) != '\n');

    write(client_socket, sent_message, strlen(sent_message));

    bzero(received_message, sizeof(received_message));
    read(client_socket, received_message, MAXMESSAGE);
    printf("Resposta do server: %s", received_message);

    close(client_socket);

    exit(0);

}
