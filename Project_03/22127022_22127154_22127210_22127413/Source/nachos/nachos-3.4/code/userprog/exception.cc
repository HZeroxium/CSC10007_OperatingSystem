// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#define MaxFileLength 32
#define MAX_NUM_LENGTH 11

/// @brief Print buffer to console instantly
/// @param buffer Buffer to print
void SynchPrint(char *buffer)
{
    gSynchConsole->Write(buffer, strlen(buffer) + 1);
    // gSynchConsole->Write("\n", 1);
}

/// @brief Print number to console instantly
/// @param number Number to print
void SynchPrint(int number)
{
    char buffer[MaxFileLength];
    sprintf(buffer, "%d", number);
    SynchPrint(buffer);
}

/// @brief Print float number to console instantly
/// @param number Float number to print
void SynchPrint(float number)
{
    char buffer[MaxFileLength];
    sprintf(buffer, "%f", number);
    // Round to 2 decimal places

    for (int i = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == '.')
        {
            if (buffer[i + 3] >= '5')
            {
                buffer[i + 2]++;
            }
            buffer[i + 3] = '\0';
            break;
        }
    }

    SynchPrint(buffer);
}

/// @brief Increase Program Counter (needed for each system call) (4 bytes for each instruction)
void IncreasePC()
{
    int counter = machine->ReadRegister(PCReg);     // Read current Program Counter
    machine->WriteRegister(PrevPCReg, counter);     // Write current Program Counter to Previous Program Counter
    counter = machine->ReadRegister(NextPCReg);     // Read Next Program Counter
    machine->WriteRegister(PCReg, counter);         // Write Next Program Counter to Program Counter
    machine->WriteRegister(NextPCReg, counter + 4); // Write Next Program Counter + 4 to Next Program Counter
}

/// @brief Copy buffer from User memory space to System memory space
/// @param virtAddr User space address
/// @param limit Limit of buffer
/// @return Buffer from kernel space
char *User2System(int virtAddr, int limit)
{
    int i; // Index of buffer
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; // For terminal string
    if (kernelBuf == NULL)
        return kernelBuf;

    memset(kernelBuf, 0, limit + 1); // Fill buffer with 0

    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar); // Read 1 byte from User memory space
        kernelBuf[i] = (char)oneChar;
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

/// @brief Copy buffer from System memory space to User memory space
/// @param virtAddr User space address
/// @param len Limit of buffer
/// @param buffer Buffer from kernel space
/// @return Number of bytes copied
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return NULL;
    int i = 0;
    int oneChar = 0;
    // Write memory to buffer until the end of buffer or the end of string
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar); // Write 1 byte to User memory space
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

/// @brief Handle system call Halt from user program
void Handle_SC_Halt()
{
    DEBUG('a', "Shutdown, initiated by user program.\n");
    printf("Shutdown, initiated by user program.\n");
    interrupt->Halt();
}

/// @brief Handle system call ReadChar from user program
void Handle_SC_ReadChar()
{
    int maxBytes = 255;           // Maximum length of buffer
    char *buffer = new char[255]; // Buffer to store data
    int numBytes = 0;             // The number of bytes read from console
    char c = 0;                   // Character to store the result

    if (buffer == NULL) // Check if buffer is NULL (not enough memory in system)
    {
        printf("Not enough memory in system\n");
    }
    else // Buffer is not NULL
    {
        numBytes = gSynchConsole->Read(buffer, maxBytes); // Read buffer from console and return the number of bytes read

        if (numBytes > 1) // Check if the number of bytes read is greater than 1
        {
            SynchPrint("\nERROR: You can only enter 1 character!\n");
            DEBUG('a', "\nERROR: You can only enter 1 character!");
        }
        else if (numBytes == 0) // Check if the number of bytes read is 0: empty character
        {
            SynchPrint("ERROR: Empty character!\n");
            DEBUG('a', "\nERROR: Empty character!");
        }
        else // If the number of bytes read is 1, return the character to register 2
        {
            c = *buffer;
        }
    }
    machine->WriteRegister(2, c); // Write the character to register 2
    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call PrintChar from user program
void Handle_SC_PrintChar()
{
    char c = (char)machine->ReadRegister(4); // Read character parameter from register 4

    if (c != 0) // Check if the character is not null
    {
        // SynchPrint("Character that you entered: ");
        gSynchConsole->Write(&c, 1); // Write the character to console
    }

    return IncreasePC();
}

/// @brief Handle system call ReadString from user program
void Handle_SC_ReadString()
{
    int virtAddr;        // Virtual address of input string from user space
    int length;          // Length of input string
    char *buffer = NULL; // Buffer to store input string

    virtAddr = machine->ReadRegister(4); // Read virtual address of input string PARAMETER from register 4
    length = machine->ReadRegister(5);   // Read maximum length of input string PARAMETER from register 5

    buffer = User2System(virtAddr, length); // Copy buffer from User memory space to System memory space

    gSynchConsole->Read(buffer, length); // Use SynchConsole to read buffer from console

    System2User(virtAddr, length, buffer); // Copy buffer from System memory space to User memory space

    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call PrintString from user program
void Handle_SC_PrintString()
{
    int virtAddr;   // Virtual address of string from user space (PARAMETER)
    char *buffer;   // Buffer that stores string for kernel space processing
    int length = 0; // Length of buffer

    virtAddr = machine->ReadRegister(4); // Read virtual address of string PARAMETER from register 4

    buffer = User2System(virtAddr, 1024); // Copy buffer from User memory space to System memory space

    while (buffer[length] != 0) // Calculate the length of buffer
        length++;

    gSynchConsole->Write(buffer, length + 1); // Print buffer to console

    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call ReadInt from user program
void Handle_SC_ReadInt()
{
    int limit;
    int result = 0;                                  // Store the result
    char _numberBuffer[MAX_NUM_LENGTH + 2];          // Buffer to store number buffer
    memset(_numberBuffer, 0, sizeof(_numberBuffer)); // Fill buffer with 0

    int len = gSynchConsole->Read(_numberBuffer, MAX_NUM_LENGTH + 1); // Read buffer from console

    if (strcmp(_numberBuffer, "-2147483648") == 0)
    {
        result = -2147483648;
    }
    else if (len != 0) // If nothing is read, return 0
    {
        // Process the number buffer to get the result
        bool isNumber = true;
        bool isNegative = (_numberBuffer[0] == '-');
        // SynchPrint("Number that you entered: ");
        // SynchPrint(_numberBuffer);
        for (int i = isNegative; i < len; ++i)
        {
            char c = _numberBuffer[i];
            if (c < '0' || c > '9')
            {
                DEBUG('a', "Expected number but %s found\n", _numberBuffer);
                SynchPrint("Expected number but ");
                SynchPrint(_numberBuffer);
                SynchPrint(" found\n");
                isNumber = false;
                break;
            }

            limit = 2147483647;
            if (result > limit / 10)
            {
                SynchPrint("Number is out of range\n");
                result = 0;
                break;
            }
            else
            {
                result *= 10;
            }

            int v = c - '0';
            if (result > limit - v)
            {
                SynchPrint("Number is out of range\n");
                result = 0;
                break;
            }
            else
            {
                result += v;
            }
        }

        if (isNegative)
        {
            result = -result;
        }

        bool isOutOfRange = (result < int(-2147483648)) || (result > 2147483647);
        if (isNumber && isOutOfRange)
        {
            SynchPrint("Number is out of range\n");
            result = 0;
        }
    }
    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call PrintInt from user program
void Handle_SC_PrintInt()
{
    int number = machine->ReadRegister(4); // Read value of number PARAMETER from register 4
    SynchPrint(number);                    // Print number to console
    return IncreasePC();
}

/// @brief Handle system call ReadFloat from user program
void Handle_SC_ReadFloat()
{
    float *result;
    char buffer[MAX_NUM_LENGTH + 2];
    memset(buffer, 0, sizeof(buffer));
    int len = gSynchConsole->Read(buffer, MAX_NUM_LENGTH + 1);

    if (len != 0)
    {
        bool isFloat = true;
        bool isNegative = (buffer[0] == '-');
        for (int i = isNegative; i < len; ++i)
        {
            char c = buffer[i];
            if (c != '.' && (c < '0' || c > '9'))
            {
                DEBUG('a', "Expected float but %s found\n", buffer);
                SynchPrint("Expected float but ");
                SynchPrint(buffer);
                SynchPrint(" found\n");
                isFloat = false;
                break;
            }
        }

        if (isFloat)
        {
            result = new float;
            *result = atof(buffer);
        }
    }

    machine->WriteRegister(2, (int)result);
    return IncreasePC();
}

/// @brief Handle system call PrintFloat from user program
void Handle_SC_PrintFloat()
{
    float *number = (float *)machine->ReadRegister(4); // Read value of number PARAMETER from register 4
    // Round to 2 decimal places
    *number = (int)(*number * 100 + 0.5) / 100.0;
    SynchPrint(*number); // Print number to console
    return IncreasePC();
}

/// @brief Handle system call CompareFloat from user program
void Handle_SC_CompareFloat()
{
    float *a = (float *)machine->ReadRegister(4);
    float *b = (float *)machine->ReadRegister(5);
    int result = 0;
    if (*a > *b)
    {
        result = 1;
    }
    else if (*a < *b)
    {
        result = -1;
    }
    machine->WriteRegister(2, result);
    return IncreasePC();
}

/// @brief Handle system call FreeFloat from user program
void Handle_SC_FreeFloat()
{
    float *number = (float *)machine->ReadRegister(4);
    if (number != NULL)
        delete number;
    return IncreasePC();
}

/// @brief Handle system call FloatToString from user program
void Handle_SC_FloatToString()
{
    float *number = (float *)machine->ReadRegister(4);
    int virtAddr = machine->ReadRegister(5);
    char *buffer = new char[MAX_NUM_LENGTH + 2];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%f", *number);
    System2User(virtAddr, MAX_NUM_LENGTH + 2, buffer);
    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call CreateFile from user program
void Handle_SC_CreateFile()
{
    int virtualAddr; // Virtual address of the filename
    char *filename;  //  Buffer to store filename
    int result = 0;  // Result of the function

    virtualAddr = machine->ReadRegister(4); // Read virtual address of filename PARAMETER from register 4

    filename = User2System(virtualAddr, MaxFileLength + 1); // Copy buffer from User memory space to System memory space

    if (filename == NULL) // If filename is NULL (not enough memory in system)
    {
        printf("\n Not enough memory in system");
        result = -1; // Failed to create file, return -1
    }
    else // If filename is not NULL
    {
        if (!fileSystem->Create(filename, 0)) // If file is not created
        {
            // printf("\n Error create file '%s'", filename);
            result = -1; // Failed to create file, return -1
        }
        else // If file is created
        {
            result = 0; // Success, return 0
        }
    }

    machine->WriteRegister(2, result); // return the result to system call register 2
    delete filename;
    return IncreasePC();
}

/// @brief Handle system call Open from user program (Open file)
void Handle_SC_Open()
{
    int virtAddr = machine->ReadRegister(4); // Virtual address of filename PARAMETER from register 4
    int type = machine->ReadRegister(5);     // Type (Open mode) of the file PARAMETER from register 5
    char *filename;                          // Buffer to store filename
    int result = -2;                         // Result of the function

    filename = User2System(virtAddr, MaxFileLength); // Copy buffer from User memory space to System memory space

    int allocatedSlot = fileSystem->GetAllocatedSlot(); // Find free slot in file system

    if (allocatedSlot != -1) // If there is free slot
    {
        if (type == READ_WRITE || type == READ_ONLY) // Handle ReadOnly and ReadWrite file cases
        {
            fileSystem->file_table[allocatedSlot] = fileSystem->Open(filename, type); // Open file with filename and type

            if (fileSystem->file_table[allocatedSlot] != NULL)
            {
                result = allocatedSlot; // Success, return free slot for normal file
            }
            else // Open file failed
            {
                result = -1; // Failed to open file, return -1
            }
        }
        else if (type == STDIN) // ConsoleInput: stdin
        {
            result = 0; // Success, return 0 for STDIN
        }
        else // ConsoleOutput: stdout
        {
            result = 1; // Success, return 1 for STDOUT
        }
    }
    else // No free slot
    {
        SynchPrint("No free slot");
        result = -1; // Failed to open file, return -1
    }

    if (filename != NULL)
        delete[] filename;

    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call Close from user program (Close file)
void Handle_SC_Close()
{
    int id = machine->ReadRegister(4); // Read file descriptor (OpenFileId) PARAMETER from register 4
    int result = -2;                   // Result of the function

    if (id >= 0 && id < MAX_FILE && fileSystem->file_table[id] != NULL) // If file descriptor is in range and file is exist
    {
        delete fileSystem->file_table[id]; // Delete file
        fileSystem->file_table[id] = NULL; // Set file to NULL
        result = 0;                        // Success, return 0
    }
    else // If file descriptor is out of range or file is not exist
    {
        result = -1; // Failed to close file, return -1
    }

    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call SC_Read from user program
void Handle_SC_Read()
{
    int virtAddr = machine->ReadRegister(4);  // Read virtual address of buffer PARAMETER from register 4
    int charcount = machine->ReadRegister(5); // Read number of characters PARAMETER from register 5
    int id = machine->ReadRegister(6);        // Read file descriptor PARAMETER from register 6
    int OldPos;                               // Old position of file (current position of file before read)
    int NewPos;                               // New position of file (current position of file after read)
    char *buf;                                // Kernel buffer
    int result = -3;                          // Value return for Read function

    if (id < 0 || id >= MAX_FILE) // If file descriptor is out of range
    {
        SynchPrint("\nOut of range file descriptor.");
        result = -1; // Failed to read file, return -1
    }

    else if (fileSystem->file_table[id] == NULL) // If file is not exist
    {
        SynchPrint("\nCan't open file because file is not exist.");
        result = -1; // Failed to read file, return -1
    }

    else if (fileSystem->file_table[id]->type == STDOUT) // If file is stdout
    {
        SynchPrint("\nCan't open console output to read.");
        result = -1; // Failed to read file, return -1
    }
    else // If file is exist
    {
        OldPos = fileSystem->file_table[id]->GetCurrentPos(); // Get current position of file
        buf = User2System(virtAddr, charcount);               // Convert buffer to system space
        if (fileSystem->file_table[id]->type == STDIN)        // If file is stdin
        {
            int size = gSynchConsole->Read(buf, charcount); // Read from console
            System2User(virtAddr, size, buf);               // Copy buffer to user space
            result = size;                                  // Write size to register 2, Success
        }
        else if ((fileSystem->file_table[id]->Read(buf, charcount)) > 0) // If file is normal file
        {
            NewPos = fileSystem->file_table[id]->GetCurrentPos(); // Actual number of bytes read
            System2User(virtAddr, NewPos - OldPos, buf);          // Copy buffer to user space
            result = NewPos - OldPos;                             // Write number of bytes read to register 2, Success
        }
        else // If file is empty
        {
            result = -2; // Write -2 to register 2, Success
        }
    }
    if (result != -1 && result != -3) // If read file successfully
    {
        // SynchPrint("Read file success");
    }
    delete buf;
    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call SC_Write from user program
void Handle_SC_Write()
{
    int virtAddr = machine->ReadRegister(4);  // Read virtual address of buffer PARAMETER from register 4
    int charcount = machine->ReadRegister(5); // Read number of characters PARAMETER from register 5
    int id = machine->ReadRegister(6);        // Read file descriptor PARAMETER from register 6
    int OldPos;                               // Old position of file (current position of file before write)
    int NewPos;                               // New position of file (current position of file after write)
    char *buf;                                // Kernel buffer
    int result = -3;                          // Value return for Write function

    if (id < 0 || id > MAX_FILE) // If file descriptor is out of range
    {
        SynchPrint("\nOut of range file descriptor.");
        result = -1; // Failed to write file, return -1
    }
    else if (fileSystem->file_table[id] == NULL) // If file is not exist
    {
        SynchPrint("\nCan't open file because file is not exist.");
        result = -1; // Failed to write file, return -1
    }
    else if (fileSystem->file_table[id]->type == READ_ONLY || fileSystem->file_table[id]->type == STDIN) // If file is read only or stdin
    {
        SynchPrint("\nCan't write file because file is only read or stdin.");
        result = -1; // Failed to write file, return -1
    }
    else // If file is exist
    {
        OldPos = fileSystem->file_table[id]->GetCurrentPos(); // Get current position of file
        buf = User2System(virtAddr, charcount);               // Convert buffer to system space
        if (fileSystem->file_table[id]->type == READ_WRITE)   // If file is read and write file
        {
            if ((fileSystem->file_table[id]->Write(buf, charcount)) > 0) // Write to file
            {
                NewPos = fileSystem->file_table[id]->GetCurrentPos();
                result = NewPos - OldPos;
            }
        }
        else if (fileSystem->file_table[id]->type == STDOUT) // If file is stdout
        {
            int i = 0;
            while (buf[i] != 0 && buf[i] != '\n') // Write buffer to console until the end of buffer or the end of line
            {
                gSynchConsole->Write(buf + i, 1); // Write 1 byte to console
                i++;
            }
            buf[i] = '\n';
            gSynchConsole->Write(buf + i, 1); // Write character '\n' to console
            result = i - 1;
        }
    }
    if (result != -1 && result != -3) // If write file successfully
    {
        // SynchPrint("Write file success");
    }
    delete buf;
    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call SC_Execfrom user program
void Handle_SC_Exec()
{
    int result = -1; // Result of the function
    // Read system call parameters
    int virtAddr = machine->ReadRegister(4); // Read virtual address of filename PARAMETER from register 4

    // Copy buffer from User memory space to System memory space
    char *filename = User2System(virtAddr, MaxFileLength);

    // Check if filename is NULL (not enough memory in system)
    OpenFile *executable = fileSystem->Open(filename); // Open file with filename

    if (executable != NULL) // If file is not exist
    {
        result = pTab->ExecUpdate(filename); // Update process table with new process
    }

    // Write result to register 2
    machine->WriteRegister(2, result);

    delete executable; // Delete executable file
    delete[] filename;

    // Increase Program Counter
    return IncreasePC();
}

/// @brief Handle system call SC_Join from user program
void Handle_SC_Join()
{
    // Read system call parameters
    int id = machine->ReadRegister(4); // Read process id PARAMETER from register 4

    // Call JoinUpdate function to update process table with process id
    int result = pTab->JoinUpdate(id);

    // Write result to register 2
    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call SC_Exit from user program
void Handle_SC_Exit()
{
    // Read system call parameters
    int exitStatus = machine->ReadRegister(4); // Read exit status PARAMETER from register 4

    // Check if exit status is not 0
    if (exitStatus != 0)
    {
        SynchPrint("Exit with status: ");
        SynchPrint(exitStatus);
        return IncreasePC();
    }

    // Call ExitUpdate function to update process table with exit status
    int res = pTab->ExitUpdate(exitStatus);

    // Write result to register 2
    machine->WriteRegister(2, res);

    // Free space and finish current thread
    currentThread->FreeSpace(); // Free space for current thread
    currentThread->Finish();    // Finish current thread
    return IncreasePC();
}

/// @brief Handle system call SC_CreateSemaphore from user program
void Handle_SC_CreateSemaphore()
{
    int result = -1;
    // Read system call parameters
    int virtAddr = machine->ReadRegister(4); // Read virtual address of semaphore name PARAM
    int semval = machine->ReadRegister(5);   // Read semaphore value PARAM

    // Copy buffer from User memory space to System memory space
    char *name = User2System(virtAddr, MaxFileLength);

    if (name == NULL) // If name is NULL (not enough memory in system)
    {
        SynchPrint("Not enough memory in system\n");
    }
    else
    {
        // Create semaphore with Create method of STable
        result = sTab->Create(name, semval);

        // Check if semaphore is already exist
        if (result == -1)
        {
            SynchPrint("Can't create semaphore because semaphore is already exist\n");
        }
    }
    delete[] name;

    // Write result to register 2
    machine->WriteRegister(2, result);

    // Increase Program Counter
    return IncreasePC();
}

/// @brief Handle system call SC_Down from user program
void Handle_SC_Down()
{
    int result = -1;
    // Read system call parameters
    int virtAddr = machine->ReadRegister(4); // Read virtual address of semaphore name PARAM

    // Copy buffer from User memory space to System memory space
    char *name = User2System(virtAddr, MaxFileLength);

    // Check if name is valid
    if (name == NULL)
    {
        SynchPrint("Not enough memory in system\n");
    }
    else
    {
        // Down semaphore with name
        result = sTab->Wait(name);

        // Check if semaphore is not exist
        if (result == -1)
        {
            SynchPrint(name);
            SynchPrint("Can't down semaphore because semaphore is not exist\n");
        }
    }
    delete[] name;
    // Write result to register 2
    machine->WriteRegister(2, result); // Write result to register 2

    // Increase Program Counter
    return IncreasePC();
}

/// @brief Handle system call SC_Up from user program
void Handle_SC_Up()
{
    int result = -1;
    // Read system call parameters
    int virtAddr = machine->ReadRegister(4); // Read virtual address of semaphore name PARAM

    // Copy buffer from User memory space to System memory space
    char *name = User2System(virtAddr, MaxFileLength);

    // Check if name is valid
    if (name == NULL)
    {
        SynchPrint("Not enough memory in system\n");
    }
    else
    {
        // Up semaphore with name
        result = sTab->Signal(name);

        // Check if semaphore is not exist
        if (result == -1)
        {
            SynchPrint("Can't down semaphore because semaphore is not exist\n");
        }
    }
    delete[] name;
    // Write result to register 2
    machine->WriteRegister(2, result); // Write result to register 2

    // Increase Program Counter
    return IncreasePC();
}

/// @brief Handle system call SC_Seek from user program
void Handle_SC_Seek()
{
    int pos = machine->ReadRegister(4); // Read position PARAMETER from register 4
    int id = machine->ReadRegister(5);  // Read file descriptor PARAMETER from register 5
    int result = -1;                    // Result of the function

    if (id < 0 || id >= MAX_FILE) // If file descriptor is out of range
    {
        SynchPrint("\nOut of range file descriptor.");
    }

    if (fileSystem->file_table[id] == NULL) // If file is not exist
    {
        SynchPrint("\nCan't open file because file is not exist.");
    }

    else if (fileSystem->file_table[id]->type == STDIN || fileSystem->file_table[id]->type == STDOUT) // If file is stdin or stdout
    {
        SynchPrint("\nCan't seek console input or output.");
    }

    pos = (pos == -1) ? fileSystem->file_table[id]->Length() : pos; // If position is -1, set position to the end of file

    if (pos > fileSystem->file_table[id]->Length() || pos < 0) // If position is out of range
    {
        SynchPrint("\nOut of range position.");
    }

    else // If position is in range
    {
        fileSystem->file_table[id]->Seek(pos); // Set position of file to pos
        result = pos;                          // Success, return pos
    }

    // Write result to register 2
    machine->WriteRegister(2, result);
    return IncreasePC();
}

/// @brief Exception handler for user program system calls
/// @param which Type of exception
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2); // Read system call code from register 2

    switch (which)
    {
    case SyscallException: // System call exception
        switch (type)
        {
        case SC_Halt:
            return Handle_SC_Halt();
        case SC_ReadInt:
            return Handle_SC_ReadInt();
        case SC_PrintInt:
            return Handle_SC_PrintInt();
        case SC_ReadFloat:
            return Handle_SC_ReadFloat();
        case SC_PrintFloat:
            return Handle_SC_PrintFloat();
        case SC_ReadChar:
            return Handle_SC_ReadChar();
        case SC_PrintChar:
            return Handle_SC_PrintChar();
        case SC_ReadString:
            return Handle_SC_ReadString();
        case SC_PrintString:
            return Handle_SC_PrintString();
        case SC_CreateFile:
            return Handle_SC_CreateFile();
        case SC_Open:
            return Handle_SC_Open();
        case SC_Close:
            return Handle_SC_Close();
        case SC_Read:
            return Handle_SC_Read();
        case SC_Write:
            return Handle_SC_Write();
        case SC_CompareFloat:
            return Handle_SC_CompareFloat();
        case SC_FreeFloat:
            return Handle_SC_FreeFloat();
        case SC_FloatToString:
            return Handle_SC_FloatToString();
        case SC_Exec:
            return Handle_SC_Exec();
        case SC_Join:
            return Handle_SC_Join();
        case SC_Exit:
            return Handle_SC_Exit();
        case SC_CreateSemaphore:
            return Handle_SC_CreateSemaphore();
        case SC_Down:
            return Handle_SC_Down();
        case SC_Up:
            return Handle_SC_Up();
        case SC_Seek:
            return Handle_SC_Seek();
        default:
            interrupt->Halt();
            break;
        }
    case NoException:
        return;
    case PageFaultException:
        printf("PageFaultException: No valid translation found\n");
        interrupt->Halt();
        break;
    case ReadOnlyException:
        printf("ReadOnlyException: Write attempted to page marked \"read-only\"\n");
        interrupt->Halt();
        break;
    case BusErrorException:
        printf("BusErrorException: Translation resulted in an invalid physical address\n");
        interrupt->Halt();
        break;
    case AddressErrorException:
        printf("AddressErrorException: Unaligned reference or one that was beyond the end of the address space\n");
        interrupt->Halt();
        break;
    case OverflowException:
        printf("OverflowException: Integer overflow in add or sub.\n");
        interrupt->Halt();
        break;
    case IllegalInstrException:
        printf("IllegalInstrException: Unimplemented or reserved instr.\n");
        interrupt->Halt();
        break;
    case NumExceptionTypes:
        printf("NumExceptionTypes: Number exception types\n");
        interrupt->Halt();
        break;
    default:
        printf("Unexpected user mode exception %d %d\n", which, type);
        interrupt->Halt();
        break;
    }
}
