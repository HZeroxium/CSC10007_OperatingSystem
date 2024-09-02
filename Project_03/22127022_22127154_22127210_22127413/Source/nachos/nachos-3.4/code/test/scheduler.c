#include "syscall.h"

void main()
{
    int pingPID, pongPID; // Process ID
    int success;          // Check if the operation is successful

    // Initialize 2 Semaphores to manage 2 threads
    success = CreateSemaphore("ping", 5);
    if (success == -1)
        return;

    success = CreateSemaphore("pong", 5);
    if (success == -1)
        return;

    PrintString("Ping-Pong Program\n\n");

    // Create ping and pong threads
    pingPID = Exec("./test/ping");
    pongPID = Exec("./test/pong");

    // Wait for the threads to finish
    Join(pingPID);
    Join(pongPID);
}