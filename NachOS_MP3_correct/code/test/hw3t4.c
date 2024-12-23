#include "syscall.h"

int main() {
    int n, i;
    for (n = 1; n < 20; ++n) {
        PrintInt(4);
        for (i = 0; i < 100; ++i);
    }
    Exit(1);
}
