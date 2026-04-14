#include "handler.hpp"
#include "parser.hpp"
#include "protocol.hpp"
#include "resource_man.hpp"
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
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


sem_t resource_mutexes[RESOURCE_MUTEX_COUNT];

#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_GRAY   "\033[90m"

int main(int argc, char **argv) {
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

  std::string run_log_file = get_timestamp_string() + "_log.txt";

  for (int i = 0; i < RESOURCE_MUTEX_COUNT; i++) {
    sem_init(&resource_mutexes[i], 0, 1);
  }
  
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
    snprintf(args->log_file_path, sizeof(args->log_file_path), "%s", run_log_file.c_str());
    pthread_create(&thread, nullptr, handler, args);
    pthread_detach(thread);
  }

  exit(0);

  return 0;
}
