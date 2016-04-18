#ifndef WCONFIG_H
#define WCONFIG_H

#include <limits.h>

#define WCONF_MAX_RULES 32
#define WCONF_MAX_LINE_BUF 1024
#define WCONF_MAX_VAL 64

/**
 * holds information about several server properties as defined
 * in the config file
 */
typedef struct {
    char document_root[WCONF_MAX_VAL];      /* path to the files which the server is supposed to serve */
    char method[WCONF_MAX_VAL];             /* method for serving clients */
    unsigned short port;                    /* port to listen on */
    int backlog;                            /* backlog */
    char real_path[PATH_MAX];               /* the absolute path to the server binary */
    char mime[WCONF_MAX_VAL];               /* name of the mime file */
    char logfile[WCONF_MAX_VAL];            /* default logging method */
} conf_s;

/**
 * holds a single rule (later to be used with a ruleset
 */
typedef struct {
    const char* rule;                       /* string used by sscanf */
    void* target;                           /* address to the target when the rule is matched */
} conf_rule_s;


/**
 * holds all rules for reading data from the config file
 */
typedef struct {
    int c;                                  /* the number of rules available in this ruleset */
    int flag;                               /* bitarray of flags (if the rule was exec'ed or not */
    conf_rule_s rules[WCONF_MAX_RULES];     /* the rules used in the set, should correspond to c */
} conf_ruleset_s;

/**
 * @brief initializes the config
 */
void conf_init(conf_s *);


/**
 * @brief initializes a ruleset
 */
void conf_rset_init(conf_ruleset_s *);


/**
 * @brief adds a rule to a ruleset
 */
void conf_rset_add(conf_ruleset_s *, conf_rule_s);

/**
 * @brief creates a rule
 */
conf_rule_s conf_rule_create(const char *, void *);


/**
 * @brief reads a config file and applies the given ruleset
 */
int conf_read(const char *, const char *, conf_ruleset_s);

/**
 * @brief sets the real path to a conf_s
 */
int conf_set_realpath(conf_s *, char *);

/**
 * @brief sets the conf port from a char*
 */
int conf_set_port(conf_s *, char *);

/**
 * @brief sets the conf method from a char*
 */
int conf_set_method(conf_s *, char *);

/**
 * @brief sets the conf logfile from a char*
 */
int conf_set_logfile(conf_s *, char *);

/**
 * @brief validates the configuration file
 */
int conf_validate(conf_s);

/**
 * @brief print the error using a error code
 */
void conf_print_err(int);

/**
 * @brief prints the contents of the config struct
 */
void print_config(conf_s);

#endif
