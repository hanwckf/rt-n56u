#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

int
getnext (
        char *  src,
        int     separator,
        char *  dest
        )
{
        char *  c;
        int     len;

        if ( (src == NULL) || (dest == NULL) ) {
                return -1;
        }

        c = strchr(src, separator);
        if (c == NULL) {
                strcpy(dest, src);
                return -1;
        }
        len = c - src;
        strncpy(dest, src, len);
        dest[len] = '\0';
        return len + 1;
}

int
str_to_mac (
        unsigned char * mac,
        char *          str
        )
{
        int             len;
        char *          ptr = str;
        char            buf[128];
        int             i;

        for (i = 0; i < 5; i++) {
                if ((len = getnext(ptr, ':', buf)) == -1) {
                        return 1; /* parse error */
                }
                mac[i] = strtol(buf, NULL, 16);
                ptr += len;
        }
        mac[5] = strtol(ptr, NULL, 16);

        return 0;
}

int
str_to_ip (
        unsigned long * ip,
        char *          str
        )
{
        int             len;
        char *          ptr = str;
        char            buf[128];
        unsigned char   c[4];
        int             i;

        for (i = 0; i < 3; ++i) {
                if ((len = getnext(ptr, '.', buf)) == -1) {
                        return 1; /* parse error */
                }
                c[i] = atoi(buf);
                ptr += len;
        }
        c[3] = atoi(ptr);
        *ip = (c[0]<<24) + (c[1]<<16) + (c[2]<<8) + c[3];
        return 0;
}


