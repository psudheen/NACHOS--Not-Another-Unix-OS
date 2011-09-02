// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"
//#include "synch.h"

#define UserStackSize		1024 	// increase this as necessary! (128*8)

#define MaxOpenFiles 256
#define MaxChildSpaces 256
#define MAXFILENAME 256
#define MAX_POSSIBLE_LOCKS      1024
#define MAX_POSSIBLE_CONDITIONS MAX_POSSIBLE_LOCKS
#define MAX_POSSIBLE_THREADS 128

extern int LockUsuageCntr[MAX_POSSIBLE_LOCKS];
extern int CVUsuageCntr[MAX_POSSIBLE_LOCKS];
class _TranslationEntry {
  public:
    int virtualPage;    // The page number in virtual memory.
    int physicalPage;   // The page number in real memory (relative to the
                        //  start of "mainMemory"
    bool valid;         // If this bit is set, the translation is ignored.
                        // (In other words, the entry hasn't been initialized.)
    bool readOnly;      // If this bit is set, the user program is not allowed
                        // to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
                        // page is referenced or modified.
    bool dirty;         // This bit is set by the hardware every time the
                        // page is modified.
		int location;       // 0 - In exectable,1 - In swap file and  2 - niether
    int ProcID;         // Process ID for finding VPN
    int SwapLocation;   // To track where to swap the file
		int file_offset;
};

class AddrSpace {
 private:
    	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
		
					// address space
  public:
	OpenFile *exec;
	_TranslationEntry *myPageTable;
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;			// Table of openfiles
		Table lockTable;            // Table of User program locks
    Table conditionTable;       // Table of User program Conditon variables
		_TranslationEntry* GetpageTable()
    {
      return myPageTable;
    }
	 void SetpageTable(_TranslationEntry *_myPageTable)
    {
       myPageTable=_myPageTable;
    } 
    int GetnumPages()
    {
       return  numPages;
    }
    void SetnumPages(int mynumPages)
     {
        numPages=mynumPages;
     }
	void DelPageTable()
	{
	   delete myPageTable;
	}
	void DestroyAddrSpaceBitMap(); //Function to destroy/clear BitMap used by this addrspace
	void DestroyStackBitMap(unsigned int thisStackLoc);//Function to destroy/clear BitMap used by this addrspace
};


#endif // ADDRSPACE_H
