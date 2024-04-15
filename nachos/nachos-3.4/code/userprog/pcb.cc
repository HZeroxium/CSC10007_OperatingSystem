#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

PCB::PCB(int id)
{
    joinsem = new Semaphore("JoinSem", 0);
    exitsem = new Semaphore("ExitSem", 0);
    mutex = new Semaphore("Mutex", 1);

    pid = id;
    exitcode = 0;
    numwait = 0;

    if (id)
        parentID = currentThread->processID;
    else
        parentID = 0;

    thread = NULL;
    JoinStatus = -1;
}

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

int PCB::GetID()
{
    return pid;
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

void PCB::SetExitCode(int ec)
{
    exitcode = ec;
}

void PCB::SetFileName(char *name)
{
    strcpy(filename, name);
}

void PCB::IncNumWait()
{
    mutex->P();
    numwait++;
    mutex->V();
}

void PCB::DecNumWait()
{
    mutex->P();
    if (numwait)
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

char *PCB::GetNameThread()
{
    return thread->getName();
}