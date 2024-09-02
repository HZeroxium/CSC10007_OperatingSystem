#include "ptable.h"
#include "system.h"
#include "openfile.h"

//************************************************************************************************
//*************************** CONSTRUCTOR AND DESTRUCTOR *****************************************
//************************************************************************************************

/// @brief Constructor
/// @param size Size of the process table
PTable::PTable(int size)
{
    int i;
    if (size < 0)
        return;

    psize = size;
    bm = new BitMap(size);
    bmsem = new Semaphore("bmsem", 1);

    for (i = 0; i < MAX_PROCESS; i++)
    {
        pcb[i] = 0;
    }

    // Mark the first slot as used for the parent process (scheduler)
    bm->Mark(0);

    pcb[0] = new PCB(0);
    pcb[0]->SetFileName("./test/scheduler");
    pcb[0]->parentID = -1;
}

/// @brief Destructor
PTable::~PTable()
{
    int i;
    if (bm != 0)
        delete bm;

    for (i = 0; i < psize; i++)
    {
        if (pcb[i] != 0)
            delete pcb[i];
    }

    if (bmsem != 0)
        delete bmsem;
}

//************************************************************************************************
//************************************ GETTERS ***************************************************
//************************************************************************************************

/// @brief Get the free slot in the process table to save information for a new process
/// @return The free slot in the process table
int PTable::GetFreeSlot()
{
    return bm->Find();
}

/// @brief Check if the process ID exists or not
/// @param pID Process ID
/// @return True if the process ID exists, otherwise False
bool PTable::IsExist(int pID)
{
    return bm->Test(pID);
}

/// @brief Remove the process from the process table
/// @param pID Process ID
void PTable::Remove(int pID)
{
    bm->Clear(pID);
    if (pcb[pID] != 0)
    {
        delete pcb[pID];
        pcb[pID] = 0;
    }
}

/// @brief Get the file name of the process
/// @param pID Process ID
/// @return The file name of the process
char *PTable::GetFileName(int pID)
{
    return pcb[pID]->GetFileName();
}

//************************************************************************************************
//********************************* SYSTEM CALL HANDLERS *****************************************
//************************************************************************************************

/// @brief Execute a new process with the given name
/// @param name The name of the process
/// @return The process ID of the new process
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

    // Compare the program name and the name of the currentThread to make sure that this program does not call itself.
    if (strcmp(name, "./test/scheduler") == 0 || strcmp(name, currentThread->getName()) == 0)
    {
        printf("\nPTable::Exec : Can't not execute itself.\n");
        bmsem->V();
        return -1;
    }

    // Find a free slot in the Ptable.
    int ID = GetFreeSlot();

    // Check if have free slot
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

/// @brief Join the process with the given ID
/// @param id The process ID
/// @return The exit code of the process
int PTable::JoinUpdate(int id)
{

    // Check if the process ID is valid.
    if (id < 0 || id >= psize || pcb[id] == NULL)
    {
        printf("\nPTable::JoinUpdate : Can't not join id is invalid.\n");
        return -1;
    }

    if (pcb[id]->parentID != currentThread->processID)
    {
        printf("\nPTable::JoinUpdate : Can't not join pcb[id]->parentID is invalid.\n");
        return -1;
    }

    // Increase the number of processes waiting for the process to finish in the parent process.
    pcb[pcb[id]->parentID]->IncNumWait();

    // Wait for the process to finish.
    pcb[id]->JoinWait();

    // After the process is finished, the process is released.

    // Get the exit code of the process.
    int exitcode = pcb[id]->GetExitCode();

    // Release the process
    pcb[id]->ExitRelease();

    return exitcode;
}

/// @brief Update the exit code of the process
/// @param exitcode The exit code of the process
/// @return The exit code of the process
int PTable::ExitUpdate(int exitcode)
{
    // Get the process ID of the currentThread.
    int pID = currentThread->processID;

    // If the process is the main process, call Halt().
    if (pID == 0)
    {
        currentThread->FreeSpace();
        interrupt->Halt();
        return 0;
    }

    // If the process ID is invalid, return -1.
    if (IsExist(pID) == false)
    {
        printf("\nPTable::ExitUpdate : Can't not exit pID is invalid.\n");
        return -1;
    }

    // Set the exit code for the process.
    pcb[pID]->SetExitCode(exitcode);
    pcb[pcb[pID]->parentID]->DecNumWait();

    // Release the parent process (if any) that is waiting for the process to finish.
    pcb[pID]->JoinRelease();

    // Ask the parent process to allow the process to exit.
    pcb[pID]->ExitWait();

    // Remove the process from the process table.
    Remove(pID);

    return exitcode;
}
