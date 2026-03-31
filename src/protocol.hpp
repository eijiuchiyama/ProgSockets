#pragma once

#include <pthread.h>

#define MAX_MESSAGE 4096

typedef enum {
    CREATE,
    GET,
    SET,
    LIST,
    RESERVE,
    RELEASE,
} Command;

typedef struct {
  Command command;
  long id;
  char* value;
} Message;

typedef enum {
  NO_ERROR = 0,
  UNKNOWN_COMMAND = 1,
} ParseError;

typedef struct {
  Message msg;
  ParseError error;
} ParseResult;
