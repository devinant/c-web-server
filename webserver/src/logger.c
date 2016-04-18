#define LOG_FORMAT_CLF "%s %s %s %s \"%s %s HTTP/%s\" %i %jd"

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include "handle.h"
#include "util.h"
#include "logger.h"

// CLF - host ident authuser date request status bytes

extern conf_s conf;
extern log_s logger;

/* used for initializing the log */
void log_open_default() {}
void log_open_syslog(void) {
    openlog("httpd 1.0", LOG_PID|LOG_CONS, LOG_USER);
}

void log_close_default(void) {}

void log_doit(FILE * file, int priority, const char *fmt, va_list ap) {
    char buf[1024];

    if (file == stdout)
        printf("%d: ", priority);

    vsnprintf(buf, 1023, fmt, ap);
    strncat(buf, "\n", 1023);
    fputs(buf, file);
}

void log_write_file(int priority, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    FILE *logfile = fopen(conf.logfile, "a+");
    log_doit(logfile, priority, fmt, ap);
    if (logfile)
        fclose(logfile);
    va_end(ap);
}

void log_write_stdout(int priority, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_doit(stdout, priority, fmt, ap);
    va_end(ap);
}

void log_init(log_s* log, void *o, void *w, void *c) {
    log->open  = o;
    log->write = w;
    log->close = c;
}

void log_stdout(log_s *log) {
    log_init(log, &log_open_default, &log_write_stdout, &log_close_default);
}

void log_file(log_s *log) {
    log_init(log, &log_open_default, &log_write_file, &log_close_default);
}

// Uses the default system calls to syslog.
// logger.open  = openlog
// logger.write = syslog
// logger.close = closelog
void log_syslog(log_s *log) {
    log_init(log, &log_open_syslog, &syslog, &closelog);
}

void log_fatal(const char *fmt, ...) {
    logger.open();
    logger.write(LOG_ERR, fmt);
    logger.close();

    exit_fatal(fmt);
}

void log_req(handle_s handle) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    date_t date;

    date_now(&date, date_as_clf, localtime_r);
    getpeername(handle.req.descriptor, (struct sockaddr *)&addr, &addr_size);

    logger.open();
    logger.write(5, LOG_FORMAT_CLF, inet_ntoa(addr.sin_addr), "-", "-", date, handle.req.method,
           handle.req.uri, handle.req.version, handle.res.status, (intmax_t)handle.res.info.st_size);
    logger.close();
}
