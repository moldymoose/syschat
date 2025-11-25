#ifndef CLIENT_H
#define CLIENT_H

#include "../common/common.h"

typedef struct {
    char username[USERNAME_MAX];
    char host_ip[INET_ADDRSTRLEN];
} Connection_Details;

Connection_Details get_connection_details(void);

#endif