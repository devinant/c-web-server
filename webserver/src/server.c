#include "errno.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include "server.h"
#include "response.h"
#include "request.h"
#include "util.h"
#include "logger.h"

#define BUFSIZE 1024

extern conf_s conf;
extern int sigint;

/* Signal handler to reap zombie processes, used by server_fork */
void server_fork_wait(int sig) {
    // Wait for all child processes that terminated. WNOHANG
    // will return 0 if we are waiting for no children.
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int server_handle(handle_s *handle) {
    // Handle existing connection
    char buf[BUFSIZE];
    ssize_t nrOfBytes;
    if ((nrOfBytes = recv(handle->req.descriptor, &buf, BUFSIZE, 0)) == -1) {
        perror("recv");
        return -1;
    } else if (nrOfBytes == 0) {
        close(handle->req.descriptor);
    } else {
        // Clean up the buffer.
        // Remove newline
        if (nrOfBytes > 1 && buf[nrOfBytes - 1] == '\n' && buf[nrOfBytes - 2] == '\r') {
            buf[nrOfBytes - 2] = '\0';
        }

        req_extract(buf, handle);
        req_controller(handle);
        log_req(*handle);

        close(handle->req.descriptor);
        // return 0 if connection closes so that mux can remove it from the master set
        return 0;
    }
    return 0;
}

/**
 * creates a new server
 */
int server_create() {
    struct sockaddr_in sin;
    int sock, reuseaddr = 1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit_fatal("Unable to create socket descriptor");

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(conf.port);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) < 0) exit_fatal("Could not set SO_REUSEADDR");
    if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) == -1) exit_fatal("Unable to bind to %d", conf.port);
    if (listen(sock, conf.backlog) == -1) exit_fatal("Unable to listen");

    if (streq(conf.method, "mux")) server_mux(sock);
    if (streq(conf.method, "fork")) server_fork(sock);

    return 0;
}


/**
 * accepts clients using fork
 */
int server_fork(int sock) {
    struct sigaction sa;
    handle_s handle;
    int flags;

    /* Set up the signal handler */
    sa.sa_handler = server_fork_wait;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    // Make accept non-blocking so that we can use CTRL+C to kill the process.
    if ((flags = fcntl(sock, F_GETFL, 0)) < 0) exit_fatal("fnctl");
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) exit_fatal("fnctl");

    while (sigint) {
        handle_init(&handle);
        handle.req.descriptor = accept(sock, 0, 0);

        if (handle.req.descriptor < -1) {
            perror("accept");
            return -1;
        }

        // EAGAIN is not an error.
        if (handle.req.descriptor != -1) {
            pid_t pid;

            if ((pid = fork()) == -1) {
                close(handle.req.descriptor); // Fork failed
                return -1;
            }
            else if (pid == 0) {
                close(sock); // We are in the child process
                server_handle(&handle);
                return 0;
            } else {
                close(handle.req.descriptor); // We are in the parent
            }
        }
    }

    // Kill all child processes (if any are serving currently)
    kill(0, SIGTERM);

    return 0;
}


/**
 * accepts clients using the mux method (select)
 */
int server_mux(int sock) {
    fd_set readfds, master;
    handle_s handle;

    // Variable to keep track of the greatest file descriptor (used for loop limits)
    int fdmax, i;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_ZERO(&master);

    FD_SET(sock, &master);
    fdmax = sock;

    // Set timeout-time for select operation.
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    while (sigint) {
        handle_init(&handle);
        readfds = master;

        // Select a file descriptor to handle next
        if (select(fdmax + 1, &readfds, NULL, NULL, &tv) == -1) {
            if (errno == 4) { // Enable CTRL+C
                FD_ZERO(&master);
                FD_ZERO(&readfds);
            } else {
                perror("select");
            }
        }

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &readfds)) {
                // Check if the selected file descriptor is a new connection
                if (i == sock) {
                    if ((handle.req.descriptor = accept(sock, 0, 0)) == -1) {
                        perror("accept");
                        return -1;
                    }

                    // Add newly accepted connection to the FD_SET
                    FD_SET(handle.req.descriptor, &master);

                    // Update fdmax if necessary
                    if (handle.req.descriptor > fdmax) fdmax = handle.req.descriptor;
                } else {
                    // Set the handle
                    handle.req.descriptor = i;

                    if (server_handle(&handle) == 0)
                        FD_CLR(i, &master); //The socket was closed, remove it from the master-set
                }
            }
        }
    }

    return 0;
}