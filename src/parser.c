#include "parser.h"

#include <string.h>
#include <stdio.h>

#include "utils.h"

int parse(char *raw_url, Url *url) {

    // pointer to update curr url element while preserving original string
    char *ptr = raw_url;
    
    if (strncmp(ptr, "ftp://", 6) != 0) {
        printf("Invalid protocol in url\n");
        return -1;
    }

    ptr += 6;

    char *at = strchr(ptr, '@');

    if (at != NULL) {
        char *colon = strchr(ptr, ':');

        // get user
        strncpy(url->user, ptr, colon-ptr);
        url->user[colon-ptr] = '\0';
        ptr += colon-ptr+1;
        
        // get password
        strncpy(url->password, ptr, at-ptr);
        url->user[at-ptr] = '\0';
        ptr += at-ptr+1;
    } else {
        strcpy(url->user, "anonymous");
        strcpy(url->password, "password");
    }

    char *slash = strchr(ptr, '/');
    strncpy(url->hostname, ptr, slash-ptr);
    url->hostname[slash-ptr] = '\0';
    ptr += slash-ptr;   // dont skip the / char, because it can cause problem if the resource is the file itself

    strcpy(url->resource, ptr);

    if (getHostIp(url) != 0) return -1;

    // creates a pointer to the last occurence of /, then copies the file name to url struct
    char *f = strrchr(ptr, '/');
    strcpy(url->file, f+1);

    return 0;
}
