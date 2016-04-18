#ifndef WLOGGER_H
#define WLOGGER_H
#include "handle.h"

void log_req(handle_s handle);
void log_fatal(const char *fmt, ...);

typedef struct {
    void (*open)();                             /* function to call when opening the log */
    void (*write)(int, const char *, ...);      /* function to call when logging a message to the log */
    void (*close)(void);                        /* function to call when closing a log */
} log_s;

void log_init(log_s *, void *, void *, void *);

/* use stdout as output */
void log_stdout(log_s *);

/* use a log file as output */
void log_file(log_s *);

/* yse syslog as output */
void log_syslog(log_s *);
#endif
