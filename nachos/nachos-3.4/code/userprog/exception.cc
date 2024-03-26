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

bool compareNumAndString(int integer, const char *s)
{
    if (integer == 0)
        return strcmp(s, "0") == 0;

    int len = strlen(s);

    if (integer < 0 && s[0] != '-')
        return false;

    if (integer < 0)
        s++, --len, integer = -integer;

    while (integer > 0)
    {
        int digit = integer % 10;

        if (s[len - 1] - '0' != digit)
            return false;

        --len;
        integer /= 10;
    }

    return len == 0;
}

void SynchPrint(char *buffer)
{
    gSynchConsole->Write(buffer, strlen(buffer) + 1);
}

void SynchPrint(int number)
{
    char buffer[MaxFileLength];
    sprintf(buffer, "%d", number);
    SynchPrint(buffer);
    SynchPrint("\n");
}

int SysReadNum()
{
    char _numberBuffer[MAX_NUM_LENGTH + 2];
    memset(_numberBuffer, 0, sizeof(_numberBuffer));

    int len = gSynchConsole->Read(_numberBuffer, MAX_NUM_LENGTH + 1);
    // Read nothing -> return 0
    if (len == 0)
        return 0;

    // Check comment below to understand this line of code
    bool isNegative = (_numberBuffer[0] == '-');

    int result = 0;

    for (int i = isNegative; i < len; ++i)
    {
        char c = _numberBuffer[i];
        if (c < '0' || c > '9')
        {
            DEBUG('a', "Expected number but %s found\n", _numberBuffer);
            return 0;
        }
        result = result * 10 + (c - '0');
    }
    return result;
}

void PrintPC()
{
    int prevCounter = machine->ReadRegister(PrevPCReg);
    int counter = machine->ReadRegister(PCReg);
    int nextCounter = machine->ReadRegister(NextPCReg);
    SynchPrint("+ PrevPC: ");
    SynchPrint(prevCounter);
    SynchPrint("+ PC: ");
    SynchPrint(counter);
    SynchPrint("+ NextPC: ");
    SynchPrint(nextCounter);
}

void IncreasePC()
{
    int counter = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
    machine->WriteRegister(NextPCReg, counter + 4);
}

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char *User2System(int virtAddr, int limit)
{
    int i; // chi so index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; // can cho chuoi terminal
    if (kernelBuf == NULL)
        return kernelBuf;

    memset(kernelBuf, 0, limit + 1);

    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
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

void Handle_SC_Halt()
{
    DEBUG('a', "Shutdown, initiated by user program.\n");
    printf("Shutdown, initiated by user program.\n");
    interrupt->Halt();
}

void Handle_SC_ReadChar()
{
    int maxBytes = 255;
    char *buffer = new char[255];
    int numBytes = 0;
    if (buffer == NULL)
    {
        printf("Not enough memory in system\n");
        machine->WriteRegister(2, 0);
        return;
    }
    SynchPrint("Enter a character: ");
    numBytes = gSynchConsole->Read(buffer, maxBytes);
    if (numBytes > 1)
    {
        SynchPrint("You can only enter 1 character!");
        DEBUG('a', "\nERROR: You can only enter 1 character!");
        machine->WriteRegister(2, 0);
    }
    else if (numBytes == 0)
    {
        SynchPrint("Empty character!");
        DEBUG('a', "\nERROR: Empty character!");
        machine->WriteRegister(2, 0);
    }
    else
    {
        char c = *buffer;
        machine->WriteRegister(2, c);
    }

    delete buffer;
    return IncreasePC();
}

void Handle_SC_PrintChar()
{
    char c = (char)machine->ReadRegister(4);
    if (c != 0)
    {
        SynchPrint("Character that you entered: ");
        gSynchConsole->Write(&c, 1);
    }

    return IncreasePC();
}

void Handle_SC_ReadString()
{
    int virtAddr, length;
    char *buffer;
    SynchPrint("Enter a string: ");
    virtAddr = machine->ReadRegister(4);    // Lay dia chi tham so buffer truyen vao tu thanh ghi so 4
    length = machine->ReadRegister(5);      // Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5
    buffer = User2System(virtAddr, length); // Copy chuoi tu vung nho User Space sang System Space
    gSynchConsole->Read(buffer, length);    // Goi ham Read cua SynchConsole de doc chuoi
    System2User(virtAddr, length, buffer);  // Copy chuoi tu vung nho System Space sang vung nho User Space
    delete buffer;
    return IncreasePC();
}

void Handle_SC_PrintString()
{
    int virtAddr;
    char *buffer;
    virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
    buffer = User2System(virtAddr, 255); // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai 255 ki tu
    int length = 0;
    while (buffer[length] != 0)
        length++; // Dem do dai that cua chuoi
    SynchPrint("String that you entered: ");
    gSynchConsole->Write(buffer, length + 1); // Goi ham Write cua SynchConsole de in chuoi
    delete buffer;
    return IncreasePC();
}

void Handle_SC_ReadInt()
{
    SynchPrint("Enter an integer: ");
    int result = 0;
    char _numberBuffer[MAX_NUM_LENGTH + 2];
    memset(_numberBuffer, 0, sizeof(_numberBuffer));

    int len = gSynchConsole->Read(_numberBuffer, MAX_NUM_LENGTH + 1);
    // Read nothing -> return 0
    if (len == 0)
        return;

    // Check comment below to understand this line of code
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
    machine->WriteRegister(2, result);
    return IncreasePC();
}

void Handle_SC_PrintInt()
{
    SynchPrint("Integer that you entered: ");
    int number = machine->ReadRegister(4);
    SynchPrint(number);
    return IncreasePC();
}

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
    machine->WriteRegister(3, decimal);
    SynchPrint("Float that you entered: ");
    SynchPrint(result);
    SynchPrint(".");
    SynchPrint(decimal);
    return IncreasePC();
}

void Handle_SC_PrintFloat()
{
    SynchPrint("Float that you entered: ");
    int number = machine->ReadRegister(4);
    SynchPrint(number);
    SynchPrint(".");
    return IncreasePC();
}

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

    printf("\n Create file '%s' success\n", filename);
    machine->WriteRegister(2, 0); // return the result to system call register 2
    delete filename;
    return IncreasePC();
}

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
