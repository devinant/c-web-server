#ifndef WSERVER_H
#define WSERVER_H

#include "config.h"
#include "handle.h"

int server_create();
int server_handle(handle_s *handle);
int server_fork(int);
int server_mux(int);
#endif