#include "syscall.h"

int main(void) {
    char test[] = "abcdefghijklmnopqrstuvwxyz";
    int success = Create("file1.test");
    OpenFileId fid;
    int i;

    if (success != 1)
        MSG("Failed on creating file");
    fid = Open("file1.test");
    int tmp;
    if (fid < 0)
        MSG("Failed on opening file");

    for (i = 0; i < 26; ++i) {
        int count = Write(test + i, 1, fid);
        if (count != 1)
            MSG("Failed on writing file");
    }

    // tmp = Open("file1.test");
    // if (tmp == -1)
    //     MSG("Reopen failed");
    success = Close(fid);
    if (success != 1)
        MSG("Failed on closing file");

    fid = Open("file1.test");
    if (fid < 0)
        MSG("Failed on opening file after closing");
    success = Close(fid);
    if (success != 1)
        MSG("Failed on closing file");

    MSG("PASS!!!");
    Halt();
}
