#include "syscall.h"

int main()
{
    char buffer[100];
    PrintString("Enter a string: ");
    ReadString(buffer, 100);
    PrintString("The string you entered is: ");
    PrintString(buffer);
}