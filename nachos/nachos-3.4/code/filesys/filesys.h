// filesys.h
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system.
//	The "STUB" version just re-defines the Nachos file system
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of
//	a disk simulator.  The disk is simulated using the native UNIX
//	file system (in a file named "DISK").
//
//	In the "real" implementation, there are two key data structures used
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"

#define MAX_FILE 15

typedef int OpenFileID;
#ifdef FILESYS_STUB // Temporarily implement file system calls as
// calls to UNIX, until the real file system
// implementation is available

class FileSystem
{
public:
	OpenFile **file_table; // A table to store all the file
	int index;			   // Index of the file table

	/// @brief Default constructor
	/// @param format Format the file system
	FileSystem(bool format)
	{
		file_table = new OpenFile *[MAX_FILE];
		index = 0;
		for (int i = 0; i < MAX_FILE; ++i)
		{
			file_table[i] = NULL;
		}
		this->Create("stdin", 0);							// Create stdin with initial size 0
		this->Create("stdout", 0);							// Create stdout with initial size 0
		file_table[index++] = this->Open("stdin", STDIN);	// Open stdin with type STDIN at slot 0
		file_table[index++] = this->Open("stdout", STDOUT); // Open stdout with type STDOUT at slot 1
	}

	/// @brief Destructor
	~FileSystem()
	{
		for (int i = 0; i < MAX_FILE; ++i)
		{
			if (file_table[i] != NULL)
				delete file_table[i];
		}
		delete[] file_table;
	}

	/// @brief Create a new file with the given name and initial size
	/// @param name Name of the file in ./code directory
	/// @param initialSize Initial size of the file
	/// @return True if the file is created successfully, otherwise false
	bool Create(char *name, int initialSize)
	{
		int fileDescriptor = OpenForWrite(name);

		if (fileDescriptor == -1)
			return FALSE;
		Close(fileDescriptor);
		return TRUE;
	}

	/// @brief Open the file with the given name
	/// @param name Name of the file in ./code directory
	/// @return OpenFile pointer of the opened file
	OpenFile *Open(char *name)
	{
		int fileDescriptor = OpenForReadWrite(name, FALSE);

		if (fileDescriptor == -1)
			return NULL;
		return new OpenFile(fileDescriptor);
	}

	/// @brief Open the file with the given name and open mode
	/// @param name Name of the file in ./code directory
	/// @param type Type of the file (0: Read/Write, 1: Read only, 2: Console Input, 3: Console Output)
	/// @return OpenFile pointer of the opened file
	OpenFile *Open(char *name, int type)
	{
		int fileDescriptor = OpenForReadWrite(name, FALSE);

		if (fileDescriptor == -1)
		{
			return NULL;
		}

		return new OpenFile(fileDescriptor, type);
	}

	/// @brief Get the allocated slot in the file table
	/// @return Allocated slot in the file table
	int GetAllocatedSlot()
	{
		for (int i = 2; i < 15; i++)
		{
			if (file_table[i] == NULL)
				return i;
		}
		return -1;
	}

	/// @brief Delete a file (UNIX unlink)
	bool Remove(char *name)
	{
		return Unlink(name) == 0;
	}
};

#else // FILESYS
class FileSystem
{
public:
	OpenFile **file_table; // A table to store all the file
	int index;			   // Index of the file table

	FileSystem(bool format); // Initialize the file system.
							 // Must be called *after* "synchDisk"
							 // has been initialized.
							 // If "format", there is nothing on
							 // the disk, so initialize the directory
							 // and the bitmap of free blocks.

	/// @brief Create a new file with the given name and initial size
	bool Create(char *name, int initialSize);

	/// @brief Open the file with the given name
	/// @param name Name of the file in ./code directory
	/// @return OpenFile pointer of the opened file
	OpenFile *Open(char *name);

	/// @brief Open the file with the given name and open mode
	/// @param name Name of the file in ./code directory
	/// @param type Type of the file (0: Read/Write, 1: Read only, 2: Console Input, 3: Console Output)
	/// @return OpenFile pointer of the opened file
	OpenFile *Open(char *name, int type);

	/// @brief Get the allocated slot in the file table
	/// @return Allocated slot in the file table
	int GetAllocatedSlot();

	/// @brief Delete a file (UNIX unlink)
	/// @param name Name of the file in ./code directory
	/// @return True if the file is deleted successfully, otherwise false
	bool Remove(char *name); // Delete a file (UNIX unlink)

	/// @brief List all the files in the file system
	void List(); // List all the files in the file system

	/// @brief Print all the files and their contents
	void Print(); // List all the files and their contents

private:
	OpenFile *freeMapFile;	 // Bit map of free disk blocks,
							 // represented as a file
	OpenFile *directoryFile; // "Root" directory -- list of
							 // file names, represented as a file
};

#endif // FILESYS

#endif // FS_H
