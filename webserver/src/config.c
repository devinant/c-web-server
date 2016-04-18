#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "config.h"
#include "util.h"

#define WCONF_ERR_DOCUMENT_ROOT -1
#define WCONF ERR_PORT -2
#define WCONF_ERR_METHOD -3
#define WCONF_ERR_MIME -5
#define WCONF_ERR_LOG -6

void conf_init(conf_s *conf) {
    memset(conf, 0, sizeof(*conf));
	//Set logfile to syslog as default
	conf_set_logfile(conf, "syslog");
}

void conf_rset_init(conf_ruleset_s *rset) {
    memset(rset, 0, sizeof(*rset));
    rset->c = 0;
}

void conf_rset_add(conf_ruleset_s *set, conf_rule_s rule) {
	if (set->c < WCONF_MAX_RULES)
		set->rules[set->c++] = rule;
}

conf_rule_s conf_rule_create(const char* rule, void *target) {
	conf_rule_s cf;

	cf.rule = rule;
	cf.target = target;

	return cf;
}

int conf_read(const char *real_path, const char *file, conf_ruleset_s rset) {
	FILE *fd;
	char fbuf[PATH_MAX];
	char buf[WCONF_MAX_LINE_BUF];

    pathcat(real_path, file, fbuf);

    if ((fd = fopen(fbuf, "r")) == 0) return -1;

	while(fgets(buf, WCONF_MAX_LINE_BUF, fd) != 0) {
		int i;
		for (i = 0;buf[0] != '#' && i < rset.c; i++) {
			if (!bit_isset(rset.flag, i) && sscanf(buf, rset.rules[i].rule, rset.rules[i].target) == 1)
				bit_set(rset.flag, i);
		}
	}

	fclose(fd);
	return 1;
}

int conf_set_realpath(conf_s* conf, char* real_path) {
	char buf[PATH_MAX];
	char* path = 0;

	if (strlen(real_path) == 0 || realpath(real_path, buf) == 0) return -1;

	// Retrieve the directory and copy it to our config
	path = dirname(buf);
	strncpy(conf->real_path, path, strlen(path));

	return 1;
}

int conf_set_port(conf_s* conf, char *_port) {
	unsigned short port = (unsigned short)strtoul(_port, NULL, 0);

	if (conf && (port > 0 && port < 65535)) {
		conf->port = port;
		return 1;
	}

	return 0;
}

int conf_set_method(conf_s *conf, char *method) {
	if (strlen(method) == 0)
		return WCONF_ERR_METHOD;

	strncpy(conf->method, method, strlen(method));

	return 1;
}

int conf_set_logfile(conf_s *conf, char *logfile){
	if (strlen(logfile) == 0)
		return WCONF_ERR_LOG;

	strncpy(conf->logfile, logfile, strlen(logfile));

	return 1;
}

/**
 * validates the supplied config
 */
int conf_validate(conf_s conf) {
    // Check if the document root is a path
	if (strlen(conf.document_root) == 0 || !ispath(conf.document_root))
		return WCONF_ERR_DOCUMENT_ROOT;

    // Check if mime types exist.
    char fbuf[PATH_MAX];
    pathcat(conf.document_root, conf.mime, fbuf);

    if (strlen(conf.mime) == 0 || !ispath(fbuf)) return WCONF_ERR_MIME;

    // Check if the method is valid
	if (strlen(conf.method) == 0)
		return WCONF_ERR_METHOD;

	const char * m[] = { "fork", "mux" };
	int i, found = 0;
	for (i = 0; i < 4 && found == 0; i++) {
		if ( strncmp(conf.method, m[i], strlen(m[i])) == 0 )
			found = 1;
	}

	if (found == 0)
		return WCONF_ERR_METHOD;

	return 1;
}

/**
 * prints the error using a error code
 */
void conf_print_err(int err_no) {
	switch(err_no) {
		case WCONF_ERR_DOCUMENT_ROOT: exit_fail("The document root is invalid"); break;
		case WCONF_ERR_METHOD: exit_fail("Invalid method handler"); break;
        case WCONF_ERR_MIME: exit_fail("The mime types file could not be found."); break;
		default: exit_fail("Fatal error when reading the configuration file."); break;
	}
}

void print_config(conf_s c) {
	printf("root: '%s', port: '%d', handler: '%s', backlog: %d\nreal_path: '%s', mime: '%s'\n", c.document_root, c.port, c.method, c.backlog, c.real_path, c.mime);
}
