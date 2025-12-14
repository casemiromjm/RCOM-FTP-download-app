#include "connection.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int connection_init(Url url, Conn *conn) {
    // create sockets

    struct sockaddr_in server_addr;
    size_t bytes;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(url.host_ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(FTP_PORT);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((conn->ctrl_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    /*connect to the server*/
    if (connect(conn->data_sock,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }



}

int close_connection() {
    
    return 0;
}