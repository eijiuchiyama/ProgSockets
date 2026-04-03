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
#include "handler.hpp"

Response return_response(ParseResult parse_result, pthread_t* requester){
	Response response;
	long id = parse_result.msg.id;
	char* value = parse_result.msg.value;
	
	if(parse_result.error == UNKNOWN_COMMAND){
		response.status_code = 7;
	} else if(parse_result.msg.command == GET){
		response = get(id, requester);
	} else if(parse_result.msg.command == SET){
		response = set(id, value, requester);
	} else if(parse_result.msg.command == CREATE){
		response = create(value, requester);
	} else if(parse_result.msg.command == RESERVE){
		response = reserve(id, requester);
	} else if(parse_result.msg.command == RELEASE){
		response = release(id, requester);
	} else if(parse_result.msg.command == LIST){
		response = list();
	}
	
	return response;
}
