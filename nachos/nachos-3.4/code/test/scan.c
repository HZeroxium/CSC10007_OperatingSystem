#include "syscall.h"

int main()
{
    int success;                // Check if the operation is successful
    SpaceId si_scan, si_result; // File id
    char c_readFile;            // Character to read from the file
    int scan1, scan2, scan3;    // Scanners
    int scan;                   // Number of passengers scanned
    int flag_done_result;       // Flag to indicate that the result file has been read

    scan1 = scan2 = scan3 = 0;

    while (1)
    {
        Down("m_s");

        // Open result.txt file to write the number of passengers scanned
        si_result = Open("result.txt", 0);
        if (si_result == -1)
        {
            Up("passenger");
            return;
        }

        while (1)
        {
            Down("scan");
            c_readFile = 0;
            // Open scan.txt file to read the number of passengers scanned
            si_scan = Open("scan.txt", 1);
            if (si_scan == -1)
            {
                Close(si_result);
                Up("passenger");
                return;
            }

            scan = 0;
            flag_done_result = 0;
            while (1)
            {
                if (Read(&c_readFile, 1, si_scan) == -2)
                {
                    Close(si_scan);
                    break;
                }
                if (c_readFile != '*')
                {
                    scan = scan * 10 + (c_readFile - 48);
                }
                else
                {
                    flag_done_result = 1;
                    Close(si_scan);
                    break;
                }
            }

            // Write the index of the scanner to the result.txt file
            if (scan != 0)
            {
                if (scan1 <= scan2 && scan1 <= scan3)
                {
                    scan1 += scan;
                    Write("1", 1, si_result);
                }
                else if (scan2 <= scan1 && scan2 <= scan3)
                {
                    scan2 += scan;
                    Write("2", 1, si_result);
                }
                else
                {
                    scan3 += scan;
                    Write("3", 1, si_result);
                }
            }

            if (flag_done_result == 1)
            {
                scan1 = scan2 = scan3 = 0;
                Close(si_result);
                Up("passenger");
                break;
            }

            Up("passenger");
        }
    }
}