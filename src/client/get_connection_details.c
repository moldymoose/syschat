#include "client_header.h"

Connection_Details get_connection_details(void) {
    Connection_Details details;

    while(!prompt(details.username, USERNAME_MAX, "Enter your username: ")) {
        printf("Error: username must be between 0 and 24 characters\n");
    }
    while(!prompt(details.host_ip, INET_ADDRSTRLEN, "Ener host address: ")) {
        printf("Error: host address too long to be valid\n");
    }

    return details;
}