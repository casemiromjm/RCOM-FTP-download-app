#ifndef CONNECTION_H
#define CONNECTION_H

#include "url.h"

#define FTP_PORT 21

/**
 * Struct for handling connection
 */
typedef struct Conn {
    int ctrl_sock;
    int data_sock;
} Conn;

/**
 * Initialize a connection
 * @return 0 if succeeded, -1 otherwise
 */
int connection_init(Url url, Conn *conn);

/**
 * Closes an open connection
 * @return 0 if succeeded, -1 otherwise
 */
int close_connection();

#endif
