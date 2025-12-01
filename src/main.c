#include "common.h"
int verbose = 0;
int main(int argc, char *argv[]) {

    config_t config = {0};

    int opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"server", no_argument, 0, 's'},
        {"address", required_argument, 0, 'a'},
        {"port", required_argument, 0, 'p'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "hsa:p:v", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [--server] [--address <address>] [--port <port>]\n", argv[0]);
                exit(EXIT_SUCCESS);
            case 's':
                config.is_server = true;
                break;
            case 'a':
                strncpy(config.address, optarg, INET_ADDRSTRLEN - 1);
                config.address[INET_ADDRSTRLEN - 1] = '\0';
                break;
            case 'p':
                config.port = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                VPRINTF("Verbose mode enabled\n");
                break;
            default:
                fprintf(stderr, "Usage: %s [--server] [--address <address>] [--port <port>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (config.port == 0) {
        char buffer[6] = {0};
        while (atoi(buffer) < 1024 || atoi(buffer) > 65535) {
            prompt(buffer, sizeof(buffer), "Enter port number (1024-65535): ");
        }
        config.port = atoi(buffer);
    }

    if (config.is_server) {
        VPRINTF("Starting in server mode\n");
        if (config.address[0] == '\0') {
            strcpy(config.address, "0.0.0.0");
            start_server(config);
        } else {
            perror("Server mode does not accept custom address");
            exit(EXIT_FAILURE);
        }
    } else {
        VPRINTF("Starting in client mode\n");
        if (config.address[0] == '\0') {
            while(!prompt(config.address, INET_ADDRSTRLEN, "Enter server address: "));
        }
        start_client(config);
    }


    return 0;
}
