#include "syscall.h"

#define MAX_SIZE (10)

int main()
{
    int a[MAX_SIZE]; // Array to be sorted
    int n;           // Number of elements in the array
    int order;       // Order of sorting

    /* Quicksort */

    int stack[MAX_SIZE]; // Stack for storing left and right indices of sub arrays
    int top;             // Top of the stack
    int left;            // Left index of the sub array to be sorted
    int right;           // Right index of the sub array to be sorted
    int pivotIndex;      // Index of the pivot element
    int pivot;           // Pivot element
    int i, j;            // Loop variables
    int temp;            // Temporary variable for swapping

    /* File handling */
    int openFileId;                   // File descriptor for the file to be written
    char *filename = "quicksort.txt"; // Name of the file to be written

    top = -1;
    left = 0;

    PrintString("*======================================================*\n");
    PrintString("| Welcome to the Quick Sort Integer program in Nachos! |\n");
    PrintString("*======================================================*\n");
    PrintChar('\n');

    // Read n, the number of elements in the array
    do
    {
        PrintString("Enter number of elements in the array: ");
        n = ReadInt();
        if (n <= 0 || n > MAX_SIZE)
            PrintString("n must be an integer between 1 and 100 (inclusive), please try again\n");
    } while (n <= 0 || n > MAX_SIZE);

    right = n - 1;
    i = 0;

    // Read the elements of the array
    for (i; i < n; i++)
    {
        PrintString("+ Enter element [");
        PrintInt(i);
        PrintString("]: ");
        a[i] = ReadInt();
    }

    // Print the array to console
    PrintString("Initial array: [");
    for (i = 0; i < n - 1; i++)
    {
        PrintInt(a[i]);
        PrintString(", ");
    }
    if (n > 0)
        PrintInt(a[n - 1]);
    PrintString("]\n");

    // Read the order of sorting
    do
    {
        PrintString("Enter type of sorting (1: increasing, 2: decreasing): ");
        order = ReadInt();
        if (order != 1 && order != 2)
            PrintString("Wrong input, please try again\n");
    } while (order != 1 && order != 2);

    // Quick sort
    stack[++top] = left;
    stack[++top] = right;

    while (top >= 0)
    {
        right = stack[top--];
        left = stack[top--];

        pivotIndex = left + (right - left) / 2;
        pivot = a[pivotIndex];

        i = left, j = right;

        while (i <= j)
        {
            while (a[i] < pivot)
                i++;
            while (a[j] > pivot)
                j--;

            if (i <= j)
            {
                temp = a[i];
                a[i] = a[j];
                a[j] = temp;
                i++;
                j--;
            }
        }

        if (left < j)
        {
            stack[++top] = left;
            stack[++top] = j;
        }

        if (i < right)
        {
            stack[++top] = i;
            stack[++top] = right;
        }
    }

    // Reverse the array if order is decreasing
    if (order == 2)
    {
        for (i = 0; i < n / 2; i++)
        {
            temp = a[i];
            a[i] = a[n - i - 1];
            a[n - i - 1] = temp;
        }
    }

    // Print the sorted array to console
    PrintString("Sorted array:  [");
    for (i = 0; i < n - 1; i++)
    {
        PrintInt(a[i]);
        PrintChar(',');
        PrintChar(' ');
    }
    if (n > 0)
        PrintInt(a[n - 1]);
    PrintString("]\n");

    // Open the file to write the sorted array
    openFileId = Open(filename, 0);

    while (openFileId == -1)
    {
        PrintString("Can't open file ");
        PrintString(filename);
        PrintChar('\n');
        if (CreateFile(filename) == -1)
        {
            PrintString("Can't create file quicksort.txt\n");
            return 1;
        }
        else
        {
            openFileId = Open(filename, 0);
            PrintString("Create file quicksort.txt to write sorted array successfully\n");
        }
    }

    // Write the sorted array to the file
    for (i = 0; i < n; i++)
    {
        Write(&a[i], 4, openFileId);
    }

    Close(openFileId);

    PrintString("\nWrite sorted array to file quicksort.txt successfully\n\n");

    PrintString("*========================================================*\n");
    PrintString("|   Thank you for using the Quick Sort Integer program!  |\n");
    PrintString("*========================================================*\n");
    PrintChar('\n');

    return 0;
}

#undef MAX_SIZE