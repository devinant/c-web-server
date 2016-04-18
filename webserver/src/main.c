#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <server.h>
#include <unistd.h>
#include <signal.h>
#include "config.h"
#include "usage.h"
#include "daemon.h"
#include "util.h"
#include "logger.h"

int sigint = 1;
conf_s conf;
log_s logger;

void sigint_handler(int sig) {
    sigint = 0;
}

void jail_proc(int isd) {
    if (chdir(conf.document_root) == -1)
        printf("Could not change the root dir\n");

    if (isd) {
        if (chroot(conf.document_root) == -1)
            printf("Could not jail the process. Are you running as sudo?\n");
        else {
            // set the root directory to /
            conf.document_root[0] = '/';
            conf.document_root[1] = '\0';
            setuid(getuid());
        }
    }
}

int main(int argc, char **argv) {
    int conf_err = 0;
    int option;
    int dflag = 0;
    conf_ruleset_s rset;

    // Set the default logger to syslog
    log_syslog(&logger);

    // Initialize the config and its rulesets
    conf_init(&conf);
    conf_rset_init(&rset);

    // Set the real path
    conf_set_realpath(&conf, argv[0]);

    conf_rset_add(&rset, conf_rule_create("root = %s", &conf.document_root));
    conf_rset_add(&rset, conf_rule_create("handler = %s", &conf.method));
    conf_rset_add(&rset, conf_rule_create("port = %hu", &conf.port));
    conf_rset_add(&rset, conf_rule_create("backlog = %d", &conf.backlog));
    conf_rset_add(&rset, conf_rule_create("mime = %s", &conf.mime));
    conf_rset_add(&rset, conf_rule_create("logfile = %s", &conf.logfile));

    if (conf_read(conf.real_path, ".lab3-config", rset) == -1)
        exit_fatal("The configuration file could not be read");

    while((option = getopt(argc, argv, "hp:dl:s:")) != -1) {
        switch(option) {
            case 'p': conf_set_port(&conf, optarg); break;
            case 'd': dflag = 1; break;
            case 'l':
                conf_set_logfile(&conf, optarg);

                if (strncmp(optarg, "stdout", 6) == 0)
                    log_stdout(&logger);
                else
                    log_file(&logger);
                break;
            case 's': conf_set_method(&conf, optarg); break;
            case 'h': default: usage(argv[0]); exit(EXIT_SUCCESS);
        }
    }

    // Register signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    if ((conf_err = conf_validate(conf)) < 0)
        conf_print_err(conf_err);

    // Daemonize the process if the dflag is set
    if (dflag) {
        log_syslog(&logger);
        daemonize();
    } else {
        jail_proc(dflag);
    }

    server_create();

    return 0;
}
