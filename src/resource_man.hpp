#ifndef RESOURCE_MAN_H
#define RESOURCE_MAN_H

#include <pthread.h>

typedef struct{
  int id;
  char* value;
  bool reserved;
  pthread_t* belongs_to;
} Resource;


typedef struct{
    int status_code;
    int id;
    char* value;
    int count;
    Resource** list;

} Response; 

void initialize();
Response create(char*, pthread_t*);
Response get(int, pthread_t*);
Response set(int, char*, pthread_t*);
Response reserve(int, pthread_t*);
Response release(int, pthread_t*);
Response list();

#endif