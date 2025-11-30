#include "common.h"

int prompt(char* buffer, size_t size, char* message) {
    char tempBuffer[size + 1];  // temp buffer has space for newline
    printf("%s", message);

    if (fgets(tempBuffer, size + 1, stdin) == NULL) {
        fprintf(stderr, "Error with fgets\n");
        return 0;
    }
    size_t length = strlen(tempBuffer);

    if (length > 0 && tempBuffer[length - 1] == '\n') {
        tempBuffer[length -1] = '\0';
    } else {
        printf("Input exceeded maximum length of %zu characters.\n", size - 1);
        flush_input();
        return 0;
    }
    
    if (tempBuffer[0] == '\0') {
        printf("Input cannot be empty.\n");
        return 0;
    }

    strcpy(buffer, tempBuffer);
    return 1;
}