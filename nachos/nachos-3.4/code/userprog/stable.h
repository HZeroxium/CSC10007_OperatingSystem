#ifndef STABLE_H
#define STABLE_H

#include "synch.h"
#include "bitmap.h"

#define MAX_SEMAPHORE 10

class STable
{
private:
    BitMap *bm;
    Semaphore *semTab[MAX_SEMAPHORE];

public: // Constructor & Destructor
    STable();
    ~STable();

public: // Methods
    int Create(char *name, int init);
    Semaphore *Get(char *name);
    int FindFreeSlot();

    int Up(char *name);
    int Down(char *name);
};

#endif