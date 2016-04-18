#ifndef WRESPONSE_H
#define WRESPONSE_H
#include "handle.h"

/* struct used only for error responses */
typedef struct {
    char buf[8192];             /* buffer for responses */
    size_t len;                 /* the size of the buffer */
} err_response_s;

/**
 * initializes a response struct (memset)
 */
void res_init(response_s *);
void res_prepare(handle_s *);

/* various response codes */
int res_200(handle_s *handle);
int res_400(handle_s *handle);
int res_403(handle_s *handle);
int res_404(handle_s *handle);
int res_500(handle_s *handle);
int res_501(handle_s *handle);
int sendHead(handle_s *handle);

void response_print(response_s);
#endif