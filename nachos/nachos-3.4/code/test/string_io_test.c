#include "syscall.h"

int main()
{
    char buffer[100];
    PrintString("*======================================*\n");
    PrintString("| Welcome to the String I/O Test Suite |\n");
    PrintString("*======================================*\n");

    PrintString("\nEnter a string: ");
    ReadString(buffer, 100);
    PrintString("The string you entered is: ");
    PrintString(buffer);
    PrintChar('\n');

    PrintString("Exiting String I/O Test Suite\n\n");
}