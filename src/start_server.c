#include "common.h"

void start_server(config_t config) {
    VPRINTF("Starting server at %s:%d\n", config.address, config.port);

    // Create socket
    int server_fd = create_socket();

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(config.port);
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address

    VPRINTF("Binding socket\n");
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    VPRINTF("Listening on socket\n");
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    VPRINTF("Server listening on %s:%d\n", config.address, config.port);
    
    server_event_loop(server_fd);

    // Probably unreachable since server_event_loop has no exit
    close(server_fd);
}