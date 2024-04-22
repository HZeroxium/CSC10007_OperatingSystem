#include "syscall.h"

int main()
{
    int success;                                          // Check if the operation is successful
    SpaceId si_input, si_output, si_passenger, si_result; // File id
    int count;                                            // Number of scan times
    char c_readFile;                                      // Character to read from the file

    PrintString("Security scanning system starting...\n");
    // Initialize 4 Semaphores to manage 3 threads
    success = CreateSemaphore("main", 0);
    if (success == -1)
        return 1;
    success = CreateSemaphore("passenger", 0);
    if (success == -1)
        return 1;
    success = CreateSemaphore("scan", 0);
    if (success == -1)
        return 1;
    success = CreateSemaphore("m_s", 0);
    if (success == -1)
        return 1;

    // Create output.txt file to write the final result
    success = CreateFile("output.txt");
    if (success == -1)
    {
        PrintString("Can't create output.txt\n");
        return 1;
    }

    // Open input.txt file to read
    si_input = Open("input.txt", 1);
    if (si_input == -1)
    {
        PrintString("Can't open input.txt\n");
        return 1;
    }

    // Open output.txt file to read and write
    si_output = Open("output.txt", 0);
    if (si_output == -1)
    {
        Close(si_input);
        PrintString("Can't open output.txt\n");
        return 1;
    }

    // Read the number of scan times in the input.txt file
    count = 0;
    while (1)
    {
        Read(&c_readFile, 1, si_input);
        if (c_readFile != '\n')
        {
            if (c_readFile >= '0' && c_readFile <= '9')
                count = count * 10 + (c_readFile - 48);
        }
        else
        {
            break;
        }
    }

    // Execute the passenger.c thread
    success = Exec("./test/passenger");
    if (success == -1)
    {
        Close(si_input);
        Close(si_output);
        PrintString("Can't execute passenger\n");
        return 1;
    }

    // Execute the scan.c thread
    success = Exec("./test/scan");
    if (success == -1)
    {
        Close(si_input);
        Close(si_output);
        PrintString("Can't execute scan\n");
        return 1;
    }

    while (count--)
    {
        // Create passenger.txt file to write the passenger list
        success = CreateFile("passenger.txt");
        if (success == -1)
        {
            Close(si_input);
            Close(si_output);
            PrintString("Can't create passenger.txt\n");
            return 1;
        }

        // Open passenger.txt file to write the passenger list
        si_passenger = Open("passenger.txt", 0);
        if (si_passenger == -1)
        {
            Close(si_input);
            Close(si_output);
            PrintString("Can't open passenger.txt\n");
            return 1;
        }

        while (1)
        {
            if (Read(&c_readFile, 1, si_input) < 1)
            {
                // Read to the end of the file
                break;
            }
            if (c_readFile != '\n')
            {
                Write(&c_readFile, 1, si_passenger);
            }
            else
            {
                break;
            }
        }

        // Close the passenger.txt file
        Close(si_passenger);

        // Up the passenger.c thread to start
        Up("passenger");

        // Main process waits for the passenger.c thread to finish
        Down("main");

        // Read the result from result.txt file and write to output.txt file
        si_result = Open("result.txt", 1);
        if (si_result == -1)
        {
            Close(si_input);
            Close(si_output);
            PrintString("Can't open result.txt\n");
            return 1;
        }

        si_passenger = Open("passenger.txt", 0);
        while (1)
        {
            Read(&c_readFile, 1, si_passenger);
            Write(&c_readFile, 1, si_output);
            Write(" ", 1, si_output);
            Read(&c_readFile, 1, si_passenger);
            if (Read(&c_readFile, 1, si_result) < 1)
            {
                Write("\r\n", 2, si_output);
                Close(si_result);
                Up("m_s");
                break;
            }
            Write(&c_readFile, 1, si_output);
            Write("\t", 1, si_output);
            Write("\t", 1, si_output);
        }
    }

    // Close the input.txt and output.txt files
    Close(si_input);
    Close(si_output);

    PrintString("Security scanning system finished\n");
    return 0;
}