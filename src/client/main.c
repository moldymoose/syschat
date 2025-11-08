#include "../common/common.h"

int main() {
    int sock = create_socket();
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE] = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    read(sock, buffer, BUF_SIZE);
    printf("Received: %s", buffer);

    close(sock);
    return 0;
}
