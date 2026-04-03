#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include "resource_man.hpp"
#include "parser.hpp"
#include "protocol.hpp"

Response return_response(ParseResult, pthread_t);
