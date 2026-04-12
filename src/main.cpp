#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <semaphore.h>
#include <fstream>
#include "resource_man.hpp"
#include "parser.hpp"
#include "protocol.hpp"
#include "handler.hpp"

sem_t mutex;

struct ThreadArgs {
    int conn;
    int id;
};

std::string serialize(ParseResult parse_result, Response response, std::ofstream &file, pthread_t self)
{

	std::string error_codes[8] = {"", "Recurso já reservado", "Recurso inexistente", "Recurso não reservado", "", "Limite de recursos", "", "Método inexistente"};

	if(response.status_code != 0){
		file << "ERRO " << std::to_string(response.status_code) << " " << error_codes[response.status_code] << std::endl;
		return std::string(u8"😡​") + " " + std::to_string(response.status_code) + " " + error_codes[response.status_code] + "\n";
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
	struct ThreadArgs *t_args = (struct ThreadArgs *)args;
	int conn = t_args->conn;
	int conn_id = t_args->id;
	free(args);

	char received_message[MAX_MESSAGE + 1];
	pthread_t self = pthread_self();
	long n;
	ParseResult parse_result;
	Response response;
	std::string serial_response;
	
	std::ofstream file("log.txt", std::ios::app);
	file << "CONEXÃO #" << conn_id << std::endl;
  
    while ((n = read(conn, received_message, MAX_MESSAGE)) > 0)
    {
      received_message[n] = 0;
      printf("[#%05d][Cliente enviou:] %s", conn_id, received_message);
      file << received_message << " " << conn_id << std::endl;

      parse_result = parse(received_message); 
      response = return_response(parse_result, &self);
      
      serial_response = serialize(parse_result, response, file, self);
      write(conn, serial_response.c_str(), serial_response.length());
    }
  
    printf("[#%05d][Conexão encerrada]\n", conn_id);
    file << "DESCONEXÃO " << conn_id << std::endl;
    file.close();
    release_all_from_client(&self);

	return NULL;
}

int main(int argc, char **argv)
{
  int listen_socket, connection_socket;
  struct sockaddr_in server_info;

  if (argc != 2) {
      fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
      exit(1);
  }

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

  sem_init(&mutex, 0, 1);
  
  int connection_counter = 0;
  
  for (;;)
  {
    if ((connection_socket = accept(listen_socket, (struct sockaddr *)NULL, NULL)) == -1)
    {
      perror("erro no accept\n");
      exit(5);
    };

    pthread_t thread;

    struct ThreadArgs *args = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs));
    args->conn = connection_socket;
    args->id = ++connection_counter;
    pthread_create(&thread, NULL, handler, args);
    pthread_detach(thread);

    printf("[#%05d][Conexão aberta]\n", args->id);
  }

  exit(0);

  return 0;
}
