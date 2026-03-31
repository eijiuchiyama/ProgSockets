#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include "resource_man.hpp"
#include "parser.hpp"
#include "protocol.hpp"

#define PORT 8842

void *handler(void *args)
{
  int conn = *((int *)args);
  free(args);
  char received_message[MAX_MESSAGE + 1];
  long n;
  
    while ((n = read(conn, received_message, MAX_MESSAGE)) > 0)
    {
      received_message[n] = 0;
      printf("[Cliente enviou:] %s", received_message);

      parse(received_message);
    }
  
    printf("[Uma conexão encerrada]\n");

  
  return NULL;
}

int main(int argc, char **argv)
{
  int listen_socket, connection_socket;
  struct sockaddr_in server_info;

  if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("erro ao criar socket\n");
    exit(2);
  }

  int opt = 1;
  if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
  {
    perror("setsockopt(SO_REUSEADDR) failed");
  }


  bzero(&server_info, sizeof(server_info));
  server_info.sin_family = AF_INET;
  server_info.sin_addr.s_addr = htonl(INADDR_ANY);
  server_info.sin_port = htons(PORT);
  if ((bind(listen_socket, (struct sockaddr *)&server_info, sizeof(server_info)) == -1))
  {
    perror("erro no bind\n");
    exit(3);
  }

  if (listen(listen_socket, 1) == -1)
  {
    perror("erro no listen\n");
    exit(4);
  }

  printf("[Servidor no ar. Aguardando conexões na porta %s]\n", argv[1]);

  for (;;)
  {
    if ((connection_socket = accept(listen_socket, (struct sockaddr *)NULL, NULL)) == -1)
    {
      perror("erro no accept\n");
      exit(5);
    };

    pthread_t thread;

    int *conn_copy = (int*) malloc(sizeof(int));
    *conn_copy = connection_socket;
    pthread_create(&thread, NULL, handler, conn_copy);
    pthread_detach(thread);

    printf("[Uma conexão aberta]\n");
  }

  exit(0);

  return 0;
}
