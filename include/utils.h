#ifndef UTILS_H
#define UTILS_H

#include "url.h"

/**
 * Gets Host IP from a given Url Struct
 * @return 0 if succeeded, -1 otherwise
 */
int getHostIp(Url *url);

#endif
