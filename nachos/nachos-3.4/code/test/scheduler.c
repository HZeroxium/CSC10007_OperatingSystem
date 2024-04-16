#include "syscall.h"

void main()
{
    int pingPID, pongPID;
    int f_Success;
    f_Success = CreateSemaphore("ping", 5);
    if (f_Success == -1)
        return;

    f_Success = CreateSemaphore("pong", 0);
    if (f_Success == -1)
        return;

    PrintString("Ping-Pong test starting...\n\n");
    pingPID = Exec("./test/ping");
    pongPID = Exec("./test/pong");
    Join(pingPID);
    Join(pongPID);
}