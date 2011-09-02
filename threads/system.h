// system.h 
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
extern int ClientID;
extern int ServerID;
extern int netname;
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
extern int currentTLB;
extern int fifo_index;
extern Lock *offsetLock;
extern Lock *tlbLock;
extern Lock *SwapFileLock;
extern OpenFile *SwapFile;
extern BitMap *SwapBitMap;
extern Lock* swaplocLock;
extern Lock* locLock;
extern List *fifo;
extern Lock* fifoLock;
extern int dummyfifo[1000];
extern int dummyfifoCnt;
extern int rplacementPol;
// class _TranslationEntry {
  // public:
    // int virtualPage;    // The page number in virtual memory.
    // int physicalPage;   // The page number in real memory (relative to the
                        // //  start of "mainMemory"
    // bool valid;         // If this bit is set, the translation is ignored.
                        // // (In other words, the entry hasn't been initialized.)
    // bool readOnly;      // If this bit is set, the user program is not allowed
                        // // to modify the contents of the page.
    // bool use;           // This bit is set by the hardware every time the
                        // // page is referenced or modified.
    // bool dirty;         // This bit is set by the hardware every time the
                        // // page is modified.
		// int location;       // 0 - In exectable,1 - In swap file and  2 - niether
    // int ProcID;         // Process ID for finding VPN
    // int SwapLocation;   // To track where to swap the file
		// int file_offset;
// };

extern _TranslationEntry *ipt;
extern Lock *iptlock;

#endif

#endif // SYSTEM_H
