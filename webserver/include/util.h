#ifndef WUTIL_H
#define WUTIL_H
#include <time.h>

void exit_fail(const char *format, ...);
void exit_success(const char *format, ...);
void exit_fatal(const char *format, ...);

/**
 * @brief checks if a bit is set on a bit index
 */
int bit_isset(int a, int b);

/**
 * @brief sets a bit on a bitindex
 */
void bit_set(int a, int b);

/**
 * @brief checks if a string is a path.
 */
int ispath(const char *path);

/**
 * @brief combines path a with path b and returns the result in buf
 */
void pathcat(const char *, const char *, char *);

/**
 * @brief returns the file extension of a given string. If no extension is
 * found, a empty string is returned
 */
const char *fext(const char *);

/**
 * @brief wrapper for strncmp, the second argument is used for the n in strncmp
 * streq("test", "test2") == -1
 * streq("test", "test") == 0
 */
int streq(const char *, const char *);

/**
 * @brief wrapper for sendfile on BSD and GNU systems.
 */
int sys_sendfile(int, int, off_t *, off_t*);


/**
 * date_t used in custom date functions (date_at, date_now, date_as_*)
 */
typedef char date_t[30];

/**
 * @brief sets rfc1123 compliant date
 */
void date_as_rfc1123(date_t *, struct tm *);

/**
 * @brief sets clf (common log format) date
 */
void date_as_clf(date_t *, struct tm *);

/**
 * @brief sets a date
 * @example
 *     date_t mydate;
 *     time_t t;
 *
 *     // Get the date in CLF format in GMT
 *     date_at(&mydate, t, date_as_clf, gmtime_r);
 *
 *     // Get the date in RFC1123 format in local time
 *     date_at(&mydate, t, date_as_rfc1123, localtime_r);
 */
void date_at(date_t *date, const time_t *t, void(*fn)(date_t *, struct tm *), struct tm *(*zone)(const time_t*, struct tm *));
void date_now(date_t *date, void(*as_date)(date_t *, struct tm *), struct tm *(*zone)(const time_t*, struct tm *));
#endif
