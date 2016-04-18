#include "usage.h"

/*
 * usage
 * prints the usage of the program
 * self - the name of the script
 */
void usage(const char* self) {
	printf("usage: %s [ -h | -p port | -d | -l file | -s]\n", self);
	println('h', "",        "print this help text");
	println('p', "port",	"runs the http server on a given port");
	println('d', "",        "runs the http server as a deamon");
	println('l', "logfile", "prints log messages to given logfile, syslog is used by default");
	println('s', "method",	"request handling method [fork | mux]");
	printf("\n");
}

/**
 * prints a usage line
 */
void println(char arg, const char* val, const char* message) {
	printf("\t -%c %-10s %s\n", arg, val, message);
}
