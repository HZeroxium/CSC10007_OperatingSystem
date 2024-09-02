#include "syscall.h"

#define NULL 0
#define BACKSPACE 8
#define ESC 27
#define SPACE 32
#define DELETE 127
#define TAB 9

int main()
{
    char c;
    PrintString("*====================================*\n");
    PrintString("| Welcome to the Char I/O Test Suite |\n");
    PrintString("*====================================*\n");

    PrintString("Enter a character: ");
    c = ReadChar();
    PrintString("-> You entered: ");
    switch (c)
    {
    case NULL:
        PrintString("NULL\n");
        break;
    case ESC:
        PrintString("ESC\n");
        break;
    case SPACE:
        PrintString("SPACE\n");
        break;
    case TAB:
        PrintString("TAB\n");
        break;
    default:
        PrintChar(c);
        break;
    }
    PrintChar('\n');

    PrintString("Exiting Char I/O Test Suite\n\n");
}