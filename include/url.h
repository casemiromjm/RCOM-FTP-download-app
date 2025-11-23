#ifndef URL_H
#define URL_H

#define MAX_LENGTH 500

typedef struct Url {
    char user[MAX_LENGTH];
    char password[MAX_LENGTH];
    char host[MAX_LENGTH];
    char resource[MAX_LENGTH];
} Url;

#endif
