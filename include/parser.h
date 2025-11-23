#ifndef PARSER_H
#define PARSER_H

#include "url.h"

/**
 * Parses the received to url
 * @param raw_url Url in string format
 * @param url Url after being parsed in url format
 * @return 0 if succeded, -1 otherwise
 */
int parse(char *raw_url, Url *url);

#endif