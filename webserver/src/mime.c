#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "util.h"
#include "config.h"
#include "mime.h"


void remove_newline(char *str) {
    ssize_t len = strlen(str);

    if (len > 0) str[ len - 1 ] = '\0';
}

/**
 * finds the mime in the buffer
 */
size_t strnmime(const char *word, size_t len, char *buf) {
    size_t mimelen = 0;
    char *nexttok = 0, *tok_, *tok = strtok_r(buf, " \t", &nexttok);

    while((tok_ = strtok_r(NULL, " \t", &nexttok)) && mimelen == 0) {
        if (strncmp(word, tok, len) == 0 || strncmp(word, tok_, len) == 0) {
            mimelen = strlen(tok);
        }
    }

    return mimelen;
}

/**
 * reads the mime file and retrieves the mime for a specific resource
 */
int mime_read(conf_s conf, const char *file, content_type ctype) {
    FILE *fd;
    char fbuf[PATH_MAX], buf[WCONF_MAX_LINE_BUF];

    const char *word = fext(file);
    size_t len = strlen(word);
    size_t pos = 0;

    pathcat(conf.document_root, conf.mime, fbuf);

    if ((fd = fopen(fbuf, "r")) == 0) return -1;

    while(fgets(buf, WCONF_MAX_LINE_BUF, fd) != 0 && *ctype == 0) {
        // Remove newline
        remove_newline(buf);

        if (buf[0] != '#') {
            pos = strnmime(word, len, buf);
            if (pos) {
                strncpy(ctype, buf, pos);
            }
        }
    }

    fclose(fd);

    if (pos == 0)
        strncpy(ctype, "text/plain", 11);

    return 1;
}
