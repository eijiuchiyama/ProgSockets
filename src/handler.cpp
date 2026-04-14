#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
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

#define SUCCESS std::string(u8"👍")
#define ERROR   std::string(u8"😡")
static const std::string ERROR_MESSAGES[] = {
    "",
    "Recurso já reservado",
    "Recurso inexistente",
    "Recurso não reservado",
    "",
    "Limite de recursos",
    "",
    "Método inexistente"
};

static std::string serialize(ParseResult parse_result, Response response)
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


static inline sem_t *mutex_for_request(ParseResult parse_result) {
  if (parse_result.msg.command == GET || parse_result.msg.command == SET ||
      parse_result.msg.command == RESERVE ||
      parse_result.msg.command == RELEASE) {
    long id = parse_result.msg.id;
    if (id >= 0) {
      return &resource_mutexes[id % RESOURCE_MUTEX_COUNT];
    }
  }

  return &resource_mutexes[0];
}

Response return_response(ParseResult parse_result, pthread_t *requester) {
  Response response;
  long id = parse_result.msg.id;
  char *value = parse_result.msg.value;

  if (parse_result.error == UNKNOWN_COMMAND) {
    response.status_code = 7;
  }
  else if (parse_result.msg.command == CREATE) {
    response = create(value, requester);
  }
  else if (parse_result.msg.command == LIST) {
    response = list();
  }
  else {
    sem_t *request_mutex = mutex_for_request(parse_result);
    sem_wait(request_mutex);
    if (parse_result.msg.command == GET) {
      response = get(id, requester);
    } else if (parse_result.msg.command == SET) {
      response = set(id, value, requester);
    }  else if (parse_result.msg.command == RESERVE) {
      response = reserve(id, requester);
    } else if (parse_result.msg.command == RELEASE) {
      response = release(id, requester);
    }
    sem_post(request_mutex);
  }

  return response;
}

void *handler(void *args)
{
	struct ThreadArgs *t_args = (struct ThreadArgs *)args;
	int conn = t_args->conn;
	int conn_id = t_args->id;
  std::string log_file_path = t_args->log_file_path;
	free(args);

	char received_message[MAX_MESSAGE + 1];
	pthread_t self = pthread_self();
	long n;
	ParseResult parse_result;
	Response response;
	std::string serial_response;
	
  std::ofstream log_file(log_file_path, std::ios::app);
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
