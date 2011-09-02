
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
    
#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
//#include "addrspace.h"
#ifdef USER_PROGRAM
#include "addrspace.h"
#endif

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
extern Lock *mailBoxLock;
extern int nextMailBoxNo;
extern int netname;
extern void KernelFn();
extern void KernelFn2();
extern List *MsgQ;
extern bool lostPacket;
extern Lock *msgQLock;
extern int myClientID;
extern int extracredit;
#endif

//#include "../userprog/addrspace.h"
#include "synch.h"
#ifdef USER_PROGRAM
typedef struct procTbl_t
{
  AddrSpace *myProcAddrSpace;   
  int TotalThreads;
  int AliveThreadCount;
  int PID;
  int StackLoc[2000];
};

extern procTbl_t processTable[2000];
extern int TotalProcessCount; //Total count all the process that are created in the OS - Global across OS
extern Lock *processTableLock;
extern BitMap    *PhyMemBitMap;
extern Lock *pageTableLock[MAX_POSSIBLE_LOCKS];
extern Lock *bitMapLock;
extern int ActiveNoOfProcess;
#endif

#endif // SYSTEM_H