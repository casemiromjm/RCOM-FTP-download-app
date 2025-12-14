#ifndef UTILS_H
#define UTILS_H

#include "url.h"

/**
 * Gets Host IP from a given Url Struct
 * @return 0 if succeeded, -1 otherwise
 */
int getHostIp(Url *url);

/**
 * Creates a socket and connects it
 * @param sock Socket created
 * @return 0 if OK, -1 otherwise
 */
int create_socket(Url url, int *sock);

#endif
