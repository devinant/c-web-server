#ifndef WHANDLE_H
#define WHANDLE_H

#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits.h>
#include "mime.h"

/**
 * A request struct contains information about the request
 */
typedef struct {
    int descriptor;             /* socket file descriptor from accept() */
    char method[7];             /* request method (GET, HEAD, etc. */
    char uri[2000];             /* the uri */
    char path[PATH_MAX];        /* the path to the file */
    char version[8];            /* the http version of the req */
} request_s;

/**
 * A response struct contains response specific information
 */
typedef struct {
    int descriptor;             /* file descriptor */
    int status;                 /* the resulting status code */
    struct stat info;           /* info about the file */
    content_type mime;          /* mime type */
} response_s;


/**
 * A handle contains both the request and response
 * structs.
 */
typedef struct {
    request_s req;              /* the request struct */
    response_s res;             /* the response struct */
} handle_s;

void handle_init(handle_s *handle);
#endif