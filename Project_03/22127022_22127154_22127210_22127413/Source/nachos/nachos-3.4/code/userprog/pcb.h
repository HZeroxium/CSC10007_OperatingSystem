#ifndef PCB_H
#define PCB_H

#include "thread.h"
#include "synch.h"

/// @brief Process Control Block
class PCB
{
private:
    Semaphore *joinsem; // Semaphore for join process
    Semaphore *exitsem; // Semaphore for exit process
    Semaphore *mutex;   // Semaphore for mutual exclusion access

    int exitcode;
    int numwait; // Number of waiting processes

    Thread *thread; // Thread of the program
    char filename[32];

public:
    int parentID; // ID of the parent process

public:
    PCB(int id);
    ~PCB();

public: // Getters and Setters
    int GetID();
    int GetNumWait();
    int GetExitCode();
    char *GetNameThread();
    char *GetFileName();

    void SetExitCode(int ec);
    void SetFileName(char *name);

public: // Process control functions
    int Exec(char *filename, int pID);

    void JoinWait();
    void ExitWait();

    void JoinRelease();
    void ExitRelease();

    void IncNumWait();
    void DecNumWait();
};

#endif