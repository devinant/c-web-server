#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include "logger.h"
#include "daemon.h"
#include "util.h"

extern log_s logger;

void daemonize() {
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit limit;
    struct sigaction sig_action;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &limit) < 0)
        log_fatal("Cannot get file limit");

    if ((pid = fork()) < 0)
        log_fatal("Cannot fork");
    else if (pid == 0)
        exit(EXIT_SUCCESS);

    setsid();
    sig_action.sa_handler = SIG_IGN;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    if (sigaction(SIGHUP, &sig_action, NULL) < 0)
        log_fatal("Cannot SIGHUP");

    if ((pid = fork()) < 0)
        log_fatal("Cannot fork");
    else if (pid != 0)
        exit_success("pid: %d\n", pid);

    if (chdir("/") < 0)
        log_fatal("Cannot change directory to /");

    if (limit.rlim_max == RLIM_INFINITY) limit.rlim_max = 1024;

    for(i = 0; i < limit.rlim_max; i++) close(i);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        // Exiting here closes the daemon, needs investigation
        //logger.open();
        //logger.write(5, "Unexpected file descriptors (fd0: %d, fd1: %d, fd2: %d)", fd0, fd1, fd2);
        //logger.close();
        // log_fatal("Unexpected filedescriptors");
    }
}
