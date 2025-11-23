// main entrypoint for the program

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

Url url;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }

    if (parse(argv[1], &url) != 0) {
        printf("Error parsing url\n");
        return -1;
    }

    printf( "Host: %s\n"
            "IP Address: %s\n"
            "User: %s\n"
            "Password: %s\n"
            "Resource: %s\n"
            "File: %s\n",
            
            url.hostname,
            url.host_ip,
            url.user,
            url.password,
            url.resource,
            url.file
        );

    return 0;
}
