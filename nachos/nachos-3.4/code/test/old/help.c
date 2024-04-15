#include "syscall.h"

#define BUFFER_SIZE (1024)

int main()
{
    int openFileId;           // File descriptor for the file to be read
    char buffer[BUFFER_SIZE]; // Buffer to store the content of the file
    int result = 0;           // Result of the read operation

    PrintString("*==========================================================*\n");
    PrintString("|      Help command for Nachos file system (mota.txt)      |\n");
    PrintString("*==========================================================*\n");
    PrintChar('\n');

    openFileId = Open("mota.txt", 1); // Open the file to read

    while (openFileId == -1) // If the file open failed
    {
        CreateFile("mota.txt"); // Create the file
        openFileId = Open("mota.txt", 1);
    }

    if (openFileId != -1) // If the file is opened successfully
    {
        result = Read(buffer, BUFFER_SIZE, openFileId); // Read the content of the file
        if (result == -1)
        {
            PrintString("!!!ERROR: File is not found or id is out of range or the file is not opened in read mode (stdout)\n");
        }
        else if (result == -2)
        {
            PrintString("!!!ERROR: File is empty (EOF)\n");
        }
        else if (result == -3)
        {
            PrintString("!!!ERROR: Unknown error occurs\n");
        }
        else
        {
            PrintString(buffer); // Print the content of the file
        }
        Close(openFileId); // Close the file
    }
    else // If the file open failed
    {
        PrintString("Can't open file\n"); // Print error message
    }

    PrintString("*=============================================*\n");
    PrintString("| Thank you for using this program in Nachos! |\n");
    PrintString("*=============================================*\n");

    return 0;
}