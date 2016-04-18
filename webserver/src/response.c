#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <time.h>
#include "util.h"
#include <sys/errno.h>
#include <sys/fcntl.h>
#include "config.h"
#include "request.h"
#include "handle.h"
#include "response.h"

#define ERR "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">" \
            "<head><meta http-equiv=\"Content-Type\" content=\"%s; charset=utf-8\"><title>%d: %s</title></head>" \
            "<body style=\"text-align: center\"><h1>%d %s</h1><p>%s</p><hr /><p><em>httpd 1.0 (%s %s %s)</em></p></body>" \
            "</html>"

extern conf_s conf;

/**
 * returns information about the os running on this server
 */
struct utsname get_server_os() {
    struct utsname name;
    uname(&name);

    return name;
}

/**
 * initializes a response struct
 */
void res_init(response_s *res) {
    memset(res, 0, sizeof(*res));
}

/**
 * Removes all instances of string remove in a another string
 */
void uri_safestring(char *s, const char *remove) {
    size_t len = strlen(remove);

    while((s = strstr(s, remove)))
        memmove(s, s + len, 1 + strlen(s + len));
}

/**
 * Sends a header message to the socket descriptor.
 * returns status code.
 */
int sendHead(handle_s *handle) {
    res_init(&handle->res);

    // Remove all occurrences of "../" from the URI.
    uri_safestring(handle->req.uri, "../");

    // Make sure GET / maps to GET /index.html
    if (strlen(handle->req.uri) == 1 && strncmp(handle->req.uri, "/", 1) == 0)
        strncpy(handle->req.uri, "/index.html", 12);

    // the full path will be mapped using document root + the resource
    pathcat(conf.document_root, &handle->req.uri[1], handle->req.path);

    int validPath = req_uri_valid(handle);

    // Check if requested path is invalid
    if (validPath == 0)
        return res_403(handle); // Path Invalid

    else if (validPath == -1)
        return res_404(handle); // Path not found.

    if ((handle->res.descriptor = open(handle->req.path, O_RDONLY)) == -1)
        return res_500(handle); // File could not be opened even when the path is valid.

    if (errno == EACCES)
        return res_403(handle);

    // Prepare the request for further processing
    res_prepare(handle);

    return res_200(handle);
}

/**
 * prepares a response to the server by reading the mime type of the file
 * and getting file status.
 */
void res_prepare(handle_s *handle) {
    mime_read(conf, handle->req.path, handle->res.mime);
    fstat(handle->res.descriptor, &handle->res.info);
}

/**
 * variadic function for sending data to a socket, much like printf.
 */
ssize_t sendf(int sock, const char *fmt, ...) {
    char buf[8192];

    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, 8192, fmt, va);
    va_end(va);

    return send(sock, buf, strlen(buf), 0);
}

/**
 * variadic function used before sending any data to a socket.
 * returns the err_response_s struct with calculated length of the
 * buffer and the buffer itself.
 */
err_response_s before_sendf(const char *fmt, ...) {
    err_response_s respond;
    memset(&respond, 0, sizeof(respond));

    va_list va;
    va_start(va, fmt);
    vsnprintf(respond.buf, 8192, fmt, va);
    va_end(va);

    respond.len = strlen(respond.buf);

    return respond;
}


/*
 * STATUS HEADERS
 * All status headers end with \r\n (CRLF) as defined by the HTTP/1.0 spec.
 */


/**
 * send the status header to the client
 * format: HTTP/<version> <code> <reason>
 */
void head_status(const handle_s handle, int code, const char *reason) {
    sendf(handle.req.descriptor, "HTTP/%s %d %s\r\n", handle.req.version, code, reason);
}


/**
 * sends the current date to the client in GMT time as defined in rfc 1945.
 * the actual date type is defined by rfc 1123.
 * format: "Date: <date> GMT"
 */
void head_date(const handle_s handle) {
    date_t date;
    date_now(&date, date_as_rfc1123, gmtime_r);
    sendf(handle.req.descriptor, "Date: %s\n", date);
}


/**
 * sends the last modified header to the client in GMT time as defined in rfc 1945.
 * the actual date type is defined by rfc 1123
 * format: "Last Modified: <date> GMT"
 */
void head_last_modified(const handle_s handle) {
    date_t last_modified;
    #if defined(__linux__)
    date_at(&last_modified, &handle.res.info.st_mtime, date_as_rfc1123, gmtime_r);
    #endif

    #if defined(__APPLE__) && defined(__MACH__)
    date_at(&last_modified, &handle.res.info.st_ctimespec.tv_sec, date_as_rfc1123, gmtime_r);
    #endif
    sendf(handle.req.descriptor, "Last-Modified: %s\r\n", last_modified);
}


/**
 * sends the content type to the client (via mimes)
 * format: "Content-Type: <mime>"
 */
void head_content_type(const handle_s handle) {
    sendf(handle.req.descriptor, "Content-Type: %s\r\n", handle.res.mime);
}


/**
 * sends the content length to the client in bytes
 * format: "Content-Length: <bytes>"
 */
void head_content_length(const handle_s handle) {
    sendf(handle.req.descriptor, "Content-Length: %ld\r\n", handle.res.info.st_size);
}


/**
 * sends information about the server to the client
 * format: "Server: <message>"
 */
void head_server(const handle_s handle) {
    struct utsname os = get_server_os();
    sendf(handle.req.descriptor, "Server: httpd 1.0 (%s %s %s)\r\n", os.sysname, os.release, os.machine);
}


/**
 * sends the connection header to the client.
 * currently only "close"
 * format: "Connection: Close"
 */
void head_connection(const handle_s handle, const char* type) {
    sendf(handle.req.descriptor, "Connection: %s\r\n", type);
}

/**
 * not an actual header, but a separator between the content body and headers of the file.
 * required by the spec
 */
void head_newline(const handle_s handle) {
    sendf(handle.req.descriptor, "\r\n");
}


/**
 * a default header used by all request types
 */
void res_default(const handle_s handle, int code, const char *reason) {
    head_status(handle, code, reason);
    head_connection(handle, "Close");
    head_server(handle);
    head_date(handle);
    head_content_type(handle);
    head_content_length(handle);
}


/**
 * constructs all headers for errors using the arguments supplied below.
 */
int res_error(handle_s *handle, int code, const char *reason, const char *req_body) {
    struct utsname name = get_server_os();

    // set the status code to what we have been given
    handle->res.status = code;

    // set the mime to text/html
    strncpy(handle->res.mime, "text/html", 10);

    // create the error message
    err_response_s re = before_sendf(ERR, handle->res.mime, code, reason, code, reason, req_body, name.sysname, name.release, name.machine);
    handle->res.info.st_size = (off_t)re.len;

    res_default(*handle, code, reason);
    head_newline(*handle);

    send(handle->req.descriptor, re.buf, re.len, 0);

    return handle->res.status;
}

/* sends headers related to status code 200 */
int res_200(handle_s *handle) {
    handle->res.status = 200;
    res_default(*handle, 200, "OK");
    head_last_modified(*handle);
    head_newline(*handle);

    return handle->res.status;
}


/**
 * uses res_error to construct headers for status 400
 */
int res_400(handle_s *handle) {
    return res_error(handle, 400, "Bad Request", "The request could not be understood by the server due to malformed syntax.");
}


/**
 * uses res_error to construct headers for status 400
 */
int res_403(handle_s *handle) {
    return res_error(handle, 403, "Forbidden", "The server understood the request, but is refusing to fulfill it.");
}


/**
 * uses res_error to construct headers for status 400
 */
int res_404(handle_s *handle) {
    return res_error(handle, 404, "Not Found", "The requested resource could not be found.");
}


/**
 * uses res_error to construct headers for status 400
 */
int res_500(handle_s *handle) {
    return res_error(handle, 500, "Internal Server Error", "The server encountered an unexpected condition which prevented it from fulfilling the request.");
}


/**
 * uses res_error to construct headers for status 400
 */
int res_501(handle_s *handle) {
    return res_error(handle, 501, "Not Implemented", "The server does not support the functionality required to fulfill the request.");
}


/**
 * printing of response structs, valuable for debugging
 */
void response_print(response_s res) {
    printf("Response: desc: '%d', mime: '%s', size: '%ld'\n", res.descriptor, res.mime, (long)res.info.st_size);
}
