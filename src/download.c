// main entrypoint for the program

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

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

    // printf( "Host: %s\n"
    //         "IP Address: %s\n"
    //         "User: %s\n"
    //         "Password: %s\n"
    //         "Resource: %s\n"
    //         "File: %s\n",
            
    //         url.hostname,
    //         url.host_ip,
    //         url.user,
    //         url.password,
    //         url.resource,
    //         url.file
    //     );
    
    int sockfd;
    struct sockaddr_in server_addr;
    size_t bytes;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(url.host_ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(21);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }
    
    char buf[1000];
    bytes = recv(sockfd, buf, 1000, 0);
    if (bytes > 0)
        printf("Bytes read %ld\n", bytes);
    else {
        perror("recv()");
        exit(-1);
    }
    printf("%s\n", buf);
    if (close(sockfd)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}
