#include "ptable.h"
#include "system.h"
#include "openfile.h"

PTable::PTable(int size)
{
    if (size < 0)
        return;

    psize = size;
    bm = new BitMap(size);
    bmsem = new Semaphore("bmsem", 1);

    for (int i = 0; i < MAX_PROCESS; i++)
    {
        pcb[i] = 0;
    }

    bm->Mark(0);

    pcb[0] = new PCB(0);
    pcb[0]->SetFileName("./test/scheduler");
    pcb[0]->parentID = -1;
}

PTable::~PTable()
{
    if (bm != 0)
        delete bm;

    for (int i = 0; i < psize; i++)
    {
        if (pcb[i] != 0)
            delete pcb[i];
    }

    if (bmsem != 0)
        delete bmsem;
}

int PTable::ExecUpdate(char *name)
{
    // Call mutex->P(); to avoid loading 2 processes at the same time.
    bmsem->P();

    // Check the validity of the program "name".
    if (name == NULL)
    {
        printf("\nPTable::Exec : Can't not execute name is NULL.\n");
        bmsem->V();
        return -1;
    }

    // Check the existence of the program "name" by calling the Open method of the fileSystem class.
    if (strcmp(name, "./test/scheduler") == 0 || strcmp(name, currentThread->getName()) == 0)
    {
        printf("\nPTable::Exec : Can't not execute itself.\n");
        bmsem->V();
        return -1;
    }

    int ID = GetFreeSlot();

    if (ID == -1)
    {
        printf("\nPTable::Exec : Can't not get free slot.\n");
        bmsem->V();
        return -1;
    }

    pcb[ID] = new PCB(ID);
    pcb[ID]->SetFileName(name);
    pcb[ID]->parentID = currentThread->processID;

    int pID = pcb[ID]->Exec(name, ID);

    bmsem->V();
    return ID;
}

int PTable::JoinUpdate(int id)
{
    if (id < 0 || id >= psize)
    {
        printf("\nPTable::JoinUpdate : Can't not join id is invalid.\n");
        return -1;
    }

    if (pcb[id] == 0)
    {
        printf("\nPTable::JoinUpdate : Can't not join pcb[id] is NULL.\n");
        return -1;
    }

    if (pcb[id]->parentID != currentThread->processID)
    {
        printf("\nPTable::JoinUpdate : Can't not join pcb[id]->parentID is invalid.\n");
        return -1;
    }

    pcb[pcb[id]->parentID]->IncNumWait();

    pcb[id]->JoinWait();

    int exitcode = pcb[id]->GetExitCode();

    pcb[id]->ExitRelease();

    return exitcode;
}

int PTable::ExitUpdate(int exitcode)
{
    int pID = currentThread->processID;

    if (pID == 0)
    {
        currentThread->FreeSpace();
        interrupt->Halt();
        return 0;
    }

    if (IsExist(pID) == false)
    {
        printf("\nPTable::ExitUpdate : Can't not exit pID is invalid.\n");
        return -1;
    }

    pcb[pID]->SetExitCode(exitcode);
    pcb[pcb[pID]->parentID]->DecNumWait();

    pcb[pID]->JoinRelease();

    pcb[pID]->ExitWait();

    Remove(pID);

    return exitcode;
}

int PTable::GetFreeSlot()
{
    return bm->Find();
}

bool PTable::IsExist(int pID)
{
    return bm->Test(pID);
}

void PTable::Remove(int pID)
{
    bm->Clear(pID);
    if (pcb[pID] != 0)
    {
        delete pcb[pID];
        pcb[pID] = 0;
    }
}

char *PTable::GetFileName(int pID)
{
    return pcb[pID]->GetFileName();
}