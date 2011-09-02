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
	exec=executable;

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

	DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
				numPages, size);
	// first, set up the translation 
	pageTableLock[currentThread->currentProcID]->Acquire();	
	myPageTable=new _TranslationEntry[NumPhysPages];
	for(int i = 0; i< numPages; i++)
	{
		myPageTable[i].virtualPage = i;
		myPageTable[i].valid = TRUE;
		myPageTable[i].use = FALSE;  //Hardware sets this whenever page is referenced or modified
		myPageTable[i].dirty = FALSE;
		myPageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
		myPageTable[i].physicalPage=-1;
		if(i < (unsigned int)(divRoundUp((noffH.code.size + noffH.initData.size),PageSize)))
		{	
			myPageTable[i].file_offset=noffH.code.inFileAddr+(i*PageSize);	
			myPageTable[i].location=0;			
		} 	
		else
		{
			myPageTable[i].location=2;
		}
	}
	
	
	pageTableLock[currentThread->currentProcID]->Release();   
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete myPageTable;
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
   int i;
	 machine->pageTableSize = numPages;
  IntStatus old=interrupt->SetLevel(IntOff);
	for(i=0;i<TLBSize;i++)
	{
		if(machine->tlb[i].valid==true)
		{
			ipt[machine->tlb[i].physicalPage].dirty=machine->tlb[i].dirty;
		}
	}
	for(i=0;i<TLBSize;i++)
	{
		//if(machine->tlb[i].valid && machine->tlb[i].dirty)
			machine->tlb[i].valid=false;
	}
	interrupt->SetLevel(old);
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
		//PhyMemBitMap->Clear(pageTable[i].physicalPage); 
		pageTableLock[currentThread->currentProcID]->Acquire(); 
		myPageTable[i].physicalPage=-1;
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
		//PhyMemBitMap->Clear(pageTable[myStackLocTop+i].physicalPage); 
		myPageTable[myStackLocTop+i].physicalPage=-1;
	}
	bitMapLock->Release();
 }

 
