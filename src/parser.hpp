#ifndef PARSER_H
#define PARSER_H

#include "protocol.hpp"

// declarações (sem implementação)
char* get_line(char*);
char* next_line(char*);
ParseResult parse(char*, pthread_t*);

#endif