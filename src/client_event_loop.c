#include "common.h"

void client_event_loop(int socket_fd) {
    VPRINTF("Entering client event loop on fd %d\n", socket_fd);

    while(true) {
        // Set up file descriptor to monitor for input
        fd_set read_fds;

        // Clear the set
        FD_ZERO(&read_fds);
        
        // Add stdin and socket_fd to the set
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(socket_fd, &read_fds);

        // Highest file descriptor number
        int max_fd = socket_fd > STDIN_FILENO ? socket_fd : STDIN_FILENO;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Input from user
            char input_buffer[BUF_SIZE - sizeof(proto_header_t)] = {0};
            if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                char buffer[BUF_SIZE] = {0};

                proto_header_t* header = (proto_header_t*)buffer;
                header->type = PROTO_MESSAGE;
                int real_length = strlen(input_buffer);
                header->length = htonl(real_length);

                char* data = (char*)&header[1];
                strncpy(data, input_buffer, real_length);
                write(socket_fd, header, sizeof(proto_header_t) + real_length);
            }
        }

        if (FD_ISSET(socket_fd, &read_fds)) {
            // Input from server
            char buffer[BUF_SIZE] = {0};
            int bytes_read = read(socket_fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                VPRINTF("Server disconnected or read error on fd %d\n", socket_fd);
                break;
            }

            proto_header_t* header = (proto_header_t*)buffer;
            header->type = ntohl(header->type);
            header->length = ntohl(header->length);

            if (header->type == PROTO_MESSAGE) {
                char* data = (char*)&header[1];
                data[header->length] = '\0'; // Null-terminate the message
                VPRINTF("Message from server: %s", data);
            } else {
                VPRINTF("Unknown protocol type %d received on fd %d\n", header->type, socket_fd);
            }
        }
    }

    VPRINTF("Exiting client event loop on fd %d\n", socket_fd);
}