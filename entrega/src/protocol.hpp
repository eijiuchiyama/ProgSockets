#pragma once

#include <pthread.h>
#include <string>

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
  char *value;
} Message;

typedef enum {
  NO_ERROR = 0,
  UNKNOWN_COMMAND = 1,
} ParseError;

typedef struct {
  Message msg;
  ParseError error;
} ParseResult;

const std::string OK_CODE = u8"👍";
const std::string ERROR_CODE = u8"😡";

const std::string error_codes[8] = {"",
                                    "Recurso já reservado",
                                    "Recurso inexistente",
                                    "Recurso não reservado",
                                    "",
                                    "Limite de recursos",
                                    "",
                                    "Método inexistente"};
