#include "common.h"

void receive_handshake(int fd) {
    VPRINTF("Receiving handshake from fd %d\n", fd);
    // Buffer is just raw bytes
    char buffer[BUF_SIZE] = {0};
    read(fd, buffer, sizeof(proto_header_t) + sizeof(int));
    // Header points to the start of the buffer
    proto_header_t* header = (proto_header_t*)buffer;

    header->type = ntohl(header->type);
    header->length = ntohl(header->length);

    // Data is integer immediately after header
    int* data = (int*)(&header[1]);
    *data = ntohl(*data); // Should be PROTO_VERSION

    if(header->type != PROTO_HANDSHAKE) {
        perror("Invalid handshake type");
        exit(EXIT_FAILURE);
    }

    if (*data != PROTO_VERSION) {
        perror("Incompatible protocol version");
        exit(EXIT_FAILURE);
    }

    VPRINTF("Handshake received from fd %d: version %d\n", fd, *data);
}