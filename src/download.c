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
#include <assert.h>

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

// parses the 227 message, retrieving the ip and returning the port
int parse_ip(const char* buf, char* ip) {
    // get to opening paren and skip it
    while (*buf != '(') {
        buf++;
    }
    buf++;
    int ip_index = 0;
    // copy first octet to the ip address
    ip[ip_index] = *buf;
    buf++;
    ip_index++;
    if (isdigit(*buf)) {
        ip[ip_index] = *buf;
        buf++;
        ip_index++;
        if (isdigit(*buf)) {
            ip[ip_index] = *buf;
            buf++;
            ip_index++;
        }
    }
    assert(*buf == ',');
    ip[ip_index] = '.';
    buf++;
    ip_index++;
    // copy second octet to the ip address
    ip[ip_index] = *buf;
    buf++;
    ip_index++;
    if (isdigit(*buf)) {
        ip[ip_index] = *buf;
        buf++;
        ip_index++;
        if (isdigit(*buf)) {
            ip[ip_index] = *buf;
            buf++;
            ip_index++;
        }
    }
    assert(*buf == ',');
    ip[ip_index] = '.';
    buf++;
    ip_index++;
    // copy third octet to the ip address
    ip[ip_index] = *buf;
    buf++;
    ip_index++;
    if (isdigit(*buf)) {
        ip[ip_index] = *buf;
        buf++;
        ip_index++;
        if (isdigit(*buf)) {
            ip[ip_index] = *buf;
            buf++;
            ip_index++;
        }
    }
    assert(*buf == ',');
    ip[ip_index] = '.';
    buf++;
    ip_index++;
    // copy fourth octet to the ip address
    ip[ip_index] = *buf;
    buf++;
    ip_index++;
    if (isdigit(*buf)) {
        ip[ip_index] = *buf;
        buf++;
        ip_index++;
        if (isdigit(*buf)) {
            ip[ip_index] = *buf;
            buf++;
            ip_index++;
        }
    }
    assert(*buf == ',');
    buf++;
    ip[ip_index] = '\0';

    // Now the port
    int port_oct1 = *buf - '0';
    buf++;
    if (isdigit(*buf)) {
        port_oct1 *= 10;
        port_oct1 += *buf - '0';
        buf++;
        if (isdigit(*buf)) {
            port_oct1 *= 10;
            port_oct1 += *buf - '0';
            buf++;
        }
    }
    assert(*buf == ',');
    buf++;
    // second octet
    int port_oct2 = *buf - '0';
    buf++;
    if (isdigit(*buf)) {
        port_oct2 *= 10;
        port_oct2 += *buf - '0';
        buf++;
        if (isdigit(*buf)) {
            port_oct2 *= 10;
            port_oct2 += *buf - '0';
            buf++;
        }
    }
    assert(*buf == ')');
    printf("oct1 %d oct2 %d\n", port_oct1, port_oct2);
    return port_oct1 * 256 + port_oct2;
}

size_t parse_size(const char* buf) {
    // skip code and space
    buf += 4;
    size_t size = *buf - '0';
    buf++;
    while (isdigit(*buf)) {
        size *= 10;
        size += *buf - '0';
        buf++;
    }
    return size;
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

    // send TYPE message (switching to binary mode)
    sprintf(buf, "TYPE I\r\n");
    send_message(sockfd, buf, strlen(buf));
    // receive 200 (switched to binary mode)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);

    // send SIZE message
    sprintf(buf, "SIZE %s\r\n", url.resource);
    send_message(sockfd, buf, strlen(buf));
    // receive 213 (file size in bytes)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);
    // parse the file size
    size_t file_size = parse_size(buf);
    printf("file size is %ld\n", file_size);
    
    // send PASV message
    sprintf(buf, "PASV\r\n");
    send_message(sockfd, buf, strlen(buf));
    // receive 227 (passive mode entered, plus the new address)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);

    // compute the new address
    char data_ip[] = "255.255.255.255";
    int data_port = parse_ip(buf, data_ip);
    printf("%s:%d\n", data_ip, data_port);
    
    // create a separate connection to the new address
    int data_socket;
    struct sockaddr_in data_addr;
    bzero((char *) &data_addr, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = inet_addr(data_ip);    /*32 bit Internet address network byte ordered*/
    data_addr.sin_port = htons(data_port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((data_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(data_socket,
                (struct sockaddr *) &data_addr,
                sizeof(data_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    // send RETR message
    sprintf(buf, "RETR %s\r\n", url.resource);
    send_message(sockfd, buf, strlen(buf));
    // receive 150 (transfer started)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);
    
    // read from data socket, write to the file each time
    FILE* file = fopen(url.file, "w");
    size_t total_bytes = 0;
    while (total_bytes < file_size) {
        bytes = recv(data_socket, buf, 10000, 0);
        if (bytes > 0)
            printf("Bytes read %ld\n", bytes);
            // 1==1;
        else {
            perror("recv() failed");
            exit(-1);
        }
        total_bytes += bytes;
        fwrite(buf, 1, bytes, file);
    }
    assert(total_bytes == file_size);

    // receive 226 (transfer complete)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);

    // send QUIT message
    sprintf(buf, "QUIT\r\n");
    send_message(sockfd, buf, strlen(buf));
    // receive 221 (bye)
    get_message(sockfd, buf, 10000);
    printf("%s\n", buf);

    if (close(sockfd)<0) {
        perror("close()");
        exit(-1);
    }
    if (close(data_socket)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}
