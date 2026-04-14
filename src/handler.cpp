#include "handler.hpp"
#include "protocol.hpp"
#include "resource_man.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static inline sem_t *mutex_for_request(ParseResult parse_result) {
  if (parse_result.msg.command == GET || parse_result.msg.command == SET ||
      parse_result.msg.command == RESERVE ||
      parse_result.msg.command == RELEASE) {
    long id = parse_result.msg.id;
    if (id >= 0) {
      return &resource_mutexes[id & (RESOURCE_MUTEX_COUNT - 1)];
    }
  }

  return &resource_mutexes[0];
}

Response return_response(ParseResult parse_result, pthread_t *requester) {
  sem_t *request_mutex = mutex_for_request(parse_result);
  sem_wait(request_mutex);

  Response response;
  long id = parse_result.msg.id;
  char *value = parse_result.msg.value;

  if (parse_result.error == UNKNOWN_COMMAND) {
    response.status_code = 7;
  } else if (parse_result.msg.command == GET) {
    response = get(id, requester);
  } else if (parse_result.msg.command == SET) {
    response = set(id, value, requester);
  } else if (parse_result.msg.command == CREATE) {
    response = create(value, requester);
  } else if (parse_result.msg.command == RESERVE) {
    response = reserve(id, requester);
  } else if (parse_result.msg.command == RELEASE) {
    response = release(id, requester);
  } else if (parse_result.msg.command == LIST) {
    response = list();
  }

  sem_post(request_mutex);

  return response;
}
