#ifndef URL_H
#define URL_H

#define MAX_LENGTH 500

/**
 * Struct for storing a parsed url
 */
typedef struct Url {
    char user[MAX_LENGTH];
    char password[MAX_LENGTH];
    char hostname[MAX_LENGTH];
    char resource[MAX_LENGTH];

    char host_ip[MAX_LENGTH];
    char file[MAX_LENGTH];
} Url;

#endif
