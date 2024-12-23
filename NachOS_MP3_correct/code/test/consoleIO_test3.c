#include "syscall.h"
int a[2000];
int main() {
    int n;
    for (n = 1; n <= 1000; n++) {
        PrintInt(n);
    }
    return 0;
    // Halt();
}
