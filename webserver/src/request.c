#include <string.h>
#include <stdio.h>
#include <util.h>
#include <unistd.h>
#include <stdlib.h>
#include <handle.h>
#include "request.h"
#include "response.h"

extern conf_s conf;

/**
 * initializes a request in memory
 */
void request_init(request_s *req) {
    memset(req, 0, sizeof(*req));
}

/**
 * prints a request, useful for debugging
 */
void request_print(const request_s req) {
    printf("%s %s HTTP/%s\n", req.method, req.uri, req.version);
}


/**
 * extract values from the request (method, uri and version)
 */
int req_extract(char *buf, handle_s *handle) {
    return sscanf(buf, "%s %s HTTP/%s", handle->req.method, handle->req.uri, handle->req.version);
}

/**
 * checks if a request is valid (method, uri and version is set)
 */
int request_valid(request_s req) {
    return strlen(req.method) > 0 && strlen(req.uri) > 0 && strlen(req.version) > 0;
}

/**
 * checks if a given uri is valid
 */
char* forbiddenPath[] = {"/etc/passwd", "/private/etc/passwd"};
int req_uri_valid(handle_s *handle) {
    int i;
    char realPath[PATH_MAX];
    size_t len = sizeof forbiddenPath / sizeof *forbiddenPath;

    // Get the real path.
    if (realpath(handle->req.path, realPath) == NULL){
        // perror("Realpath");
        return -1;
    }

    // Reading the mime types is forbidden.
    if (streq(&handle->req.uri[1], conf.mime)) return 0;

    // Compare real path to forbidden paths and return 0 if reqPath is invalid.
    for (i = 0; i < len; i++){
        if (strcmp(realPath, forbiddenPath[i]) == 0) {
            // Path matched with a forbidden path, path is invalid.
            return 0;
        }
    }

    return 1;
}


/**
 * the request was a GET request.
 */
int req_get(handle_s *handle) {
    int status = sendHead(handle);

    // Close the file descriptor and send the response
    if (strncmp("GET", handle->req.method, 3) == 0 && status == 200) {
        off_t offset = 0;

        // Use the OS specific implementation of sendfile
        if (sys_sendfile(handle->req.descriptor, handle->res.descriptor, &offset, &handle->res.info.st_size) == -1)
            perror("sendfile");

        close(handle->res.descriptor);
    }

    return status;
}


/**
 * the request was a head request, send the headers and close the handle
 */
int req_head(handle_s *handle) {
    int status = sendHead(handle);

    // Closes the file descriptor if its an HEAD request.
    if (strncmp("HEAD", handle->req.method, 4) == 0)
        close(handle->res.descriptor);

    return status;
}


/**
 * controller like function to handle all requests
 */
int req_controller(handle_s *handle) {
    if (request_valid(handle->req)) {
        if (streq(handle->req.version, "1.0")) {
            if (streq(handle->req.method, "GET"))           return req_get(handle);
            else if (streq(handle->req.method, "HEAD"))     return req_head(handle);
            else if (streq(handle->req.method, "POST"))     return res_501(handle);
            else if (streq(handle->req.method, "PUT"))      return res_501(handle);
            else if (streq(handle->req.method, "DELETE"))   return res_501(handle);
            else if (streq(handle->req.method, "TRACE"))    return res_501(handle);
            else if (streq(handle->req.method, "OPTIONS"))  return res_501(handle);
            else if (streq(handle->req.method, "CONNECT"))  return res_501(handle);
            else if (streq(handle->req.method, "PATCH"))    return res_501(handle);

            return res_500(handle);
        }
    }

    return res_400(handle);
}