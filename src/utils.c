#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int getHostIp(Url *url) {
    struct hostent *h;

    if ( (h = gethostbyname(url->hostname)) == NULL) {
        printf("error getting host information\n");
        return -1;
    }

    strcpy(url->host_ip, inet_ntoa(*((struct in_addr *) h->h_addr)) );

    return 0;
}
