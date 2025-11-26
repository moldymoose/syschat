#include "common.h"

void flush_input(void) {
    int ch;

    VPRINTF("Flushing input buffer\n");
    while ((ch = getchar()) != '\n' && ch != EOF) {
        ; // Discard characters
    }
}