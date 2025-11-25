#include "client_header.h"

int main(int argc, char* argv[]) {
    Connection_Details connection_details;
    if (argc == 1) {
        // Prompt for username and host details
        connection_details = get_connection_details();
    } else if (argc == 2) {
        // TODO
        // Parse username and host address from argument
        // TODO add functionality for help flag
        fprintf(stderr, "passing username and address as argument isn't implemented yet.\n");
        exit(EXIT_FAILURE);
    } else {
        // User passed too many arguments
        fprintf(stderr, "User Error: Passed too many arguements.\n");
        exit(EXIT_FAILURE);
    }

    
    int sock_fd = create_socket();      // Gets socket descriptor from system call
    struct sockaddr_in server_addr;     // sockaddr_in stores ipv4 address info

    char buffer[BUF_SIZE] = {0};        // Buffer for reading from server


    
    // Sets address family to ipv4 (AF_INET)
    server_addr.sin_family = AF_INET;
    // Convert the IP address string to network binary format
    // and store it in sin_addr.s_addr
    if (inet_pton(AF_INET, connection_details.host_ip, &(server_addr.sin_addr)) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
    // Converts port number from system's order to network byte order standard
    server_addr.sin_port = htons(PORT);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    read(sock_fd, buffer, BUF_SIZE);
    printf("Received: %s", buffer);

    close(sock_fd);
    
    return 0;
}
