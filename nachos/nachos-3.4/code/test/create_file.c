#include "syscall.h"

int main()
{
    char fileName[256];

    PrintString("Enter file's name: ");
    ReadString(fileName, 256);

    if (CreateFile(fileName) == 0)
    {
        PrintString("Create file successfully\n");
    }
    else
        PrintString("Create file failed\n");
}