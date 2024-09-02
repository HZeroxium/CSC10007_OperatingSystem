/* syscalls.h
 * 	Nachos system call interface.  These are Nachos kernel operations
 * 	that can be invoked from user programs, by trapping to the kernel
 *	via the "syscall" instruction.
 *
 *	This file is included by user programs and by the Nachos kernel.
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt 0
#define SC_Exit 1
#define SC_Exec 2
#define SC_Join 3
#define SC_CreateFile 4
#define SC_Open 5
#define SC_Read 6
#define SC_Write 7
#define SC_Close 8
#define SC_Fork 9
#define SC_Yield 10

#define SC_ReadInt 41
#define SC_PrintInt 42
#define SC_ReadFloat 43
#define SC_PrintFloat 44
#define SC_ReadChar 45
#define SC_PrintChar 46
#define SC_ReadString 47
#define SC_PrintString 48
#define SC_CompareFloat 49
#define SC_FreeFloat 50
#define SC_FloatToString 51

#ifndef IN_ASM

/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking,
 * from the system call entry point in exception.cc.
 */

/// @brief Stop Nachos, and print out performance stats
void Halt();

/// @brief Read an integer from the console input and write it to register 2
/// @return Integer read from the console input
int ReadInt();

/// @brief Print an integer to the console output
/// @param number Integer is stored in register 4
void PrintInt(int number);

/// @brief Read a float from the console input
/// @return Pointer to the float read from the console input
float *ReadFloat();

/// @brief Print a float to the console output
/// @param number Pointer to the float is stored in register 4
void PrintFloat(float *number);

/// @brief Read a character from the console input and write it to register 2
/// @return Character read from the console input
char ReadChar();

/// @brief Write a character to the console output
/// @param character Character is stored in register 4
void PrintChar(char character);

/// @brief Read a string from the console input
/// @param buffer Buffer to store the string (Stored in register 4)
/// @param size Size of the buffer (Length of the string) (Stored in register 5)
void ReadString(char *buffer, int size);

/// @brief Write a string to the console output
/// @param buffer Buffer contains the string (Stored in register 4)
void PrintString(char *buffer);

/// @brief Compare two floats
/// @param a Pointer to the first float (Stored in register 4)
/// @param b Pointer to the second float (Stored in register 5)
/// @return 0 if the two floats are equal, -1 if the first float is less than the second float, 1 otherwise
int CompareFloat(float *a, float *b);

/// @brief Convert a float to a string
/// @param number Pointer to the float (Stored in register 4)
/// @return Pointer to the string representation of the float
void FloatToString(float *number, char *buffer);

/// @brief Deallocate the memory of a float
/// @param a Pointer to the float (Stored in register 4)
/// @return 0 if the float is deallocated successfully, -1 otherwise
int FreeFloat(float *a);
/* File system operations: Create, Open, Read, Write, Close
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 */

/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;

/* when an address space starts up, it has two open files, representing
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */

#define ConsoleInput 0
#define ConsoleOutput 1

/// @brief Create a new file with the given name
/// @param name Name of the file (Stored in register 4)
/// @return 0 if the file is created successfully, -1 otherwise
int CreateFile(char *name);

/// @brief Open the file with the given name and open mode
/// @param name Name of the file in ./code directory (Stored in register 4)
/// @param type Type of the file (0: Read/Write, 1: Read only, 2: Console Input, 3: Console Output) (Stored in register 5)
/// @return OpenFileId of the opened file (a slot in file table)
/// @note + Return -1 if the file is not found or the file is already opened
/// @note + Return -2 if unknown error occurs
OpenFileId Open(char *name, int type);

/// @brief Close the file with the given OpenFileId
/// @param id OpenFileId of the file to be closed (Stored in register 4)
void Close(OpenFileId id);

/// @brief Read data from the file and store it in the buffer
/// @param buffer Buffer to store the read data (Stored in register 4)
/// @param size Size of the data to be read - Number of bytes (Stored in register 5)
/// @param id OpenFileId of the file to be read (Stored in register 6)
/// @return Number of bytes actually read
/// @note + Return -1 if the file is not found or id is out of range or the file is not opened in read mode (stdout)
/// @note + Return -2 if the file is empty (EOF)
/// @note + Return -3 if unknown error occurs
int Read(char *buffer, int size, OpenFileId id);

/// @brief Write data from the buffer to the file
/// @param buffer Buffer containing the data to be written (Stored in register 4)
/// @param size Size of the data to be written - Number of bytes (Stored in register 5)
/// @param id OpenFileId of the file to be written (Stored in register 6)
/// @return Number of bytes actually written
/// @note + Return -1 if the file is not found or id is out of range or the file is not opened in write mode (stdin)
/// @note + Return -3 if unknown error occurs
int Write(char *buffer, int size, OpenFileId id);

//==================================================================================================
//==================================================================================================
//==================================================================================================

/* Address space control operations: Exit, Exec, and Join */

/* This user program is done (status = 0 means exited normally). */
void Exit(int status);

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;

/* Run the executable, stored in the Nachos file "name", and return the
 * address space identifier
 */
SpaceId Exec(char *name);

/* Only return once the the user program "id" has finished.
 * Return the exit status.
 */
int Join(SpaceId id);

/* User-level thread operations: Fork and Yield.  To allow multiple
 * threads to run within a user program.
 */

/* Fork a thread to run a procedure ("func") in the *same* address space
 * as the current thread.
 */
void Fork(void (*func)());

/* Yield the CPU to another runnable thread, whether in this address space
 * or not.
 */
void Yield();

#endif /* IN_ASM */

#endif /* SYSCALL_H */
