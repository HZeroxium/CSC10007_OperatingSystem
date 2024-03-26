# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as

GCCDIR = ../../../gnu-decstation-ultrix/decstation-ultrix/2.95.3/

LDFLAGS = -T script -N
ASFLAGS = -mips2
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

PATH = $(GCCDIR):/lib:/usr/bin:/bin

CC = $(GCCDIR)gcc -B../../../gnu-decstation-ultrix/
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = gcc -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

# ---------------------------------------------------------------------------------------
all: halt shell matmult sort create_file char_io_test string_io_test int_io_test float_io_test
# ---------------------------------------------------------------------------------------
start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.c > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s
# ---------------------------------------------------------------------------------------
halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
# ---------------------------------------------------------------------------------------
shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell
# ---------------------------------------------------------------------------------------
sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort
# ---------------------------------------------------------------------------------------
matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult
# ---------------------------------------------------------------------------------------
create_file.o: create_file.c
	$(CC) $(CFLAGS) -c create_file.c
create_file: create_file.o start.o
	$(LD) $(LDFLAGS) start.o create_file.o -o create_file.coff
	../bin/coff2noff create_file.coff create_file
# ---------------------------------------------------------------------------------------
char_io_test.o: char_io_test.c
	$(CC) $(CFLAGS) -c char_io_test.c
char_io_test: char_io_test.o start.o
	$(LD) $(LDFLAGS) start.o char_io_test.o -o char_io_test.coff
	../bin/coff2noff char_io_test.coff char_io_test
# ---------------------------------------------------------------------------------------
string_io_test.o: string_io_test.c
	$(CC) $(CFLAGS) -c string_io_test.c
string_io_test: string_io_test.o start.o
	$(LD) $(LDFLAGS) start.o string_io_test.o -o string_io_test.coff
	../bin/coff2noff string_io_test.coff string_io_test
# ---------------------------------------------------------------------------------------
int_io_test.o: int_io_test.c
	$(CC) $(CFLAGS) -c int_io_test.c
int_io_test: int_io_test.o start.o
	$(LD) $(LDFLAGS) start.o int_io_test.o -o int_io_test.coff
	../bin/coff2noff int_io_test.coff int_io_test
# ---------------------------------------------------------------------------------------
float_io_test.o: float_io_test.c
	$(CC) $(CFLAGS) -c float_io_test.c
float_io_test: float_io_test.o start.o
	$(LD) $(LDFLAGS) start.o float_io_test.o -o float_io_test.coff
	../bin/coff2noff float_io_test.coff float_io_test

