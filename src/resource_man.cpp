#include "resource_man.hpp"
#include <bits/stdc++.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_SIZE 30000

Resource **all_resources;

int size = 0;

void initialize() {
  all_resources = (Resource **)malloc(MAX_SIZE * sizeof(Resource *));
  for (int i = 0; i < MAX_SIZE; i++) {
    all_resources[i] = NULL;
  }
}

Response new_response() {
  Response resp;
  resp.id = -1;
  resp.value = NULL;
  resp.count = -1;
  resp.list = NULL;
  return resp;
}

Response create(char *value, pthread_t *requester) {
  pthread_once(&init_once, initialize);

  Response resp = new_response();

  if (!value) {
    resp.status_code = 6;
    return resp;
  }

  if (size >= MAX_SIZE) {
    resp.status_code = 5;
    return resp;
  }

  Resource *r = (Resource *)malloc(sizeof(Resource));

  r->id = size;
  r->value = strdup(value);
  r->belongs_to = NULL;

  all_resources[size] = r;
  size++;

  resp.status_code = 0;
  resp.id = r->id;

  return resp;
}

Response get(int id, pthread_t *requester) {
  pthread_once(&init_once, initialize);

  if (id < 0 || id >= MAX_SIZE) {
    Response resp = new_response();
    resp.status_code = 2;
    return resp;
  }

  Response resp = new_response();
  if (all_resources[id] == NULL) {
    resp.status_code = 2;
    return resp;
  }
  Resource target_resource = *all_resources[id];
  if (target_resource.belongs_to == NULL) {
    resp.status_code = 3;
    return resp;
  }
  if (target_resource.belongs_to != requester) {
    resp.status_code = 1;
    return resp;
  }

  resp.status_code = 0;
  resp.value = all_resources[id]->value;

  return resp;
}

Response set(int id, char *value, pthread_t *requester) {
  pthread_once(&init_once, initialize);

  if (id < 0 || id >= MAX_SIZE) {
    Response resp = new_response();
    resp.status_code = 2;
    return resp;
  }

  Response resp = new_response();
  if (all_resources[id] == NULL) {
    resp.status_code = 2;
    return resp;
  }
  Resource target_resource = *all_resources[id];
  if (target_resource.belongs_to == NULL) {
    resp.status_code = 3;
    return resp;
  }
  if (target_resource.belongs_to != requester) {
    resp.status_code = 1;
    return resp;
  }

  all_resources[id]->value = value;
  resp.status_code = 0;

  return resp;
}

Response reserve(int id, pthread_t *requester) {
  pthread_once(&init_once, initialize);

  if (id < 0 || id >= MAX_SIZE) {
    Response resp = new_response();
    resp.status_code = 2;
    return resp;
  }

  Response resp = new_response();
  if (all_resources[id] == NULL) {
    resp.status_code = 2;
    return resp;
  }

  Resource target_resource = *all_resources[id];
  if (target_resource.belongs_to != NULL &&
      target_resource.belongs_to != requester) {
    resp.status_code = 1;
    return resp;
  }

  all_resources[id]->belongs_to = requester;
  resp.status_code = 0;

  return resp;
}

Response release(int id, pthread_t *requester) {
  pthread_once(&init_once, initialize);

  if (id < 0 || id >= MAX_SIZE) {
    Response resp = new_response();
    resp.status_code = 2;
    return resp;
  }

  Response resp = new_response();
  if (all_resources[id] == NULL) {
    resp.status_code = 2;
    return resp;
  }

  Resource target_resource = *all_resources[id];
  if (target_resource.belongs_to == NULL) {
    resp.status_code = 3;
    return resp;
  }

  if (target_resource.belongs_to != requester) {
    resp.status_code = 1;
    return resp;
  }

  all_resources[id]->belongs_to = NULL;
  resp.status_code = 0;

  return resp;
}

Response list() {
  pthread_once(&init_once, initialize);

  Response resp = new_response();

  resp.count = size;
  resp.list = all_resources;
  resp.status_code = 0;

  return resp;
}

void release_all_from_client(pthread_t *client) {
  pthread_once(&init_once, initialize);

  for (int i = 0; i < size; i++) {
    if (all_resources[i]->belongs_to == client) {
      all_resources[i]->belongs_to = NULL;
    }
  }
}
