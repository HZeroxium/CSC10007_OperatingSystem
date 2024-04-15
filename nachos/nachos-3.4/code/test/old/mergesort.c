#include "syscall.h"

#define MAX_SIZE (20)

int main()
{
    float *a[MAX_SIZE];    // array of float pointers
    float *b[MAX_SIZE];    // temporary array of float pointers for merging (left half)
    float *c[MAX_SIZE];    // temporary array of float pointers for merging (right half)
    int n;                 // number of elements in the array
    int i, j, k;           // loop variables
    int nb;                // number of elements in b (left half)
    int nc;                // number of elements in c
    int size;              // size of the sub array to be sorted
    int left;              // left index of the sub array to be sorted
    int mid;               // middle index of the sub array to be sorted
    int right;             // right index of the sub array to be sorted
    int id;                // index of the sub array to be sorted
    int ib;                // index of the element in b
    int ic;                // index of the element in c
    OpenFileId openFileId; // File descriptor for the file to be written
    char temp[8];          // Temporary buffer for writing float to file

    PrintString("*====================================================*\n");
    PrintString("| Welcome to the Merge Sort Float program in Nachos! |\n");
    PrintString("*====================================================*\n");

    // Read n, the number of elements in the array
    do
    {
        PrintString("Enter number of elements in the array (n, 1 <= n < 100): ");
        n = ReadInt();
        if (n <= 0 || n > 100)
            PrintString("n must be an integer between 1 and 100 (inclusive), please try again\n");
    } while (n <= 0 || n > 100);
    // Read the elements of the array

    // Allocate memory for the array of float pointers
    for (i = 0; i < n; i++)
    {
        PrintString("+ Enter float element [");
        PrintInt(i);
        PrintString("]: ");
        a[i] = ReadFloat();
    }

    // Print the initial array to console
    PrintString("\n-> Initial array: [");
    for (i = 0; i < n - 1; i++)
    {
        PrintFloat(a[i]);
        PrintString(", ");
    }
    if (n > 0)
        PrintFloat(a[n - 1]);
    PrintString("]\n");

    /* Merge sort */
    // for each size is doubled
    //     meaning we sort a pair of sub arrays of size 1, 2, 4, 8, 16, ...
    for (size = 1; size <= n; size = size * 2)
    {
        // for each pair of sub arrays of given size
        //     the next pair should starts after size * 2 elements
        for (id = 0; id < n; id += size * 2)
        {
            // we currently sort a[left..right) based on a[left..mid) and a[mid..right)
            left = id;
            mid = (n < id + size ? n : id + size);
            right = (n < id + size * 2 ? n : id + size * 2);

            // copy a[left..mid) into b[0..size/2)
            nb = 0;
            for (i = left; i < mid; ++i)
            {
                b[nb++] = a[i];
            }
            // copy a[mid..right) into c[0..size/2)
            nc = 0;
            for (i = mid; i < right; ++i)
            {
                c[nc++] = a[i];
            }

            // merge a[left..mid) and a[mid..right) into a[left..right)
            for (ib = 0, ic = 0; ib < nb && ic < nc;)
            {
                if (CompareFloat(b[ib], c[ic]) <= 0)
                {
                    a[left++] = b[ib++];
                }
                else
                {
                    a[left++] = c[ic++];
                }
            }
            while (ib < nb)
            {
                a[left++] = b[ib++];
            }
            while (ic < nc)
            {
                a[left++] = c[ic++];
            }
        }
    }

    // Print the sorted array to console
    PrintString("=> Sorted array : [");
    for (i = 0; i < n - 1; i++)
    {
        PrintFloat(a[i]);
        PrintString(", ");
    }
    if (n > 0)
        PrintFloat(a[n - 1]);
    PrintString("]\n\n");

    openFileId = Open("mergesort.txt", 0); // Open the file to write the sorted array

    while (openFileId == -1) // If file open failed
    {
        PrintString("Can't open file mergesort.txt\n");
        if (CreateFile("mergesort.txt") == -1) // Create "mergesort.txt" file
        {
            PrintString("Can't create file mergesort.txt\n"); // Print error message
            return 1;
        }
        openFileId = Open("mergesort.txt", 0);
    }

    // If file opened successfully
    if (openFileId != -1)
    {
        for (i = 0; i < n; i++)
        {
            FloatToString(a[i], temp);  // Convert float to string
            Write(temp, 8, openFileId); // Write the content to the file
            Write("\n", 1, openFileId); // Write the content to the file
        }
        Close(openFileId);
        PrintString("-> Write sorted array to file mergesort.txt successfully\n\n");
    }
    else
    {
        PrintString("Cannot open file mergesort.txt\n");
    }

    // Free the memory allocated for the array of float pointers
    for (i = 0; i < n; i++)
    {
        FreeFloat(a[i]);
    }

    PrintString("*=============================================*\n");
    PrintString("| Thank you for using this program in Nachos! |\n");
    PrintString("*=============================================*\n");

    PrintChar('\n');

    return 0;
}

#undef MAX_SIZE