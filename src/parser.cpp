#include <bits/stdc++.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "protocol.hpp"
#include "resource_man.hpp"

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
  int debug = 0;

  ParseResult result;

  char *buff = strdup(input); // copia segura

  // pega comando
  char *command = strtok(buff, " \n");

  if (!command) {
    result.error = UNKNOWN_COMMAND;
    return result;
  }

  if (strcmp(command, "GET") == 0) {
    result.msg.command = GET;

    char *id_str = strtok(NULL, " \n");
    result.msg.id = id_str ? strtol(id_str, NULL, 10) : 0;
    result.msg.value = NULL;

  } else if (strcmp(command, "SET") == 0) {
    result.msg.command = SET;

    char *id_str = strtok(NULL, " \n");
    char *value = strtok(NULL, "\n");

    result.msg.id = id_str ? strtol(id_str, NULL, 10) : 0;
    result.msg.value = value ? strdup(value) : NULL;

  } else if (strcmp(command, "CREATE") == 0) {
    result.msg.command = CREATE;

    char *value = strtok(NULL, "\n");
    result.msg.id = 0;
    result.msg.value = value ? strdup(value) : NULL;

  } else if (strcmp(command, "RESERVE") == 0) {
    result.msg.command = RESERVE;

    char *id_str = strtok(NULL, " \n");
    result.msg.id = id_str ? strtol(id_str, NULL, 10) : -1;
    result.msg.value = NULL;

  } else if (strcmp(command, "RELEASE") == 0) {
    result.msg.command = RELEASE;

    char *id_str = strtok(NULL, " \n");
    result.msg.id = id_str ? strtol(id_str, NULL, 10) : -1;
    result.msg.value = NULL;

  } else if (strcmp(command, "LIST") == 0) {
    result.msg.command = LIST;
    result.msg.id = 0;
    result.msg.value = NULL;

  } else {
    result.error = UNKNOWN_COMMAND;
    return result;
  }

  result.error = NO_ERROR;

  if (debug) {
    printf("        [Comando:] %d\n", result.msg.command);
    printf("        [ID:] %ld\n", result.msg.id);
    printf("        [Valor:] %s\n", result.msg.value ? result.msg.value : "(null)");
  }

  return result;
}
