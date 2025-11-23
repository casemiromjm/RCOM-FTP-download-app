#include "parser.h"

#include <string.h>
#include <stdio.h>

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
        ptr += colon-ptr+1;
        
        // get password
        strncpy(url->password, ptr, at-ptr);
        ptr += at-ptr+1;
    } else {
        strcpy(url->user, "anonymous");
        strcpy(url->password, "password");
    }

        
    
    return 0;
}
