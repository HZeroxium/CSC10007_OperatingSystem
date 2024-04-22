#include "syscall.h"

int main()
{
    int success;                   // Check if the operation is successful
    SpaceId si_passenger, si_scan; // File id
    char c_readFile;               // Character to read from the file
    int flag_scan;                 // Flag to jump to the scan thread
    int flag_main;                 // Flag to jump to the main thread
    int lengthFile;                // File length
    int i_File;                    // File pointer

    Up("m_s");

    while (1)
    {
        lengthFile = 0;

        Down("passenger");

        // Create result.txt file to write the number of passengers scanned
        success = CreateFile("result.txt");
        if (success == -1)
        {
            Up("main"); // return to the main thread
            return;
        }

        // Open passenger.txt file to read
        si_passenger = Open("passenger.txt", 1);
        if (si_passenger == -1)
        {
            Up("main"); // return to the main thread
            return;
        }

        lengthFile = Seek(-1, si_passenger);
        Seek(0, si_passenger);
        i_File = 0;

        // Create scan.txt file
        success = CreateFile("scan.txt");
        if (success == -1)
        {
            Close(si_passenger);
            Up("main"); // return to the main thread
            return;
        }

        // Open scan.txt file to write the number of passengers scanned
        si_scan = Open("scan.txt", 0);
        if (si_scan == -1)
        {
            Close(si_passenger);
            Up("main"); // return to the main thread
            return;
        }

        // Read the number of passengers in the passenger.txt file
        while (i_File < lengthFile)
        {
            flag_scan = 0;
            Read(&c_readFile, 1, si_passenger);
            if (c_readFile != ' ')
            {
                Write(&c_readFile, 1, si_scan);
            }
            else
            {
                flag_scan = 1;
            }
            if (i_File == lengthFile - 1)
            {
                Write("*", 1, si_scan);
                flag_scan = 1;
            }
            if (flag_scan == 1)
            {
                Close(si_scan);
                Up("scan");
                Down("passenger");

                success = CreateFile("scan.txt");
                if (success == -1)
                {
                    Close(si_passenger);
                    Up("main"); // return to the main thread
                    return;
                }

                si_scan = Open("scan.txt", 0);
                if (si_scan == -1)
                {
                    Close(si_passenger);
                    Up("main"); // return to the main thread
                    return;
                }
            }
            i_File++;
        }
        Up("main");
    }
}