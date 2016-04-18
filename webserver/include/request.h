#ifndef WREQUEST_H
#define WREQUEST_H

#include "config.h"
#include "handle.h"

void request_init(request_s *);
void request_print(const request_s);

int req_uri_valid(handle_s *handle);
int req_extract(char *buf, handle_s *handle);
int req_controller(handle_s *handle);

#endif