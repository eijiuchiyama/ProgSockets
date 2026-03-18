#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_SIZE 40

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

Resource** all_resources;

int size = 0;

void initialize(){
    all_resources = malloc(MAX_SIZE * sizeof(Response));
    for(int i = 0; i < MAX_SIZE; i++){
        all_resources[i] = NULL;
    }
}

Response new_response(){
    Response resp;
    resp.id = -1;
    resp.value = NULL;
    resp.count = -1;
    resp.list = NULL;
    return resp;
}

Response create(char* value, pthread_t* requester){
    Response resp = new_response();
    if(size == MAX_SIZE-1){
        resp.status_code = 5;
        return resp;
    }

    Resource* r = malloc(sizeof(Resource));
    r->id = size;
    size++;
    r->value = value;
    r->belongs_to = NULL;
    all_resources[size] = r;

    resp.status_code = 0;
    resp.id = r->id; 
    return resp;
}

Response get(int id, pthread_t* requester){
    Response resp = new_response();
    if(id > MAX_SIZE || all_resources[id] == NULL){
        resp.status_code = 2;
        return resp;
    }
    Resource target_resource = *all_resources[id];
    if(target_resource.belongs_to == NULL){
        resp.status_code = 3;
        return resp;
    }
    if(target_resource.belongs_to != requester){
        resp.status_code = 1;
        return resp;
    }

    resp.status_code = 0;
    resp.value = all_resources[id]->value;
    return resp;
}

Response set(int id, char* value, pthread_t* requester){
    Response resp = new_response();
    if(id > MAX_SIZE || all_resources[id] == NULL){
        resp.status_code = 2;
        return resp;
    }
    Resource target_resource = *all_resources[id];
    if(target_resource.belongs_to == NULL){
        resp.status_code = 3;
        return resp;
    }
    if(target_resource.belongs_to != requester){
        resp.status_code = 1;
        return resp;
    }

    all_resources[id]->value = value;
    resp.status_code = 0;
    return resp;
}

Response reserve(int id, pthread_t* requester){
    Response resp = new_response();
    if(id > MAX_SIZE || all_resources[id] == NULL){
        resp.status_code = 2;
        return resp;
    }

    Resource target_resource = *all_resources[id];
    if(target_resource.belongs_to != requester){
        resp.status_code = 1;
        return resp;
    }

    all_resources[id]->belongs_to = requester;
    resp.status_code = 0;
    return resp;
}

Response release(int id, pthread_t* requester){
    Response resp = new_response();
    if(id > MAX_SIZE || all_resources[id] == NULL){
        resp.status_code = 2;
        return resp;
    }

    Resource target_resource = *all_resources[id];
    if(target_resource.belongs_to == NULL){
        resp.status_code = 3;
        return resp;
    }

    if(target_resource.belongs_to != requester){
        resp.status_code = 1;
        return resp;
    }

    all_resources[id]->belongs_to = NULL;
    resp.status_code = 0;
    return resp;
}

Response list(){
    Response resp = new_response();
    
    resp.count = size;
    resp.list = all_resources;
    resp.status_code = 0;
    return resp;
}

