// main entrypoint for the program

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

Url url;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Incorrect usage: ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }

    if (parse(argv[1], &url) != 0) {
        printf("Error parsing url\n");
        return -1;
    }

    return 0;
}
