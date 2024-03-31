#include "syscall.h"

int main()
{
    float *pf;
    float *f;

    PrintString("*=====================================*\n");
    PrintString("| Welcome to the Float I/O Test Suite |\n");
    PrintString("*=====================================*\n");
    PrintString("\nEnter a float: ");
    pf = ReadFloat();
    f = pf;
    PrintString("-> Float that you entered: ");
    PrintFloat(pf);

    PrintString("\nExiting Float I/O Test Suite\n\n");
}