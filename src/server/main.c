#include "../common/common.h"

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = create_socket();

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    char *message = "Hello from server!\n";
    send(client_fd, message, strlen(message), 0);

    close(client_fd);
    close(server_fd);
    return 0;
}
