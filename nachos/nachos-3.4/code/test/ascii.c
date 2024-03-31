#include "syscall.h"

int main()
{
    int openFileId; // File descriptor for the file to be written
    int result = 0; // Result of the write operation
    char *buffer = "Decimal,Char\
0,NULL\
1,SOH\
2,STX\
3,ETX\
4,EOT\
5,ENQ\
6,ACK\
7,BEL\
8,BS\
9,HT\
10,LF\
11,VT\
12,FF\
13,CR\
14,SO\
15,SI\
16,DLE\
17,DC1\
18,DC2\
19,DC3\
20,DC4\
21,NAK\
22,SYN\
23,ETB\
24,CAN\
25,EM\
26,SUB\
27,ESC\
28,FS\
29,GS\
30,RS\
31,US\
32,SPACE\
33,!\
34,\"\
35,#\
36,$\
37,%\
38,&\
39,'\
40,(\
41,)\
42,*\
43,+\
44,\"\
45,-\
46,.\
47,/\
48,0\
49,1\
50,2\
51,3\
52,4\
53,5\
54,6\
55,7\
56,8\
57,9\
58,:,\
59,;\
60,<\
61,=\
62,>\
63,?\
64,@\
65,A\
66,B\
67,C\
68,D\
69,E\
70,F\
71,G\
72,H\
73,I\
74,J\
75,K\
76,L\
77,M\
78,N\
79,O\
80,P\
81,Q\
82,R\
83,S\
84,T\
85,U\
86,V\
87,W\
88,X\
89,Y\
90,Z\
91,[\
92,\\\
93,]\
94,^\
95,_\
96,`\
97,a\
98,b\
99,c\
100,d\
101,e\
102,f\
103,g\
104,h\
105,i\
106,j\
107,k\
108,l\
109,m\
110,n\
111,o\
112,p\
113,q\
114,r\
115,s\
116,t\
117,u\
118,v\
119,w\
120,x\
121,y\
122,z\
123,{\
124,|\
125,}\
126,~\
127,DEL";           // Buffer to store the content to be written to the file

    PrintString("*====================================================*\n");
    PrintString("|   Welcome to the ASCII Table program in Nachos!    |\n");
    PrintString("*====================================================*\n");

    PrintChar('\n');

    openFileId = Open("ascii.csv", 0); // Open "ascii.txt" file in write mode
    if (openFileId != -1)              // If file opened successfully
    {
        PrintString(buffer);                     // Print the content to console output
        result = Write(buffer, 857, openFileId); // Write the content to the file
        if (result == -1)
        {
            PrintString("!!!ERROR: File is not found or id is out of range or the file is not opened in write mode (stdin)\n");
        }
        else if (result == -2)
        {
            PrintString("!!!ERROR: No space left on the disk\n");
        }
        else if (result == -3)
        {
            PrintString("!!!ERROR: Unknown error occurs\n");
        }
        else
        {
            PrintString("\n\nWrite content to file ascii.txt successfully\n"); // Print success message
        }
        Close(openFileId); // Close the file
    }
    else // If file open failed
    {
        PrintString("Can't open file\n"); // Print error message
    }

    PrintChar('\n');

    PrintString("*=============================================*\n");
    PrintString("| Thank you for using this program in Nachos! |\n");
    PrintString("*=============================================*\n");

    PrintChar('\n');

    return 0;
}