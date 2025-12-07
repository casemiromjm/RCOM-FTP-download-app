// main entrypoint for the program

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "parser.h"

Url url;

void bytedump(char* buf) {
    while (*buf != '\0') 
        printf("%02X ", *buf++);
}

// TODO: this is kinda terrible, idk
bool is_msg_complete(const char* buf, int buf_size) {
    int i = 0;
    bool final_line = false;
    while(true) {
        if (i+4 >= buf_size && !final_line)
            return false;
        if (!final_line && (i == 0 || buf[i-1] == '\n')
            && isdigit(buf[i]) && isdigit(buf[i+1]) && isdigit(buf[i+2])
            && buf[i+3] == ' ') {
            // final, skip all until CRLF
            final_line = true;
        }
        if (i+1 >= buf_size)
            return false;
        if (final_line && buf[i] == '\r' && buf[i+1] == '\n')
            return true;
        i++;
    }
    return false;
}

// reads message pieces and writes them to buf until CRLF is encountered 
// returns the code or -1 if something's wrong (e.g. buffer too small)
int get_message(int socket, char* buf, int buf_size) {
    int total_bytes = 0;
    while (total_bytes < buf_size) {
        int bytes = recv(socket, buf + total_bytes, buf_size - total_bytes, 0);
        // bytedump(buf);
        if (bytes > 0)
            printf("Bytes read %ld\n", bytes);
        else {
            perror("recv() failed");
            exit(-1);
        }
        total_bytes += bytes;
        if (is_msg_complete(buf, total_bytes))
            break;
    }
    // printf("%s\n",buf);
    if (total_bytes >= buf_size)
        return -1;
    buf[total_bytes] = '\0';
}

void send_message(int socket, char* buf, int buf_size) {
    int bytes = send(socket, buf, buf_size, 0);
    if (bytes > 0) {
        printf("Bytes sent %ld\n", bytes);
    } else {
        perror("send()");
        exit(-1);
    }
}
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
    

    char buf[10000];
    // get welcome message
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);
    // send USER message
    sprintf(buf, "USER %s\r\n", url.user);
    send_message(sockfd, buf, strlen(buf));
    // receive 331 (request for password)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);
    // send PASS message
    sprintf(buf, "PASS %s\r\n", url.password);
    send_message(sockfd, buf, strlen(buf));
    // receive 230 (login successful)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);
    if (close(sockfd)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}
