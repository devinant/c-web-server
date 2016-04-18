#ifndef WMIME_H
#define WMIME_H

#include "config.h"

typedef char content_type[64];

/**
 * uses the config file to find a mime type for a given file
 * returns -1 when the config file could not be opened
 * When a mime was found, the output is copied to the last arg, else the last arg is
 * set to 'text/plain'
 *
 * @example
 *  conf myconf;
 *  char mymime[100];
 *
 *  if (mime_read(myconf, "hello-world.html", mymime))
 *      printf("content type is: %s", mymime);
 */
int mime_read(conf_s, const char *, content_type);
#endif