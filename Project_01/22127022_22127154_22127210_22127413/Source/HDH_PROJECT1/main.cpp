#include <iostream>
#include "System.h"

int main()
{
    System OS;
    if (OS.OnCreate()) {
        OS.OnStart();
    }
    return 0;
}

