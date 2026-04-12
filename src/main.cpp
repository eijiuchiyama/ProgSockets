#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <string>
#include <fstream>
#include <iostream>

#include "resource_man.hpp"
#include "parser.hpp"
#include "protocol.hpp"
#include "handler.hpp"
#include "logger.hpp"


sem_t mutex;

struct ThreadArgs {
  int conn;
  int id;
};

#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_GRAY   "\033[90m"

#define SUCCESS std::string(u8"👍")
#define ERROR   std::string(u8"😡")
const std::string ERROR_MESSAGES[] = {
    "",
    "Recurso já reservado",
    "Recurso inexistente",
    "Recurso não reservado",
    "",
    "Limite de recursos",
    "",
    "Método inexistente"
};


std::string serialize(ParseResult parse_result, Response response)
{
	if(response.status_code != 0)
		return ERROR + " " + std::to_string(response.status_code) + " " + ERROR_MESSAGES[response.status_code] + "\n";
  
  switch (parse_result.msg.command)
  {
    case CREATE:
	    return SUCCESS + " " + std::to_string(response.id) + "\n";
    case GET:
      return SUCCESS + " " + response.value + "\n";
    case SET:
      return SUCCESS + "\n";
    case RESERVE:
      return SUCCESS + "\n";
    case RELEASE:
      return SUCCESS + "\n";
    case LIST: {
      std::string s = SUCCESS + " " + std::to_string(response.count);
      for(int i = 0; i < response.count; i++)
        s += " " + std::to_string(response.list[i]->id);
      s += "\n";
      return s;
    }
    default:
      return "";
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
	
	std::ofstream log_file("log.txt", std::ios::app);
	log_connect(log_file, conn_id);
  
    while ((n = read(conn, received_message, MAX_MESSAGE)) > 0)
    {
      received_message[n] = 0;
      std::string req(received_message);
      log_request(log_file, conn_id, req);
      
      parse_result = parse(received_message); 
      response = return_response(parse_result, &self);

      serial_response = serialize(parse_result, response);
      write(conn, serial_response.c_str(), serial_response.length());
      
      log_answer(log_file, conn_id, response.status_code, serial_response);
    }
  
    log_disconnect(log_file, conn_id);
    log_file.close();
    release_all_from_client(&self);

	return nullptr;
}

int main(int argc, char **argv)
{
  int listen_socket, connection_socket;
  struct sockaddr_in server_info;

  if (argc != 2) {
      std::cerr << "Uso: " << argv[0] << " <Porta>\n";
      exit(1);
  }

  if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("erro ao criar socket\n");
    exit(2);
  }

  int opt = 1;
  if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
  }


  bzero(&server_info, sizeof(server_info));
  server_info.sin_family = AF_INET;
  server_info.sin_addr.s_addr = htonl(INADDR_ANY);
  int port = atoi(argv[1]);
  server_info.sin_port = htons(port);
  if ((bind(listen_socket, (struct sockaddr *)&server_info, sizeof(server_info)) == -1)) {
    perror("erro no bind\n");
    exit(3);
  }

  if (listen(listen_socket, 1) == -1) {
    perror("erro no listen\n");
    exit(4);
  }

  std::cout << COLOR_YELLOW << "[Servidor no ar. Aguardando conexões na porta " << argv[1] << "]\n" << COLOR_RESET;

  sem_init(&mutex, 0, 1);
  
  int connection_counter = 0;
  
  for (;;)
  {
    if ((connection_socket = accept(listen_socket, (struct sockaddr *)nullptr, nullptr)) == -1) {
      perror("erro no accept\n");
      exit(5);
    };

    pthread_t thread;

    struct ThreadArgs *args = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs));
    args->conn = connection_socket;
    args->id = ++connection_counter;
    pthread_create(&thread, nullptr, handler, args);
    pthread_detach(thread);
  }

  exit(0);

  return 0;
}
