#include "common.h"

void receive_message(int fd) {
    VPRINTF("Receiving message from fd %d\n", fd);

    char buffer[BUF_SIZE] = {0};
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        VPRINTF("Client disconnected or read error on fd %d\n", fd);
        return;
    }

    proto_header_t* header = (proto_header_t*)buffer;
    header->type = ntohl(header->type);
    header->length = ntohl(header->length);

    if (header->type == PROTO_MESSAGE) {
        char* data = (char*)&header[1];
        data[header->length] = '\0'; // Null-terminate the message
        VPRINTF("Message from client fd %d: %s", fd, data);
    } else {
        VPRINTF("Unknown protocol type %d from fd %d\n", header->type, fd);
    }
}