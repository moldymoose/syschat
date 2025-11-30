#include "../common/common.h"

int main() {
    int sock = create_socket();
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE] = {0};
    char message[BUF_SIZE];

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

    printf("Connected to server. Type messages (or 'quit' to exit):\n");

    while (1) {
        printf("> ");
        fgets(message, BUF_SIZE, stdin);
        
        if (strncmp(message, "quit", 4) == 0) {
            break;
        }
        
        send(sock, message, strlen(message), 0);
        
        int bytes = read(sock, buffer, BUF_SIZE-1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("Server: %s", buffer);
        }
        
        memset(buffer, 0, BUF_SIZE);
    }

    close(sock);
    return 0;
}
