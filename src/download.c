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
#include "ftp_response.h"

#define FTP_PORT 21
#define MAX_BUF_SIZE 10000

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

/**
 * Reads message pieces and writes them to buf until CRLF is encountered
 * @param socket Socket being read
 * @param buf Where the message is stored
 * @param buf_size Buffer size
 * @return the response code or -1 if something's wrong (e.g. buffer too small)
 */
int get_message(int socket, char* buf, int buf_size) {
    int total_bytes = 0;

    while (total_bytes < buf_size) {
        long int bytes = recv(socket, buf + total_bytes, buf_size - total_bytes, 0);
        if (bytes > 0) {
            printf("Bytes read %ld\n", bytes);
        }
        else {
            perror("recv() failed");
            return -1;
        }

        total_bytes += bytes;
        if (is_msg_complete(buf, total_bytes)) {
            break;
        }
    }

    if (total_bytes >= buf_size)
        return -1;

    buf[total_bytes] = '\0';

    // get message code
    int res = (buf[0] - '0')*100 + (buf[1] - '0')*10 + (buf[2] - '0');
    return res;
}

/**
 * Sends a message via socket
 * @param socket Socket used to send a message
 * @param buf Where message was
 * @param buf_size Buffer size
 * @return 0 if succeeded, -1 otherwise
 */
int send_message(int socket, char* buf, int buf_size) {
    long int bytes = send(socket, buf, buf_size, 0);
    if (bytes > 0) {
        printf("Bytes sent %ld\n", bytes);
    } else {
        perror("send()");
        return -1;
    }

    return 0;
}

/**
 * Parses the 227 message (PASSIVE MODE ON)
 * @param buf Message to be parsed
 * @param ip New ip
 * @return Port for data socket
 */
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

size_t get_file_size(const char* buf) {
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

    Url url;

    // get information from param
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
    
    // creating the ctrl socket

    // ctrl_socket
    int sockfd;
    struct sockaddr_in server_addr;
    size_t bytes;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(url.host_ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(FTP_PORT);        /*server TCP port must be network byte ordered */

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
    

    char buf[MAX_BUF_SIZE];
    // get welcome message
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != WELCOME_CODE) {
        printf("Error: did not receive welcome message.\n");
        return -1;
    }
    printf("%s\n", buf);
    
    // send USER message for authentication
    sprintf(buf, "USER %s\r\n", url.user);
    if (send_message(sockfd, buf, strlen(buf)) != 0) {
        printf("Failed sending USER message.\n");
        return -1;
    }
    // (try to) receive 331 (request for password)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != PSSWRD_REQUEST_CODE) {
        printf("Error: did not receive password request\n");
        return -1;
    }
    printf("%s\n", buf);
    
    // send PASS message
    sprintf(buf, "PASS %s\r\n", url.password);
    if (send_message(sockfd, buf, strlen(buf)) != 0) {
        printf("Error: could not send the PASS\n");
        return -1;
    }
    // receive 230 (login successful)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != LOGIN_SUCCESS_CODE) {
        printf("Error: couldn't login\n");
        return -1;
    }
    printf("%s\n", buf);

    // send TYPE message (switching to binary mode)
    sprintf(buf, "TYPE I\r\n");
    if (send_message(sockfd, buf, strlen(buf)) != 0) {
        printf("Error: couldn't switch to binary mode\n");
        return -1;
    }
    // receive 200 (switched to binary mode)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != CMD_OK_CODE) {
        printf("Error: couldn't switch to binary mode\n");
        return -1;
    }
    printf("%s\n", buf);

    // send SIZE message
    sprintf(buf, "SIZE %s\r\n", url.resource);
    if (send_message(sockfd, buf, strlen(buf)) != 0) {
        printf("Error: couldn't send SIZE message\n");
        return -1;
    }
    // receive 213 (file size in bytes)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != FILE_STAT_CODE) {
        printf("Error: couldn't get file size\n");
        return -1;
    }
    printf("%s\n", buf);
    // parse the file size
    size_t file_size = get_file_size(buf);
    printf("file size is %ld\n", file_size);
    
    // send PASV message
    sprintf(buf, "PASV\r\n");
    if (send_message(sockfd, buf, strlen(buf))) {
        printf("Error: couldn't send PASV message\n");
    }
    // receive 227 (passive mode entered, plus the new address)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != PASSIVE_MODE_CODE) {
        printf("Error: couldn't enter in PASSIVE mode\n");
        return -1;
    }
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
        printf("Error: can't create data socket\n");
        return -1;
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
    if (send_message(sockfd, buf, strlen(buf)) != 0) {
        printf("Error: couldn't send RETR message\n");
        return -1;
    }
    // receive 150 (transfer started)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != OPEN_DATA_CONNECTION_CODE) {
        printf("Error: couldn't start transfer\n");
        return -1;
    }
    printf("%s\n", buf);
    
    // read from data socket, write to the file each time
    FILE* file = fopen(url.file, "w");
    size_t total_bytes = 0;
    while (total_bytes < file_size) {
        bytes = recv(data_socket, buf, MAX_BUF_SIZE, 0);
        if (bytes > 0)
            printf("Bytes read %ld\n", bytes);
            // 1==1;
        else {
            perror("recv() failed");
            return -1;
        }
        total_bytes += bytes;
        fwrite(buf, 1, bytes, file);
    }
    if (total_bytes != file_size) {
        printf("Error: transfer was problematic\n");
        return -1;
    }

    // receive 226 (transfer complete)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != TRANSFER_COMPLETE_CODE) {
        printf("Error: transfer was not done\n");
        return -1;
    }
    printf("%s\n", buf);

    // send QUIT message
    sprintf(buf, "QUIT\r\n");
    if (send_message(sockfd, buf, strlen(buf)) != 0) {
        printf("Error: couldn't send QUIT message");
        return -1;
    }
    // receive 221 (bye)
    if (get_message(sockfd, buf, MAX_BUF_SIZE) != BYE_CODE) {
        printf("Error: couldn't get bye\n");
        return -1;
    }
    printf("%s\n", buf);

    // close connectios

    if (close(sockfd) < 0) {
        printf("Error: couldn't close ctrl_socket\n");
        return -1;
    }
    if (close(data_socket) < 0) {
        printf("Error: couldn't close data_socket\n");
        return -1;
    }

    return 0;
}
