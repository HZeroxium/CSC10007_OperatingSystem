#include "syscall.h"

int main()
{
    float *pf; // Pointer to a float

    PrintString("*=====================================*\n");
    PrintString("| Welcome to the Float I/O Test Suite |\n");
    PrintString("*=====================================*\n");

    PrintString("\nEnter a float: ");
    pf = ReadFloat(); // Read a float from the user
    PrintString("-> Float that you entered: ");
    PrintFloat(pf); // Print the float that the user entered

    PrintString("\nExiting Float I/O Test Suite\n\n");
}