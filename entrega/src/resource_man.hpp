#pragma once

#include <pthread.h>
#include <semaphore.h>

#define MAX_SIZE 30000

constexpr int RESOURCE_MUTEX_COUNT = MAX_SIZE;
extern sem_t resource_mutexes[RESOURCE_MUTEX_COUNT];
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

typedef struct {
  int id;
  char *value;
  bool reserved;
  pthread_t *belongs_to;
} Resource;

typedef struct {
  int status_code;
  int id;
  char *value;
  int count;
  Resource **list;

} Response;

void initialize();
Response create(char *, pthread_t *);
Response get(int, pthread_t *);
Response set(int, char *, pthread_t *);
Response reserve(int, pthread_t *);
Response release(int, pthread_t *);
Response list();
void release_all_from_client(pthread_t *);
