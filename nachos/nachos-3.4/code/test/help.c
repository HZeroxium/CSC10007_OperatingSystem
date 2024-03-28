#include "syscall.h"

#define BUFFER_SIZE (1024)

int main()
{
    int openFileId;           // File descriptor for the file to be read
    char buffer[BUFFER_SIZE]; // Buffer to store the content of the file

    openFileId = Open("mota.txt", 1); // Open the file to read
    if (openFileId != -1)             // If the file is opened successfully
    {
        Read(buffer, BUFFER_SIZE, openFileId);   // Read the content of the file
        PrintString(buffer);                     // Print the content of the file
        Close(openFileId);                       // Close the file
        PrintString("File read successfully\n"); // Print success message
        return 0;
    }
    else // If the file open failed
    {
        PrintString("Can't open file\n"); // Print error message
        return 1;
    }
}