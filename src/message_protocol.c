#include "common.h"

ssize_t write_all(int fd, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, (const char*)buf + sent, len - sent);
        if (n <= 0) return n; // error
        sent += n;
    }
    return sent;
}

ssize_t read_all(int fd, void *buf, size_t len) {
    size_t received = 0;
    while (received < len) {
        ssize_t n = read(fd, (char*)buf + received, len - received);
        if (n <= 0) return n; // error or EOF
        received += n;
    }
    return received;
}

int send_proto_message(int fd, proto_type_e type, const void *payload, uint32_t length) {
    uint8_t header[PROTO_HEADER_SIZE];

    // Construct and pad header
    header[0] = type;
    header[1] = 0;
    header[2] = 0;
    header[3] = 0;

    uint32_t nlen = htonl(length);
    memcpy(&header[4], &nlen, 4);

    if (write_all(fd, header, PROTO_HEADER_SIZE) <= 0)
        return -1;

    if (length > 0 && write_all(fd, payload, length) <= 0)
        return -1;

    return 0;
}

int recv_proto_message(int fd, proto_type_e *type, void **payload, uint32_t *length) {
    uint8_t header[PROTO_HEADER_SIZE];

    if (read_all(fd, header, PROTO_HEADER_SIZE) <= 0)
        return -1;

    *type = header[0];
    *length = ntohl(*(uint32_t*)&header[4]);

    *payload = malloc(*length);
    if (*payload == NULL)
        return -1;

    if (*length > 0 && read_all(fd, *payload, *length) <= 0) {
        free(*payload);
        return -1;
    }

    return 0;
}
