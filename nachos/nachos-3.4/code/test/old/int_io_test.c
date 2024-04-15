#include "syscall.h"

int main()
{
    int number; // Integer to store the number entered by the user
    PrintString("*====================================*\n");
    PrintString("| Welcome to the Int I/O Test Suite  |\n");
    PrintString("*====================================*\n");

    PrintString("Enter an integer: ");
    number = ReadInt(); // Read an integer from the user
    PrintString("-> Integer that you entered: ");
    PrintInt(number); // Print the integer that the user entered

    PrintChar('\n');
    PrintString("Exiting Int I/O Test Suite\n\n");
}