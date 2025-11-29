#include "protocol.h"

void client_event_loop(int socket_fd) {
    VPRINTF("Entering client event loop on fd %d\n", socket_fd);
    char username[USERNAME_MAX] = {0};
    while(!prompt(username, USERNAME_MAX, "Please Enter Your Name: "));

    send_proto_message(socket_fd, PROTO_USERNAME, username, strlen(username));

    while(true) {
        // Clears screen with ansi escape sequence
        printf("\033[2J\033[H");

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
            char input_buffer[BUF_SIZE] = {0};
            if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                int length = strlen(input_buffer);
                send_proto_message(socket_fd, PROTO_MESSAGE, input_buffer, length);
            }
        }

        if (FD_ISSET(socket_fd, &read_fds)) {
        }
    }

    VPRINTF("Exiting client event loop on fd %d\n", socket_fd);
}