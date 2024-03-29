// CAE - MULTI - SYNCHCONSOLE DEFINITION

#include "console.h"

class SynchConsole
{
public:
	SynchConsole();					   // A SynchConsole Constructor
	SynchConsole(char *in, char *out); // Same with fn pointers
	~SynchConsole();				   // Delete a console instance

	/// @brief Read a buffer from the console input device and store it in the kernel buffer
	/// @param into Buffer to store the input
	/// @param numBytes Number of bytes to read
	/// @return Number of bytes actually read
	int Read(char *into, int numBytes);

	/// @brief Write a buffer from the kernel to the console output device
	/// @param from Kernel buffer to write
	/// @param numBytes Number of bytes to write
	/// @return Number of bytes actually written
	int Write(char *from, int numBytes);

public:
	Console *cons; // Pointer to an async console
};

// CAE - MULTI - END SECTION
