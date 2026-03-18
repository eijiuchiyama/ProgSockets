
#define _GNU_SOURCE
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

    int listen_socket, connection_socket;
    struct sockaddr_in server_info;
    char received_message[MAXMESSAGE + 1];
    pid_t process_id;
    long n;

    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("erro ao criar socket\n");
        exit(2);
    }

    bzero(&server_info, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = htonl(INADDR_ANY);
    server_info.sin_port = htons(atoi(argv[1]));
    if ((bind(listen_socket, (struct sockaddr *)&server_info, sizeof(server_info)) == -1)) {
        perror("erro no bind\n");
        exit(3);
    }

    if (listen(listen_socket, 1) == -1) {
        perror("erro no listen\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);

    for(;;) {
        if ((connection_socket = accept(listen_socket, (struct sockaddr *) NULL, NULL)) == -1) {
            perror("erro no accept\n");
            exit(5);
        };

        if ((process_id = fork()) == 0) {
            close(listen_socket);

            printf("[Uma conexão aberta]\n");
            
            while ((n = read(connection_socket, received_message, MAXMESSAGE)) > 0){
                received_message[n] = 0;
                printf("[Cliente enviou:] %s", received_message);
                
                write(connection_socket, received_message, strlen(received_message));
            }
            
            printf("[Uma conexão encerrada]\n");
            exit(0);

        }
        
        close(connection_socket);

    }

    exit(0);
}