#include "syscall.h"

int main()
{
    int number;
    PrintString("*====================================*\n");
    PrintString("| Welcome to the Int I/O Test Suite  |\n");
    PrintString("*====================================*\n");

    PrintString("Enter an integer: ");
    number = ReadInt();
    PrintString("-> Integer that you entered: ");
    PrintInt(number);
    PrintChar('\n');

    PrintString("Exiting Int I/O Test Suite\n\n");
}