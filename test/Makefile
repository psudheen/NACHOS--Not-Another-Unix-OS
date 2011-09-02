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

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt cust1 OrdT1 waitr1 cok1 mg1 n1 n2 n3 n4 cust2 OrdT2 waitr2 cok2 mg2 waiter cok mgwt cust OrdT n0 cok01

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
	
cok01.o: cok01.c
	$(CC) $(CFLAGS) -c cok01.c
cok01: cok01.o start.o
	$(LD) $(LDFLAGS) start.o cok01.o -o cok01.coff
	../bin/coff2noff cok01.coff cok01

mg1.o: mg1.c
	$(CC) $(CFLAGS) -c mg1.c
mg1: mg1.o start.o
	$(LD) $(LDFLAGS) start.o mg1.o -o mg1.coff
	../bin/coff2noff mg1.coff mg1

mg2.o: mg2.c
	$(CC) $(CFLAGS) -c mg2.c
mg2: mg2.o start.o
	$(LD) $(LDFLAGS) start.o mg2.o -o mg2.coff
	../bin/coff2noff mg2.coff mg2

mgwt.o: mgwt.c
	$(CC) $(CFLAGS) -c mgwt.c
mgwt: mgwt.o start.o
	$(LD) $(LDFLAGS) start.o mgwt.o -o mgwt.coff
	../bin/coff2noff mgwt.coff mgwt

n0.o: n0.c
	$(CC) $(CFLAGS) -c n0.c
n0: n0.o start.o
	$(LD) $(LDFLAGS) start.o n0.o -o n0.coff
	../bin/coff2noff n0.coff n0

n1.o: n1.c
	$(CC) $(CFLAGS) -c n1.c
n1: n1.o start.o
	$(LD) $(LDFLAGS) start.o n1.o -o n1.coff
	../bin/coff2noff n1.coff n1
	
n2.o: n2.c
	$(CC) $(CFLAGS) -c n2.c
n2: n2.o start.o
	$(LD) $(LDFLAGS) start.o n2.o -o n2.coff
	../bin/coff2noff n2.coff n2
n3.o: n3.c
	$(CC) $(CFLAGS) -c n3.c
n3: n3.o start.o
	$(LD) $(LDFLAGS) start.o n3.o -o n3.coff
	../bin/coff2noff n3.coff n3
n4.o: n4.c
	$(CC) $(CFLAGS) -c n4.c
n4: n4.o start.o
	$(LD) $(LDFLAGS) start.o n4.o -o n4.coff
	../bin/coff2noff n4.coff n4	

	
cok1.o: cok1.c
	$(CC) $(CFLAGS) -c cok1.c
cok1: cok1.o start.o
	$(LD) $(LDFLAGS) start.o cok1.o -o cok1.coff
	../bin/coff2noff cok1.coff cok1


cust1.o: cust1.c
	$(CC) $(CFLAGS) -c cust1.c
cust1: cust1.o start.o
	$(LD) $(LDFLAGS) start.o cust1.o -o cust1.coff
	../bin/coff2noff cust1.coff cust1	

cust.o: cust.c
	$(CC) $(CFLAGS) -c cust.c
cust: cust.o start.o
	$(LD) $(LDFLAGS) start.o cust.o -o cust.coff
	../bin/coff2noff cust.coff cust	
	
OrdT1.o: OrdT1.c
	$(CC) $(CFLAGS) -c OrdT1.c
OrdT1: OrdT1.o start.o
	$(LD) $(LDFLAGS) start.o OrdT1.o -o OrdT1.coff
	../bin/coff2noff OrdT1.coff OrdT1

OrdT.o: OrdT.c
	$(CC) $(CFLAGS) -c OrdT.c
OrdT: OrdT.o start.o
	$(LD) $(LDFLAGS) start.o OrdT.o -o OrdT.coff
	../bin/coff2noff OrdT.coff OrdT

waitr1.o: waitr1.c
	$(CC) $(CFLAGS) -c waitr1.c
waitr1: waitr1.o start.o
	$(LD) $(LDFLAGS) start.o waitr1.o -o waitr1.coff
	../bin/coff2noff waitr1.coff waitr1

waiter.o: waiter.c
	$(CC) $(CFLAGS) -c waiter.c
waiter: waiter.o start.o
	$(LD) $(LDFLAGS) start.o waiter.o -o waiter.coff
	../bin/coff2noff waiter.coff waiter
	
cok2.o: cok2.c
	$(CC) $(CFLAGS) -c cok2.c
cok2: cok2.o start.o
	$(LD) $(LDFLAGS) start.o cok2.o -o cok2.coff
	../bin/coff2noff cok2.coff cok2

cok.o: cok.c
	$(CC) $(CFLAGS) -c cok.c
cok: cok.o start.o
	$(LD) $(LDFLAGS) start.o cok.o -o cok.coff
	../bin/coff2noff cok.coff cok

cust2.o: cust2.c
	$(CC) $(CFLAGS) -c cust2.c
cust2: cust2.o start.o
	$(LD) $(LDFLAGS) start.o cust2.o -o cust2.coff
	../bin/coff2noff cust2.coff cust2	

	
OrdT2.o: OrdT2.c
	$(CC) $(CFLAGS) -c OrdT2.c
OrdT2: OrdT2.o start.o
	$(LD) $(LDFLAGS) start.o OrdT2.o -o OrdT2.coff
	../bin/coff2noff OrdT2.coff OrdT2
	

waitr2.o: waitr2.c
	$(CC) $(CFLAGS) -c waitr2.c
waitr2: waitr2.o start.o
	$(LD) $(LDFLAGS) start.o waitr2.o -o waitr2.coff
	../bin/coff2noff waitr2.coff waitr2
	
clean:
	rm -f *.o *.coff
