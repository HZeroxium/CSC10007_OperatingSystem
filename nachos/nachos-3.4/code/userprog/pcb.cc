#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

extern void StartProcess_2(int id);

//************************************************************************************************
//*************************** CONSTRUCTOR AND DESTRUCTOR *****************************************
//************************************************************************************************

/// @brief Constructor
/// @param id Process ID
PCB::PCB(int id)
{
    joinsem = new Semaphore("JoinSem", 0);
    exitsem = new Semaphore("ExitSem", 0);
    mutex = new Semaphore("Mutex", 1);

    exitcode = 0;
    numwait = 0;

    if (id)
        parentID = currentThread->processID;
    else
        parentID = 0;

    thread = NULL;
}

/// @brief Destructor
PCB::~PCB()
{
    if (joinsem != NULL)
        delete joinsem;
    if (exitsem != NULL)
        delete exitsem;
    if (mutex != NULL)
        delete mutex;
    if (thread != NULL)
    {
        thread->FreeSpace();
        thread->Finish();
    }
}

//************************************************************************************************
//************************************ GETTERS ***************************************************
//************************************************************************************************

int PCB::GetID()
{
    return thread->processID;
}

int PCB::GetNumWait()
{
    return numwait;
}

int PCB::GetExitCode()
{
    return exitcode;
}

char *PCB::GetFileName()
{
    return filename;
}

char *PCB::GetNameThread()
{
    return thread->getName();
}

//************************************************************************************************
//************************************ SETTERS ***************************************************
//************************************************************************************************

void PCB::SetExitCode(int ec)
{
    exitcode = ec;
}

void PCB::SetFileName(char *name)
{
    strcpy(filename, name);
}

//************************************************************************************************
//************************************ PROCESS CONTROL *******************************************
//************************************************************************************************

void PCB::IncNumWait()
{
    mutex->P();
    numwait++;
    mutex->V();
}

void PCB::DecNumWait()
{
    mutex->P();
    if (numwait > 0)
        numwait--;
    mutex->V();
}

void PCB::JoinWait()
{
    joinsem->P();
}

void PCB::JoinRelease()
{
    joinsem->V();
}

void PCB::ExitWait()
{
    exitsem->P();
}

void PCB::ExitRelease()
{
    exitsem->V();
}

int PCB::Exec(char *filename, int pID)
{
    mutex->P();

    thread = new Thread(filename);

    if (thread == NULL)
    {
        printf("\nPCB::Exec : Can't not create new thread.\n");
        mutex->V();
        return -1;
    }
    thread->processID = pID;

    parentID = currentThread->processID;

    thread->Fork(StartProcess_2, pID);

    mutex->V();

    return pID;
}