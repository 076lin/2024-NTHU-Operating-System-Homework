#include "syscall.h"

int main(void) {
    char test[] = "abcdefghijklmnopqrstuvwxyz";
    int success = Create("file1.test");
    int success1 = Create("file2.test");
    int success2 = Create("file3.test");
    OpenFileId fid;
    int i;

    if (success != 1)
        MSG("Failed on creating file1");
    if (success2 != 1)
        MSG("Failed on creating file2");
    if (success1 != 1)
        MSG("Failed on creating file3");
    fid = Open("file1.test");
    PrintInt(fid);
    if (fid < 0)
        MSG("Failed on opening file1");
    for (i = 0; i < 26; ++i) {
        int count = Write(test + i, 1, fid);
        if (count != 1)
            MSG("Failed on writing file");
    }
    success = Close(fid);
    if (success != 1)
        MSG("Failed on closing file");

    fid = Open("file2.test");
    PrintInt(fid);
    if (fid < 0)
        MSG("Failed on opening file2");
    for (i = 25; i >= 0; --i) {
        int count = Write(test + i, 1, fid);
        if (count != 1)
            MSG("Failed on writing file");
    }
    success = Close(fid);
    if (success != 1)
        MSG("Failed on closing file");

    fid = Open("file3.test");
    PrintInt(fid);
    if (fid < 0)
        MSG("Failed on opening file3");
    for (i = 0; i < 26; i += 2) {
        int count = Write(test + i, 1, fid);
        if (count != 1)
            MSG("Failed on writing file");
    }
    success = Close(fid);
    if (success != 1)
        MSG("Failed on closing file");

    MSG("Success on creating file1.test");
    Halt();
}
