#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <limits.h>
#include <time.h>
#include "util.h"

#if defined(__linux__)
#include <sys/sendfile.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/socket.h>
#endif

/**
 * checks if a bit was set
 */
int bit_isset(int a, int b) {
    return (a & 1 << b) != 0;
}

/**
 * sets a bit
 */
void bit_set(int a, int b) {
    a |= 1 << b;
}

/* example from book */
static void doit(int errnoflag, int error, const char *fmt, va_list ap) {
    char buf[1024];

    vsnprintf(buf, 1023, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), 1024 - strlen(buf) - 1, ": %s", strerror(error));

    strcat(buf, "\n");
    fflush(stdout);
    fputs(buf, stderr);
    fflush(0);
}

/**
 * error exit with a supplied message.
 * no errno is set
 */
void exit_fail(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    doit(0,0,format, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

/**
 * successful exit with a supplied message
 */
void exit_success(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    doit(0,0,format, ap);
    va_end(ap);

    exit(EXIT_SUCCESS);
}

/**
 * fatal error with a supplied message.
 * exits the program and prints the errno
 */
void exit_fatal(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    doit(1, errno, format, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

/*
 * checks if a path is a path
 */
int ispath(const char *path) {
    struct stat s;
    return stat(path, &s) == 0;
}

/**
 * combines two strings to form a path and outputs the result into a buffer
 */
void pathcat(const char *a, const char* b, char *buf) {
    size_t al = strlen(a);
    size_t bl = strlen(b);

    if (al > 0 && bl > 0) {
        if (a[al - 1] == '/')
            snprintf(buf, PATH_MAX, "%s%s", a, b);
        else
            snprintf(buf, PATH_MAX, "%s/%s", a, b);
    }
}

/**
 * retrieves the file extension from a file
 */
const char *fext(const char *file) {
    const char *dot = strrchr(file, '.');
    if (!dot || file == dot) return "";
    return dot + 1;
}

/**
 * shorthand for strncmp
 */
int streq(const char *a, const char *b) {
    return strncmp(a, b, strlen(b)) == 0;
}

/**
 * abstraction of sendfile for various operating systems.
 * works currently for osx, ubuntu and debian.
 */
int sys_sendfile(int in_fd, int out_fd, off_t* offset, off_t *size) {
    #if defined(__APPLE__) && defined(__MACH__)
    int s = sendfile(out_fd, in_fd, *offset, size, NULL, 0);
    if (s == 0) return (int)*size;
    return -1;
    #endif

    #if defined(__linux__)
    ssize_t sf = sendfile(in_fd, out_fd, offset, *size);
    return (int) sf;
    #endif
}



/**
 * DATES
 */

/**
 * returns a rfc1123 compliant date using a tm struct
 */
void date_as_rfc1123(date_t *date, struct tm *tm) {
    const char *DAY_NAMES[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    const char *MON_NAMES[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    strftime(*date, 30, "---, %d --- %Y %H:%M:%S GMT", tm);
    memcpy(date,      DAY_NAMES[tm->tm_wday], 3);
    memcpy(*date + 8, MON_NAMES[tm->tm_mon],  3);
}

/**
 * returns a clf (common log format) compliant date using a tm struct
 */
void date_as_clf(date_t *date, struct tm *tm) {
    const char *MON_NAMES[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    strftime(*date, 30, "[%d/---/%Y:%H:%M:%S %z]", tm);
    memcpy(*date + 4, MON_NAMES[tm->tm_mon],  3);
}

/**
 * retrieves a date at a specific time, and outputs the result into a buffer.
 * the first functor refers the a formatting method (for example date_as_rfc and date_as_clf)
 * the second functor refers to a zone, use localtime_r or gmtime_r
 */
void date_at(date_t *date, const time_t *t, void(*fn)(date_t *, struct tm *), struct tm *(*zone)(const time_t*, struct tm *)) {
    struct tm tm;
    zone(t, &tm);
    fn(date, &tm);
}

/**
 * retrieves the current time, and outputs the result into a buffer.
 * @see date_at for information about the first and second functor.
 */
void date_now(date_t *date, void(*as_date)(date_t *, struct tm *), struct tm *(*zone)(const time_t*, struct tm *)) {
    time_t t;
    time(&t);
    date_at(date, &t, as_date, zone);
}
