#ifndef CONNECTION_H
#define CONNECTION_H

#include "url.h"

/**
 * Struct for handling connection
 */
typedef struct Conn {
    int fdSockA;
    int fdSockB;
} Conn;

/**
 * Opens a connections
 * @return 0 if succeeded, -1 otherwise
 */
int open_connection(Url url);

/**
 * Closes an open connection
 * @return 0 if succeeded, -1 otherwise
 */
int close_connection();

#endif
