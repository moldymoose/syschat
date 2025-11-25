#include "common.h"

void flush_input(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        ; // Consume the rest of the characters
    }
}