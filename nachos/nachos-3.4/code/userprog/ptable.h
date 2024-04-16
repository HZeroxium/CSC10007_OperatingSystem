#ifndef PTABLE_H
#define PTABLE_H

#include "bitmap.h"
#include "pcb.h"
#include "synch.h"

#define MAX_PROCESS 10

class PTable
{
private:
    int psize;             // Size of pcb object to save process size
    BitMap *bm;            // Check the position that has been used in pcb
    PCB *pcb[MAX_PROCESS]; // Process Control Block

    Semaphore *bmsem; // Prevent the case of loading 2 processes at the same time

public: // Constructor and Destructor
    PTable(int = 10); // Initialize the size of pcb object to save process size
                      // Set the initial value to null
    ~PTable();        // Destroy the created objects

public: 
    int ExecUpdate(char *); // Handle for system call SC_Exit
    int ExitUpdate(int);    // Handle for system call SC_Exit
    int JoinUpdate(int);    // Handle for system call SC_Join

    int GetFreeSlot();     // Find a free slot to save information for a new process
    bool IsExist(int pid); // Check if this processID exists or not?

    void Remove(int pid); // When the process ends, delete the processID from the array that manages it

    char *GetFileName(int id); // Return the name of the process
};

#endif // PTABLE_H