#include "syscall.h"

int main()
{

    int i;
    for (i = 0; i < 1000; i++)
    {
        Down("pong");
        PrintChar('A');
        Up("ping");
    }
}