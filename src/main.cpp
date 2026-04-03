#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include "resource_man.hpp"
#include "parser.hpp"
#include "protocol.hpp"
#include "handler.hpp"

std::string serialize(ParseResult parse_result, Response response)
{

	if(response.status_code != 0){
		return std::string(u8"😡​") + " " + std::to_string(response.status_code) + "\n";
	} else if(parse_result.msg.command == GET){
		return std::string(u8"👍") + " " + response.value + "\n";
	} else if(parse_result.msg.command == SET){
		return std::string(u8"👍") + "\n";
	} else if(parse_result.msg.command == CREATE){
		return std::string(u8"👍") + " " + std::to_string(response.id) + "\n";
	} else if(parse_result.msg.command == RESERVE){
		return std::string(u8"👍") + "\n";
	} else if(parse_result.msg.command == RELEASE){
		return std::string(u8"👍") + "\n";
	} else if(parse_result.msg.command == LIST){
		std::string s = "";
		s += std::string(u8"👍");
		s += " "; 
		s += std::to_string(response.count);
		for(int i = 0; i < response.count; i++){
			s += " ";
			s += std::to_string(response.list[i]->id);
		}
		s += "\n";
		return s;
	}
}

void *handler(void *args)
{
  int conn = *((int *)args);
  free(args);
  char received_message[MAX_MESSAGE + 1];
  pthread_t self = pthread_self();
  long n;
  ParseResult parse_result;
  Response response;
  std::string serial_response;
  
    while ((n = read(conn, received_message, MAX_MESSAGE)) > 0)
    {
      received_message[n] = 0;
      printf("[Cliente enviou:] %s", received_message);

      parse_result = parse(received_message); 
      response = return_response(parse_result, &self);
      
      serial_response = serialize(parse_result, response);
      write(conn, serial_response.c_str(), serial_response.length());
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
  int port = atoi(argv[1]);
  server_info.sin_port = htons(port);
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
