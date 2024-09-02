#ifndef STABLE_H
#define STABLE_H

#include "synch.h"
#include "bitmap.h"

#define MAX_SEMAPHORE 10

/// @brief Sem class to manage semaphore with name and Semaphore object
class Sem
{
private:
    char name[50];
    Semaphore *sem;

public:
    Sem(char *name, int initValue)
    {
        strcpy(this->name, name);
        sem = new Semaphore(this->name, initValue);
    }

    ~Sem()
    {
        if (sem)
            delete sem;
    }

    void Wait()
    {
        sem->P();
    }

    void Signal()
    {
        sem->V();
    }

    char *GetName()
    {
        return this->name;
    }
};

/// @brief STable class to manage semaphore table
class STable
{
private:
    BitMap *bm;
    Sem *semTab[MAX_SEMAPHORE];

public: // Constructor & Destructor
    STable();
    ~STable();

public: // Methods
    int Create(char *name, int init);
    int Wait(char *name);
    int Signal(char *name);

    int FindFreeSlot();
    Sem *GetSemaphore(char *name);
};

#endif