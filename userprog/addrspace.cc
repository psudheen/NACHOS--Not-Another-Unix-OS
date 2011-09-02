// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
//#include "synch.h"
#ifndef MACHINE_H
  #include "machine.h"
#endif
#include "bitmap.h"

//// Global variables -Start

Lock *mainMemLock = new Lock("mainMemLock");
int LockUsuageCntr[MAX_POSSIBLE_LOCKS] = {0};
int CVUsuageCntr[MAX_POSSIBLE_LOCKS] = {0};
//Global variables - end


extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles),lockTable(MAX_POSSIBLE_LOCKS),conditionTable(MAX_POSSIBLE_CONDITIONS)
{
    NoffHeader noffH;
    unsigned int i, size;
	//intialize global variables
	//Update the Process Table - Just update the PID 
	//Increment the PID
	// processTableLock->Acquire();
	// TotalProcessCount++;
	// processTable[TotalProcessCount].PID = TotalProcessCount;
	// processTable[TotalProcessCount].AliveThreadCount =0;
	// processTable[TotalProcessCount].TotalThreads =0;
	// processTable[TotalProcessCount].myProcAddrSpace = NULL;
	// for(int i<0;i<2000;i++)
	// processTable[TotalProcessCount].StackLoc[2000]=NULL;
	// processTableLock->Release(); 

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);
                                                // we need to increase the size
						// to leave room for the stack
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTableLock[currentThread->currentProcID]->Acquire();
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++)
		{
			pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
			bitMapLock->Acquire();
			int pp = PhyMemBitMap->Find();	//Find() will find the free bit and also sets the locn it found
			//printf("AddrSpace bitmap%d\n",pp);
			if(pp!=-1)
				pageTable[i].physicalPage = pp; 
			else
				printf("No more main memory left\n");
			bitMapLock->Release();
			pageTable[i].valid = TRUE;
			pageTable[i].use = FALSE;  //Hardware sets this whenever page is referenced or modified
			pageTable[i].dirty = FALSE;
			pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
																			// a separate page, we could set its 
																			// pages to be read-only
			bzero(&(machine->mainMemory[(pageTable[i].physicalPage)*(PageSize)]), PageSize);
			if(i < (unsigned int)(divRoundUp((noffH.code.size + noffH.initData.size),PageSize)))
			{	
				//put one page at a time
				executable->ReadAt(&(machine->mainMemory[(pageTable[i].physicalPage)*(PageSize)]), PageSize, noffH.code.inFileAddr + (i * PageSize));
			}	
    }
    pageTableLock[currentThread->currentProcID]->Release();
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
   // bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory

   // UserProgReadAt(executable,noffH.code.inFileAddr,size, numPages);
    
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}



//----------------------------------------------------------------------
// UserProgReadAt
// 	Function to copy bytes from virtual memory to physical memory
// 
//
//      
//----------------------------------------------------------------------
ExceptionType UserProgReadAt(OpenFile *exec, int vAdd, int size, int NoOfPagesToCopy)
{ 
	//return PageFaultException;//get rid of compiler warning
  // int VirPageNo =-1;
  // int PhyPageNo = -1;
  // int PhyAddr =-1; 
  
  // for (int i =0; i< NoOfPagesToCopy; i++)
	// {
	  // VirPageNo = vAdd/PageSize;
	  // if (VirPageNo >= pageTableSize)
		// {
		  // DEBUG('a', "virtual page # %d too large for page table size %d!\n",virtAddr, pageTableSize);
	      // return AddressErrorException;
		// }
	  // pageTableLock->Acquire(); 
	  // PhyPageNo = pageTable[VirPageNo].physicalPage;
	  // pageTableLock->Release();
	  // // if the pageFrame is too big, there is something really wrong! 
      // // An invalid translation was loaded into the page table or TLB. 
      // if (PhyPageNo >= NumPhysPages)
       // { 
		 // DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
		 // return BusErrorException;
    	// }
	  // PhyAddr = PhyPageNo* PageSize;
	  // mainMemLock->Acquire();
	  // if(exec->ReadAt(&(machine->mainMemory[PhyAddr]),PageSize,(vAdd+(i*PageSize))!=0)
	  // {
	    // mainMemLock->Release();
	  // }
	  // else
		// {
		  // mainMemLock->Release();
	      // return PageFaultException; 
		// }
	// }
}

//----------------------------------------------------------------------
// DestroyAddrSpaceBitMap
// 	Function to destroy/clear BitMap used by this addrspace
// ---------------------------------------------------------------------

void AddrSpace::DestroyAddrSpaceBitMap()
{
	//printf("OMg, I'm called\n");
	for (unsigned int i=0;i<numPages;i++) 
	{
    bitMapLock->Acquire();
		if(numPages>=NumPhysPages)	
		{
			bitMapLock->Release();
			return;
		}		
		PhyMemBitMap->Clear(pageTable[i].physicalPage); 
		pageTableLock[currentThread->currentProcID]->Acquire();
		pageTable[i].physicalPage=-1;
		pageTableLock[currentThread->currentProcID]->Release();
		bitMapLock->Release();
	}
		
}

//----------------------------------------------------------------------
// DestroyStackBitMap
// 	Function to destroy/clear BitMap used by this addrspace
// ---------------------------------------------------------------------
void AddrSpace::DestroyStackBitMap(unsigned int thisStackLoc)
{
	if(thisStackLoc<=0)
	{
		//printf("Error:Invalid stack address encountered\n");
		return;
	}
	//printf("OMg, I'm called1\n");
  unsigned int myStackLocTop=0;
	myStackLocTop=(thisStackLoc/PageSize);
	if((thisStackLoc%PageSize)!=0)
	{
		myStackLocTop++;
	}
	if(myStackLocTop<8)
	{
		//printf("Error:Invalid stack address encountered\n");
		return;
	}
	myStackLocTop=myStackLocTop-8;
	if(myStackLocTop<0)
	{
		//printf("Error:Invalid stack address encountered\n");
		return;
	}
	if((myStackLocTop%PageSize)>=NumPhysPages)
	{
		//printf("Error:Invalid stack address encountered\n");
		return;
	}
	//in each 8 pages of the stack, clear BitMap
	//printf("myStackLocTop%u\n",myStackLocTop);
	bitMapLock->Acquire();
	for (int i=0; i<8; i++) 
	{		
		PhyMemBitMap->Clear(pageTable[myStackLocTop+i].physicalPage); 
		pageTable[myStackLocTop+i].physicalPage=-1;
	}
	bitMapLock->Release();
 }


