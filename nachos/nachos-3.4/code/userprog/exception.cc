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
#include "filesys.h"

#define MaxFileLength 32
#define MAX_NUM_LENGTH 11

/// @brief Print buffer to console instantly
/// @param buffer Buffer to print
void SynchPrint(char *buffer)
{
    gSynchConsole->Write(buffer, strlen(buffer) + 1);
}

/// @brief Print number to console instantly
/// @param number Number to print
void SynchPrint(int number)
{
    char buffer[MaxFileLength];
    sprintf(buffer, "%d", number);
    SynchPrint(buffer);
    SynchPrint("\n");
}

void SynchPrint(float number)
{
    char buffer[MaxFileLength];
    sprintf(buffer, "%f", number);
    SynchPrint(buffer);
    SynchPrint("\n");
}
/// @brief Increase Program Counter (needed for each system call)
void IncreasePC()
{
    int counter = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
    machine->WriteRegister(NextPCReg, counter + 4);
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

    // Check if buffer is NULL (not enough memory in system)
    if (buffer == NULL)
    {
        printf("Not enough memory in system\n");
        machine->WriteRegister(2, 0); // Return 0 to register 2
        return;
    }

    // SynchPrint("Enter a character: ");
    numBytes = gSynchConsole->Read(buffer, maxBytes); // Read buffer from console and return the number of bytes read

    // Check if the number of bytes read is greater than 1
    if (numBytes > 1)
    {
        SynchPrint("You can only enter 1 character!");
        DEBUG('a', "\nERROR: You can only enter 1 character!");
        machine->WriteRegister(2, 0);
    }
    // Check if the number of bytes read is 0: empty character
    else if (numBytes == 0)
    {
        SynchPrint("Empty character!");
        DEBUG('a', "\nERROR: Empty character!");
        machine->WriteRegister(2, 0);
    }
    // If the number of bytes read is 1, return the character to register 2
    else
    {
        char c = *buffer;
        machine->WriteRegister(2, c);
    }

    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call PrintChar from user program
void Handle_SC_PrintChar()
{
    // Read character from register 4
    char c = (char)machine->ReadRegister(4);
    // Check if the character is not null
    if (c != 0)
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

    // SynchPrint("Enter a string: ");

    virtAddr = machine->ReadRegister(4);    // Read virtual address of input string
    length = machine->ReadRegister(5);      // Read maximum length of input string
    buffer = User2System(virtAddr, length); // Copy buffer from User memory space to System memory space
    gSynchConsole->Read(buffer, length);    // Use SynchConsole to read buffer from console
    System2User(virtAddr, length, buffer);  // Copy buffer from System memory space to User memory space
    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call PrintString from user program
void Handle_SC_PrintString()
{
    int virtAddr;
    char *buffer;
    int length = 0;

    virtAddr = machine->ReadRegister(4); // Read virtual address of string

    buffer = User2System(virtAddr, 255); // Copy buffer from User memory space to System memory space

    while (buffer[length] != 0) // Calculate the length of buffer
        length++;

    gSynchConsole->Write(buffer, length + 1); // Print buffer to console

    delete buffer;
    return IncreasePC();
}

/// @brief Handle system call ReadInt from user program
void Handle_SC_ReadInt()
{
    // SynchPrint("Enter an integer: ");
    int result = 0;                                  // Store the result
    char _numberBuffer[MAX_NUM_LENGTH + 2];          // Buffer to store number buffer
    memset(_numberBuffer, 0, sizeof(_numberBuffer)); // Fill buffer with 0

    int len = gSynchConsole->Read(_numberBuffer, MAX_NUM_LENGTH + 1); // Read buffer from console
    // If nothing is read, return 0
    if (len == 0)
        return;

    // Convert buffer to number
    bool isNegative = (_numberBuffer[0] == '-');

    for (int i = isNegative; i < len; ++i)
    {
        char c = _numberBuffer[i];
        if (c < '0' || c > '9')
        {
            DEBUG('a', "Expected number but %s found\n", _numberBuffer);
            return;
        }
        result = result * 10 + (c - '0');
    }
    if (isNegative)
        result = -result;

    machine->WriteRegister(2, result); // Write result to register 2
    return IncreasePC();
}

/// @brief Handle system call PrintInt from user program
void Handle_SC_PrintInt()
{
    // SynchPrint("Integer that you entered: ");
    int number = machine->ReadRegister(4); // Read value from register 4 and store it in number
    int *_number = &number;
    SynchPrint(*_number); // Print number to console
    return IncreasePC();
}

/// @brief Handle system call ReadFloat from user program
void Handle_SC_ReadFloat()
{
    SynchPrint("Enter a float: ");
    int result = 0;
    char _numberBuffer[MAX_NUM_LENGTH + 2];
    memset(_numberBuffer, 0, sizeof(_numberBuffer));

    int len = gSynchConsole->Read(_numberBuffer, MAX_NUM_LENGTH + 1);
    // Read nothing -> return 0
    if (len == 0)
        return;
    // Check comment below to understand this line of code
    bool isNegative = (_numberBuffer[0] == '-');
    int i = isNegative;
    for (i; i < len; ++i)
    {
        char c = _numberBuffer[i];
        if (c == '.')
        {
            break;
        }
        else if (c < '0' || c > '9')
        {
            DEBUG('a', "Expected number but %s found\n", _numberBuffer);
            return;
        }
        result = result * 10 + (c - '0');
    }

    int decimal = 0;
    int count = 0;
    for (i = i + 1; i < len; ++i)
    {
        char c = _numberBuffer[i];
        if (c < '0' || c > '9')
        {
            DEBUG('a', "Expected number but %s found\n", _numberBuffer);
            return;
        }
        decimal = decimal * 10 + (c - '0');
        count++;
    }
    if (isNegative)
        result = -result;
    machine->WriteRegister(2, result);
    SynchPrint("Float that you entered: ");
    SynchPrint(result);
    SynchPrint(".");
    SynchPrint(decimal);
    return IncreasePC();
}

/// @brief Handle system call PrintFloat from user program
void Handle_SC_PrintFloat()
{
    SynchPrint("Float that you entered: ");
    int number = machine->ReadRegister(4);
    float *f = (float *)number;
    SynchPrint(*f);
    SynchPrint(".");
    // SynchPrint(decimal);
    return IncreasePC();
}

/// @brief Handle system call CreateFile from user program
void Handle_SC_CreateFile()
{
    int virtualAddr;
    char *filename;

    DEBUG('a', "\n SC_Create call ...");
    DEBUG('a', "\n Reading virtual address of filename");

    // Take the virtual address of the filename from register 4
    virtualAddr = machine->ReadRegister(4);
    DEBUG('a', "\n Reading filename.");

    filename = User2System(virtualAddr, 32 + 1);
    if (filename == NULL)
    {
        printf("\n Not enough memory in system");
        machine->WriteRegister(2, -1); // return the result
        delete filename;
        return;
    }

    DEBUG('a', "\n Finish reading filename.");
    // Create the file with size = 0
    if (!fileSystem->Create(filename, 0))
    {
        printf("\n Error create file '%s'", filename);
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }

    // printf("\n Create file '%s' success\n", filename);
    machine->WriteRegister(2, 0); // return the result to system call register 2
    delete filename;
    return IncreasePC();
}

void Handle_SC_Open()
{
    int virtAddr = machine->ReadRegister(4);
    int type = machine->ReadRegister(5);
    char *filename;
    filename = User2System(virtAddr, MaxFileLength);

    int freeSlot = fileSystem->FindFreeSlot();
    if (freeSlot != -1)
    {
        if (type == 0 || type == 1)
        {

            if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL)
            {
                SynchPrint("Open file success");
                machine->WriteRegister(2, freeSlot);
            }
        }
        else if (type == 2) // ConsoleInput: stdin
        {
            machine->WriteRegister(2, 0);
        }
        else // ConsoleOutput: stdout
        {
            machine->WriteRegister(2, 1);
        }
    }
    else // No free slot
    {
        SynchPrint("No free slot");
        machine->WriteRegister(2, -1); // Failed to open file, return -1
    }

    delete[] filename;
    return IncreasePC();
}

void Handle_SC_Close()
{
    int id = machine->ReadRegister(4);
    if (id >= 0 && id < 10 && fileSystem->openf[id] != NULL)
    {
        delete fileSystem->openf[id];
        fileSystem->openf[id] = NULL;
        machine->WriteRegister(2, 0);
        SynchPrint("Close file success");
    }
    else
    {
        machine->WriteRegister(2, -1);
        SynchPrint("Close file failed");
    }
    return IncreasePC();
}

/// @brief Exception handler for user program system calls
/// @param which Type of exception
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which)
    {
    case SyscallException:
        switch (type)
        {
        case SC_Halt:
            return Handle_SC_Halt();
        case SC_ReadInt:
            return Handle_SC_ReadInt();
        case SC_PrintInt:
            return Handle_SC_PrintInt();
        case SC_ReadFloat:
            SynchPrint("ReadFloat\n");
            return Handle_SC_ReadFloat();
        case SC_PrintFloat:
            SynchPrint("PrintFloat\n");
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
        default:
            // printf("\nUnexpected user mode exception %d %d\n", which, type);
            printf("\n");
            interrupt->Halt();
            break;
        }
    case NoException:
        return;
    case PageFaultException:
        printf("PageFaultException\n");
        interrupt->Halt();
        break;
    case ReadOnlyException:
        printf("ReadOnlyException\n");
        interrupt->Halt();
        break;
    case BusErrorException:
        printf("BusErrorException\n");
        interrupt->Halt();
        break;
    case AddressErrorException:
        printf("AddressErrorException\n");
        interrupt->Halt();
        break;
    case OverflowException:
        printf("OverflowException\n");
        interrupt->Halt();
        break;
    case IllegalInstrException:
        printf("IllegalInstrException\n");
        interrupt->Halt();
        break;
    case NumExceptionTypes:
        printf("NumExceptionTypes\n");
        interrupt->Halt();
        break;

    default:
        printf("Unexpected user mode exception %d %d\n", which, type);
        interrupt->Halt();
        break;
    }
}
