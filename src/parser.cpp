#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "protocol.hpp"

char* get_line(char *input) {
  char* line = input;
  for (; *line != '\n' && *line != '\0'; line++);

  int size = line - input;

  char *result = strndup(input, size);
  return result;
}

char *next_line(char *input) {
  char* line = input;
  for (; *line != '\n' && *line != '\0'; line++);
  return line + 1;
}

ParseResult parse(char *input) {
  int N = strlen(input);
  ParseResult result;

  char *buff = (char*) malloc(N);
  strcpy(buff, input);

  char *command = get_line(buff);
  buff = next_line(buff);
  
  if (memcmp(command, "GET", 3) == 0) {
    result.msg.command = GET;
    char* id_str = get_line(buff);
    long id = strtol(id_str, NULL, 10);
    result.msg.id = id;
    result.msg.value = NULL;

  } else if (memcmp(command, "SET", 3) == 0) {
    result.msg.command = SET;
    char* id_str = get_line(buff);
    buff = next_line(buff);

    long id = strtol(id_str, NULL, 10);
    result.msg.id = id;
    
    result.msg.value = strdup(buff);

  } else if (memcmp(command, "CREATE", 6) == 0) {
    result.msg.command = CREATE;
    result.msg.value = strdup(buff);

  } else if (memcmp(command, "RESERVE", 7) == 0) {
    result.msg.command = RESERVE;
    char* id_str = get_line(buff);
    long id = strtol(id_str, NULL, 10);
    result.msg.id = id;
    result.msg.value = NULL;

  } else if (memcmp(command, "RELEASE", 7) == 0) {
    result.msg.command = RELEASE;
    char* id_str = get_line(buff);
    long id = strtol(id_str, NULL, 10);
    result.msg.id = id;
    result.msg.value = NULL;

  } else if (memcmp(command, "LIST", 4) == 0) {
    result.msg.command = LIST;
    result.msg.value = NULL;
  } else {
    result.error = UNKNOWN_COMMAND;
  }

  result.error = NO_ERROR;

  printf("[Comando:] %d\n", result.msg.command);
  printf("[ID:] %ld\n", result.msg.id);
  printf("[Valor:] %s\n", result.msg.value);

  return result;
}
