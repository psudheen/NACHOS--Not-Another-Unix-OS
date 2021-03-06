


// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "addrspace.h"

extern "C" { int bzero(char *, int); };

#define MAXADDR 1048576
#define MAXLOCKS 256
#define MAXCVS 256
#define MAXMV 256
using namespace std;
Lock *TblUpdateLock =new Lock("TblUpdateLock");
void HandlePageFault(int vAdd);
int getIPTindex(int vpn);
void updateTLB(int ppn);
int loadPageIntoIPT(int vpn);
int evictApage(int vpn);
int evictApage1(int vpn);
int fifofn();



int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}



int CreateLock_Syscall(unsigned int vaddr, int len)
{
    // Create a lock  with the name in the user buffer pointed to by
    // vaddr.  The lock name is at most MAXFILENAME chars long.  If
    // the lock is created successfully, it is put in the address
    // space's lock table and an id (lock id) returned that can find the lock
    // later.  If there are any errors, -1 is returned.

	if((len<=0)||(len>MAXFILENAME))
	{
		printf("Invalid size for Lock name\n");
		return -1;
	}
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    Lock *l;			// The new lock object
    int id;				// The lockid

	if((vaddr<0)||(vaddr>MAXADDR))
	 {
	 	printf("Invalid Virtual Address\n");
		return -1;
	 }

	if((vaddr+len)>MAXADDR)
	{
		 printf("%s","ERROR: Address length exceeded the total size\n");
		 return -1;
	}
  if (!buf)
	{
		printf("%s","Can't allocate kernel buffer in CreateLock\n");
		return -1;
  }

	if( copyin(vaddr,len,buf) == -1 )
	{
		printf("%s","Bad pointer passed to CreateLock\n");
		delete[] buf;
		return -1;
	}
	#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock Name should come from User Program
		// request_type;lock name; client ID, threadID=0
		sprintf(data,"%d;%s;%d;%d",SC_CreateLock,buf,netname,0);
		printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		delete[] buf;
		return atoi(buffer);
	#else
    buf[len]='\0';
		printf("Lock Name:%s\n",buf);
	//printf("Inside Cl\n");

	pageTableLock[TotalProcessCount]->Acquire();
    l = new Lock("CreateLock");
	delete[] buf;

    if ( l )
    {	  
	    if ((id = currentThread->space->lockTable.Put(l)) == -1 )
		{
			delete l;
			//no slots in the lock table
			printf("unable to create Lock \n");
		}
		else
		{
		    LockUsuageCntr[id] = 0;
		    pageTableLock[TotalProcessCount]->Release();
			//printf("Lock created\n");
			return id;
		}
    }
    pageTableLock[TotalProcessCount]->Release();
    return -1;
	#endif
}
   
void DestroyLock_Syscall(int LockPos)
{

	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}
		#ifdef NETWORK
		printf("Inside destry lock.............\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock Name should come from User Program
		// request_type;lock id; client ID, threadID=0
		sprintf(data,"%d;%d;%d;%d",SC_DestroyLock,LockPos,netname,0); 
		printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		//delete[] buf;
		return ;
	#else
    // Destroy the lock associated with id LockPos.  WITH error reporting.
    pageTableLock[TotalProcessCount]->Acquire();
    Lock *l = (Lock*)currentThread->space->lockTable.Get(LockPos);
	if(LockUsuageCntr[LockPos]==0)
	{
		if(l){
		  delete l;
		  currentThread->space->lockTable.Remove(LockPos);
		  //printf("Lock Destroyed\n");
		  }
		 else
			printf("ERROR:Destroy lock failed. Lock Index = %d!\n",LockPos);
	}
    else
      printf("%s","ERROR:Lock still in use,Destroy lock failed!\n");
    
    pageTableLock[TotalProcessCount]->Release();
		#endif
}


void AcquireLock_Syscall(int LockPos)
{
		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock ID should come from User Program
		// request_type;lock ID; client ID, threadID=0
		sprintf(data,"%d;%d;%d;%d",SC_Acquire,LockPos,netname,0);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		return;
	#else
	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}

    // Acquire the lock associated with id LockPos.  WITH error reporting
    pageTableLock[TotalProcessCount]->Acquire();
    Lock *l = (Lock*)currentThread->space->lockTable.Get(LockPos);
	//printf("Lock:%d\n", (Lock*)currentThread->space->lockTable.Get(LockPos));
    if(l){
      l->Acquire();
	  //printf("Lock Acquired\n");
	  LockUsuageCntr[LockPos]++;
	  }
    else
      printf("%s","ERROR:Acquire lock failed!\n");
    pageTableLock[TotalProcessCount]->Release();
		#endif
}

  
void ReleaseLock_Syscall(int LockPos)
{
		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock ID should come from User Program		
		// request_type;lock ID; client ID, threadID=0
		sprintf(data,"%d;%d;%d;%d",SC_Release,LockPos,netname,0);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		return;
	#else
	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}

     // Release the lock associated with id LockPos.  WITH error reporting
    pageTableLock[TotalProcessCount]->Acquire();
    Lock *l = (Lock*)currentThread->space->lockTable.Get(LockPos);
    if(l){
	  if(LockUsuageCntr[LockPos]>0)
		{
		  l->Release();
		  //printf("Lock Released\n");
		  LockUsuageCntr[LockPos]--;
		}
	  else
		{
		  printf("%s","ERROR:Release lock failed!\n");
		}
	  }
    else
      printf("%s","ERROR:Release lock failed!\n");
    pageTableLock[TotalProcessCount]->Release();
		#endif
}


int CreateCondition_Syscall(unsigned int vaddr, int len)
{
    // Create a condition  with the name in the user buffer pointed to by
    // vaddr.  The condition name is at most MAXFILENAME chars long.  If
    // the condition is created successfully, it is put in the address
    // space's lock table and an id (condition variable id) returned that can find the lock
    // later.  If there are any errors, -1 is returned.

	if((len<=0)||(len>MAXFILENAME))
	{
		printf("Invalid size for Condition name\n");
		return -1;
	}
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    Condition *c;			// The new lock object
    int id;				// The lockid
	if((vaddr<0)||(vaddr>MAXADDR))
	 {
	 	printf("Invalid Virtual Address\n");
		return -1;
	 }

	if((vaddr+len)>MAXADDR)
    {
       printf("%s","ERROR: Address length exceeded the total size\n");
       return -1;
    }

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in CreateCondition\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to CreateCondition\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';
					#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock Name should come from User Program
		sprintf(data,"%d;%s;%d;%d",SC_CreateCondition,buf,netname,0);
		printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		delete[] buf;
		return atoi(buffer);
	#else

    pageTableLock[TotalProcessCount]->Acquire();
    c = new Condition("CreateConditionVariable");
	//printf("CV:%d\n",c);
    if ( c )
       {	  
        if ((id = currentThread->space->conditionTable.Put(c)) == -1 )
		{
    		delete c;
			printf("unable to Create CV \n");
		}
    	else
    	{
			CVUsuageCntr[id]=0;
    	    pageTableLock[TotalProcessCount]->Release();
			//printf("Condition Variable created\n");
    		return id;
    	}
       }

    pageTableLock[TotalProcessCount]->Release();
    return -1; 
#endif		
}


void DestroyCondition_Syscall(int ConditionPos)
{
		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock Name should come from User Program
		sprintf(data,"%d;%d;%d;%d",SC_DestroyCondition,ConditionPos,netname,0);
		printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		//delete[] buf;
		return;
	#else
    // Destroy the ConditionVariable associated with id ConditionPos.  WITH error reporting
	if((ConditionPos<0)||(ConditionPos>MAXCVS))
	{
		printf("ERROR: Invalid CV Index\n");
		return;
	}
    pageTableLock[TotalProcessCount]->Acquire();
    Condition *c = (Condition*)currentThread->space->conditionTable.Get(ConditionPos);
	if(CVUsuageCntr[ConditionPos]==0)
	{
		if(c){
		  delete c;
		  currentThread->space->conditionTable.Remove(ConditionPos);
		  //printf("Condition Destroyed\n");
		  }
		 else
			printf("%s","ERROR:Destroy Condition failed!\n");
	}
    else
      printf("ERROR:Condition still in use, Destroy Condition failed %d!\n",ConditionPos);
    
    pageTableLock[TotalProcessCount]->Release();
		#endif
}

void Wait_Syscall(int ConditionPos,int LockPos)
{
	#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock ID should come from User Program
		sprintf(data,"%d;%d;%d;%d;%d",SC_Wait,LockPos,ConditionPos,netname,0);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		return;
	#else
	if((ConditionPos<0)||(ConditionPos>MAXCVS))
	{
		printf("ERROR: Invalid CV Index\n");
		return;
	}

	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}

    Lock *l;
	Condition *c;

    pageTableLock[TotalProcessCount]->Acquire();
	l = (Lock*)currentThread->space->lockTable.Get(LockPos);
	//printf("Lock:%d\n", (Lock*)currentThread->space->lockTable.Get(LockPos));
    c = (Condition*)currentThread->space->conditionTable.Get(ConditionPos);
	//printf("CV:%d\n %d\n", (Condition*)currentThread->space->conditionTable.Get(ConditionPos),ConditionPos);
	pageTableLock[TotalProcessCount]->Release();
    
    if (c)
		{
      		if (l)
				{
				    CVUsuageCntr[ConditionPos]++;
					//printf("Wait\n");
					c->Wait(l);
				}
		
		    else
				{
					printf("%s","Bad lock passed\n");  
				}
		}
    else
		{
      		printf("%s","Bad Condition Variable passed!\n");  
		}
		#endif
}


void Signal_Syscall(int ConditionPos,int LockPos)
{
		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock ID should come from User Program
		sprintf(data,"%d;%d;%d;%d;%d",SC_Signal,LockPos,ConditionPos,netname,0);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		return;
	#else
		if((ConditionPos<0)||(ConditionPos>MAXCVS))
	{
		printf("ERROR: Invalid CV Index\n");
		return;
	}

	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}

    Lock *l;
	Condition *c;

    pageTableLock[TotalProcessCount]->Acquire();
	l = (Lock*)currentThread->space->lockTable.Get(LockPos);
    c = (Condition*)currentThread->space->conditionTable.Get(ConditionPos);
	pageTableLock[TotalProcessCount]->Release();
	if (c)
		{
	  		if (l)
				{
					c->Signal(l);
					//printf("Signalled\n");
					CVUsuageCntr[ConditionPos]--;
				}
		
		    else
				{
					printf("%s","Bad lock passed\n");  
				}
		}
	else
		{
	  		printf("%s","Bad Condition Variable passed!\n");  
		}
		#endif
}

void Broadcast_Syscall(int ConditionPos,int LockPos)
{
			#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = ServerID;
		outMailHdr.to = 0;
		outMailHdr.from = netname;
		//Lock ID should come from User Program
		sprintf(data,"%d;%d;%d;%d;%d",SC_Broadcast,LockPos,ConditionPos,netname,0);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		printf("Sending \"%s\" to %d\n",data,ServerID);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
		printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		return;
	#else
		if((ConditionPos<0)||(ConditionPos>MAXCVS))
	{
		printf("ERROR: Invalid CV Index\n");
		return;
	}

	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}

	Lock *l;
	Condition *c;

    pageTableLock[TotalProcessCount]->Acquire();
	l = (Lock*)currentThread->space->lockTable.Get(LockPos);
    c = (Condition*)currentThread->space->conditionTable.Get(ConditionPos);
	pageTableLock[TotalProcessCount]->Release();
	if (c)
		{
	  		if (l)
				{
					c->Broadcast(l);
				}
		
		    else
				{
					printf("%s","Bad lock passed\n");  
				}
		}
	else
		{
	  		printf("%s","Bad Condition Variable passed!\n");  
		}
		#endif
}


void Yield_Syscall()
{
   
    currentThread->Yield();

}

  
  
void Exit_Syscall(int exitCode)
{

	if((currentThread->currentThreadID!=-1)&&(currentThread->currentProcID!=-1))
	{
		processTableLock->Acquire();
		if(processTable[currentThread->currentProcID].AliveThreadCount==0)
		{
			if(ActiveNoOfProcess==1)
			{
				
				processTableLock->Release();
				//Shutdown OS
				interrupt->Halt();
			}
			else
			{
				
				//Destroy page table
				delete currentThread->space;
				ActiveNoOfProcess--;
				processTableLock->Release();
				currentThread->Finish();
			}
		}
		else
		{
			//Get the stack top address to destroy this thread stack
			int myStackTop=processTable[currentThread->currentProcID].StackLoc[currentThread->currentThreadID];
			
			currentThread->space->DestroyStackBitMap(myStackTop);
			//default the stack loc on Proc table
			processTable[currentThread->currentProcID].StackLoc[currentThread->currentThreadID]=NULL;
			//Decrement the Active Thread count
			if(processTable[currentThread->currentProcID].AliveThreadCount>0)
				 processTable[currentThread->currentProcID].AliveThreadCount--;
				 
			processTableLock->Release();
			//main Thread end
			currentThread->Finish();			
		}
	}
	else
	{
		currentThread->Finish();
	}
}


void KernelFunc(int vaddr){
	
//setup all registers and then switch to user mode to run the user program
  //write to the register PCReg the virtual address.
  machine->WriteRegister(PCReg, vaddr);
  //write virtualaddress + 4 in NextPCReg.
  machine->WriteRegister(NextPCReg,vaddr+4);
  //call Restorestate function inorder to prevent information loss while context switching.
  currentThread->space->RestoreState();    
	//get value from process table write to the stack register , the starting postion of the stack for this thread.

	
	processTableLock->Acquire();
	int saddr = processTable[currentThread->currentProcID].StackLoc[currentThread->currentThreadID];   //this is the stack address in process table for current thread
	
	machine->WriteRegister(StackReg, saddr);    //write the current thread's stack address to stackReg
	processTableLock->Release();
	machine->Run();
}

void Fork_Syscall( unsigned int vaddr)
{
	
	Thread *t = new Thread("CurrentThread");
	unsigned int Size;
	if(currentThread->space==NULL)
	{
		printf("%s","ERROR: Thread has illegal address space\n");
		return;
	}

	if(vaddr<0 || (vaddr>65536))
	{
		printf("%s","ERROR: Invalid Virtual address\n");
		return;
	}
	int currentPID =-1;
	processTableLock->Acquire();
	currentPID=currentThread->currentProcID;
	processTableLock->Release();
	

	if(pageTableLock[currentPID]==NULL)
	{
		printf("pageTableLock is a NULL pointer\n");
		return; 
	}
	if((processTable[currentPID].TotalThreads+1)>MAX_POSSIBLE_THREADS)
	{
		printf("No more threads are possible!!\n");
		return;
	}
	pageTableLock[currentPID]->Acquire();
	int numPages = currentThread->space->GetnumPages();
	_TranslationEntry *PageTable = new _TranslationEntry[numPages];
	PageTable=currentThread->space->GetpageTable();
	_TranslationEntry *newPageTable = new _TranslationEntry[numPages+8];
	 
	 //copy old to new pagetable
	 for(int i = 0; i < numPages; i++){
		newPageTable[i].virtualPage = PageTable[i].virtualPage;
		newPageTable[i].physicalPage = PageTable[i].physicalPage;		
		newPageTable[i].valid = PageTable[i].valid;
		newPageTable[i].use = PageTable[i].use;  //Hardware sets this whenever page is referenced or modified
		newPageTable[i].dirty = PageTable[i].dirty;
		newPageTable[i].readOnly = PageTable[i].readOnly;  
		newPageTable[i].SwapLocation=PageTable[i].SwapLocation;
		newPageTable[i].location=PageTable[i].location;
		newPageTable[i].file_offset=PageTable[i].file_offset;
		
	 }
	 
	// initialize the new created pagetable entry
	 for(int i = numPages; i < numPages+8; i++ ){
		newPageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		newPageTable[i].valid = TRUE;
		newPageTable[i].use = FALSE;  //Hardware sets this whenever page is referenced or modified
		newPageTable[i].dirty = FALSE;
		newPageTable[i].readOnly = FALSE;  // if the code segment was entirely on        
		newPageTable[i].SwapLocation=-1;
		newPageTable[i].location=2;
		newPageTable[i].file_offset=-1;
	 }
	 
	 delete[] PageTable;
	 numPages=numPages+8;
	 currentThread->space->DelPageTable();
	 currentThread->space->SetnumPages(numPages);	 
	 currentThread->space->SetpageTable(newPageTable);

	 pageTableLock[currentPID]->Release();
	 
	 t->space=currentThread->space;
	 // // Update the process table
	 // // Get the process ID
	 processTableLock->Acquire();
	 processTable[currentPID].TotalThreads++;
	 processTable[currentPID].AliveThreadCount++;
	 t->currentThreadID=processTable[currentPID].TotalThreads;
	 t->currentProcID=currentPID;
	 processTable[currentPID].StackLoc[t->currentThreadID]=(numPages * PageSize)-16;
	 processTableLock->Release();
	 t->Fork((VoidFunctionPtr)KernelFunc,vaddr);
	
}
 

void ExecFunc(int addr)
{
	
		//setup all registers and then switch to user mode to run the user program
		//write to the register PCReg the virtual address.
    currentThread->space->InitRegisters();
		//call Restorestate function inorder to prevent information loss while context switching.
    currentThread->space->RestoreState();
		//Run the new executable as a thread!
		
    machine->Run();
		
}

SpaceId Exec_Syscall(unsigned int fileName,int len)
{
	
	
	if(len<=0)
		{
			printf("Invalid filename length for EXEC system call!\n");
			return -1;
		}
	
    OpenFile *executable;
	
    char *buf= new char(len+1);
		if(copyin(fileName,len,buf)==-1 ) 
		{
			printf("%s","Bad pointer passed to Create\n");
			delete buf;
			return -1;
		}
		
		buf[ len]='\0';
		
		
        executable = fileSystem->Open(buf);
	   
		delete buf;
		
    if(executable==0)
		{
			printf("File not found\n");
			return -1;
		}
        printf("line number 823 \n");
		AddrSpace *myProcAddrSpace=new AddrSpace(executable);
		//delete executable;
		
		//Update proc table
		processTableLock->Acquire();
		ActiveNoOfProcess++;
		processTable[TotalProcessCount].myProcAddrSpace=myProcAddrSpace;
		processTable[TotalProcessCount].PID=TotalProcessCount;
		processTable[TotalProcessCount].TotalThreads=0;
		processTable[TotalProcessCount].AliveThreadCount=0;
		
		//Get the Stack Address
		int StackTop=((myProcAddrSpace->GetnumPages())*PageSize)-16;
		processTable[TotalProcessCount].StackLoc[0]=StackTop;
		Thread *t = new Thread("ChildThread");
		t->space=myProcAddrSpace;
		t->currentThreadID=0;
		t->currentProcID=TotalProcessCount;
		TotalProcessCount++;
		processTableLock->Release();
		//Start new process created as a thread
		t->Fork((VoidFunctionPtr)ExecFunc,0);
		
		//return the address space identifier
		return (int)t->space;
}
 
void Print_Syscall(unsigned int vaddr)
{
   int len=150;
   char *buf = new char[len];  // Kernel buffer to put the name in

   if (!buf) return;
       //buffer exists
   if( copyin(vaddr,len,buf) == -1 ) {
       printf("%s","Bad pointer passed to Print\n");
       delete buf;
       return;
   }
       //data to be printed on screen exists

   buf[len]='\0';

printf(buf);
}

void Print1_Syscall(unsigned int vaddr, int i)
{
   int len=90;
   char *buf = new char[len];  // Kernel buffer to put the name in

   if (!buf) return;
       //buffer exists
   if( copyin(vaddr,len,buf) == -1 ) {
       printf("%s","Bad pointer passed to Print\n");
       delete buf;
       return;
   }
       //data to be printed on screen exists
   buf[len]='\0';

printf(buf,i);
}

void Print2_Syscall(unsigned int vaddr, int i, int j)
{
   int len=90;
   char *buf = new char[len];  // Kernel buffer to put the name in

   if (!buf) return;
       //buffer exists
   if( copyin(vaddr,len,buf) == -1 ) {
       printf("%s","Bad pointer passed to Print\n");
       delete buf;
       return;
   }
       //data to be printed on screen exists
   buf[len]='\0';

printf(buf,i,j);
}

void Print3_Syscall(unsigned int vaddr, int i, int j, int k)
{
   int len=200;
   char *buf = new char[len];  // Kernel buffer to put the name in

   if (!buf) return;
       //buffer exists
   if( copyin(vaddr,len,buf) == -1 ) {
       printf("%s","Bad pointer passed to Print\n");
       delete buf;
       return;
   }
       //data to be printed on screen exists
   buf[len]='\0';

printf(buf,i,j,k);
}

int Scan_Syscall(unsigned int vaddr)
{
   int len=2;
   int p;
   char *buf = new char[len+1];        // Kernel buffer to put the name in

   if (!buf) return -1;
       //buffer exists
   if( copyin(vaddr,len,buf) == -1 ) {
       printf("%s","Bad pointer passed to Scan\n");
       delete buf;
       return -1;
   }
       //scanf variable exists
   buf[len]='\0';
   scanf(buf,&p);
	 //printf("%d\n",p);
   return p;
}

#ifdef NETWORK
int CreateMV_Syscall(unsigned int vaddr, int len)
{
    // Create a MV  with the name in the user buffer pointed to by
    // vaddr.  The MV name is at most MAXFILENAME chars long.  

	if((len<=0)||(len>MAXFILENAME))
	{
		printf("Invalid size for Lock name\n");
		return -1;
	}
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    Lock *l;			// The new lock object
    int id;				// The lockid

	if((vaddr<0)||(vaddr>MAXADDR))
	 {
	 	printf("Invalid Virtual Address\n");
		return -1;
	 }

	if((vaddr+len)>MAXADDR)
	{
		 printf("%s","ERROR: Address length exceeded the total size\n");
		 return -1;
	}
  if (!buf)
	{
		printf("%s","Can't allocate kernel buffer in CreateLock\n");
		return -1;
  }

	if( copyin(vaddr,len,buf) == -1 )
	{
		printf("%s","Bad pointer passed to CreateLock\n");
		delete[] buf;
		return -1;
	}
	//printf("Inside NW Cl\n");
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = ServerID;
	outMailHdr.to = 0;
	outMailHdr.from = netname;
	//Lock Name should come from User Program
	// request_type;lock name; client ID, threadID=0
	sprintf(data,"%d;%s;%d;%d",SC_CreateMV,buf,netname,0);
	printf("Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	printf("Sending \"%s\" to %d\n",data,ServerID);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
	printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	delete[] buf;
	return atoi(buffer);
}
   
void DestroyMV_Syscall(int MVID)
{

	if((MVID<0)||(MVID>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}


	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = ServerID;
	outMailHdr.to = 0;
	outMailHdr.from = netname;
	//Lock Name should come from User Program
	// request_type;lock id; client ID, threadID=0
	sprintf(data,"%d;%d;%d;%d",SC_DestroyMV,MVID,netname,0); 
	printf("Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	printf("Sending \"%s\" to %d\n",data,ServerID);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
	printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	return ;
	
}


int GetMV_Syscall(int MVID)
{

	if((MVID<0)||(MVID>MAXMV))
	{
		printf("ERROR: Invalid MV Index\n");
		return -1;
	}


	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = ServerID;
	outMailHdr.to = 0;
	outMailHdr.from = netname;
	//Lock Name should come from User Program
	// request_type;lock id; client ID, threadID=0
	sprintf(data,"%d;%d;%d;%d",SC_GetMV,MVID,netname,0); 
	printf("Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	printf("Sending \"%s\" to %d\n",data,ServerID);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
	printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	return atoi(buffer);
	
}

void SetMV_Syscall(int MVID,int MVValue)
{

	if((MVID<0)||(MVID>MAXMV))
	{
		printf("ERROR: Invalid MV Index\n");
		return ;
	}
	if((MVValue<0)||(MVID>65536))
	{
		printf("ERROR: %d is Too large or less value for MV\n",MVID);
		return ;
	}

	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = ServerID;
	outMailHdr.to = 0;
	outMailHdr.from = netname;
	//Lock Name should come from User Program
	// request_type;lock id; client ID, threadID=0
	sprintf(data,"%d;%d;%d;%d;%d",SC_SetMV,MVID,MVValue,netname,0); 
	printf("Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	printf("Sending \"%s\" to %d\n",data,ServerID);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(netname, &inPktHdr, &inMailHdr, buffer);
	printf("Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	return ;
	
}


#endif

void ExceptionHandler(ExceptionType which)
 {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall
	TranslationEntry *mypagetable;
	int vpa,vpnum;

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
      case SC_CreateLock:
		DEBUG('a', "Create Lock syscall.\n");
		rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
      case SC_DestroyLock:
		DEBUG('a', "Destroy Lock syscall.\n");
		DestroyLock_Syscall(machine->ReadRegister(4));
		break;
      case SC_Acquire:
		DEBUG('a', "Acquiring lock syscall.\n");
		AcquireLock_Syscall(machine->ReadRegister(4));
		break;
      case SC_Release:
		DEBUG('a', "Releasing lock syscall.\n");
		ReleaseLock_Syscall(machine->ReadRegister(4));
		break;
      case SC_CreateCondition:
		DEBUG('a', "Create Condition variable syscall.\n");
		rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
      case SC_DestroyCondition:
		DEBUG('a', "Destroy Condition variable syscall.\n");
		DestroyCondition_Syscall(machine->ReadRegister(4));
		break;
      case SC_Wait:
		DEBUG('a', "Wait on condition variable syscall.\n");
		Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
      case SC_Signal:
		DEBUG('a', "Signal condition variable syscall.\n");
		Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
      case SC_Broadcast:
		DEBUG('a', "Broadcast condition variable syscall.\n");
		Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
      case SC_Yield:
		DEBUG('a', "Yield syscall.\n");
		Yield_Syscall();
		break;
    case SC_Exit:
		DEBUG('a', "Exit syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;
    case SC_Fork:
		DEBUG('a', "Fork syscall.\n");
		Fork_Syscall(machine->ReadRegister(4));
		break;
    case SC_Exec:
		DEBUG('a', "Exec syscall.\n");
		rv = Exec_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		//Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	  case SC_Print:
		DEBUG('a', "Print syscall.\n");
		Print_Syscall(machine->ReadRegister(4));
		break;
	  case SC_Print1:
		DEBUG('a', "Print syscall.\n");
		Print1_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	  case SC_Print2:
		DEBUG('a', "Print2 syscall.\n");
		Print2_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;
	  case SC_Print3:
		DEBUG('a', "Print3 syscall.\n");
		Print3_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
		break;
	  case SC_Scan:
		DEBUG('a', "Scan syscall.\n");
		rv=Scan_Syscall(machine->ReadRegister(4));
		break;
#ifdef NETWORK		
      case SC_CreateMV:
		DEBUG('a', "Create Lock syscall.\n");
		rv = CreateMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
      case SC_DestroyMV:
		DEBUG('a', "Destroy Lock syscall.\n");
		DestroyMV_Syscall(machine->ReadRegister(4));
		break;
      case SC_GetMV:
		DEBUG('a', "Destroy Lock syscall.\n");
		rv = GetMV_Syscall(machine->ReadRegister(4));
		break;
      case SC_SetMV:
		DEBUG('a', "Destroy Lock syscall.\n");
		SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;	
#endif		
	}

			// Put in the return value and increment the PC
			machine->WriteRegister(2,rv);
			machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
			machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
			machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
			return;
    } 
	
		else if(which == PageFaultException)
		{
			//TLB miss
			int vAdd=machine->ReadRegister(39);
			if(vAdd>=0)
			{
				//page fault handler
				HandlePageFault(machine->ReadRegister(39));
			}
			else
			{
				//checking if -ve value in the address
				printf("Negative vAdd:%d\n",vAdd);
				currentThread->Finish();
			}
		}

	
	else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }

}

void HandlePageFault(int vAdd)
{
	
	int vpn,ppn;
	vpn=vAdd/PageSize;
	if(vpn<0)
	{
		printf("Negative VPN\n");
		
	}
	//checking the corresponding entry in the ipt
	ppn=getIPTindex(vpn);
	// IPT miss!!!!
	if(ppn==-1)
	{
		//Update IPT => Load exec into main memory
		ppn=loadPageIntoIPT(vpn);
		
		if(ppn<0)
		{
			//no physical pages!!!!!
			interrupt->Halt();
		}
	}	
	updateTLB(ppn);
}

int getIPTindex(int vpn)
{
	int i;
	iptlock->Acquire();
	for(i=0;i<NumPhysPages;i++)
	{ 
	 if(ipt[i].virtualPage==vpn &&  ipt[i].valid==true && ipt[i].ProcID==currentThread->currentProcID)
	 {
		//ipt entry found
		iptlock->Release();
		return i;
	 }
	}
			
 iptlock->Release();
//ipt miss 
 return -1;
}

void updateTLB(int i)
{    
//updating the TLB entries from the IPT of the corrresponding physical page 
	IntStatus old=interrupt->SetLevel(IntOff);
	if(machine->tlb[currentTLB].valid==true)
	{
		ipt[machine->tlb[currentTLB].physicalPage].dirty=machine->tlb[currentTLB].dirty;
	}
	machine->tlb[currentTLB].virtualPage=ipt[i].virtualPage;
	machine->tlb[currentTLB].physicalPage=ipt[i].physicalPage;
	machine->tlb[currentTLB].valid=true;
	machine->tlb[currentTLB].readOnly=ipt[i].readOnly;
	machine->tlb[currentTLB].use=ipt[i].use;
	machine->tlb[currentTLB].dirty=ipt[i].dirty;
	currentTLB++;
	if(currentTLB>3)
	currentTLB=0;
	interrupt->SetLevel(old);
}

int loadPageIntoIPT(int vpn)
{
	int ppn;
	iptlock->Acquire();
	//if the page is in the executable

	if(currentThread->space->myPageTable[vpn].location==0)       
	{
		bitMapLock->Acquire();
	  ppn = PhyMemBitMap->Find();	//Find() will find the free bit and also sets the locn it found
		
		if(ppn!=-1)
		{
			currentThread->space->myPageTable[vpn].physicalPage = ppn; 
			int tempPPN=ppn;
			fifoLock->Acquire();            //
			dummyfifo[dummyfifoCnt++]=ppn; //useful for FIFO option only
			
			fifoLock->Release();		  //
			bitMapLock->Release();
		}
		else
		{
			//physical page not available
			bitMapLock->Release();
			ppn=evictApage1(vpn);
		}
	//loading exec
		currentThread->space->exec->ReadAt(&(machine->mainMemory[ppn*PageSize]),PageSize,currentThread->space->myPageTable[vpn].file_offset);
		ipt[ppn].physicalPage=ppn;
		ipt[ppn].virtualPage=vpn;
		ipt[ppn].valid = true;
		ipt[ppn].use = false;
		ipt[ppn].dirty = false;
		ipt[ppn].readOnly = false;
		ipt[ppn].ProcID=currentThread->currentProcID;
		iptlock->Release();
				
		return ppn;
	}
	else if(currentThread->space->myPageTable[vpn].location==2) //if page is in Stack
	{ 
		
		bitMapLock->Acquire();
		ppn = PhyMemBitMap->Find();	
		
		if(ppn!=-1)
		{
			fifoLock->Acquire();             //
			dummyfifo[dummyfifoCnt++]=ppn;	//useful only for FIFO option		
			fifoLock->Release();	       //
			currentThread->space->myPageTable[vpn].physicalPage = ppn; 
			bitMapLock->Release();
		}
		else
		{
			//physical page not found
			bitMapLock->Release();
			ppn=evictApage1(vpn);
		}
		ipt[ppn].physicalPage=ppn;
		ipt[ppn].virtualPage=vpn;
		ipt[ppn].valid = true;
		ipt[ppn].use = false;
		ipt[ppn].dirty = false;
	
		ipt[ppn].readOnly = false;
		ipt[ppn].ProcID=currentThread->currentProcID;
		iptlock->Release();
		
		return ppn;
	}
	else
	{
		//Read from Swap file
		int readFrom=currentThread->space->myPageTable[vpn].SwapLocation*PageSize;
		bitMapLock->Acquire();
		ppn = PhyMemBitMap->Find();	//Find() will find the free bit and also sets the locn it found
		
		if(ppn!=-1)
		{
			fifoLock->Acquire();                //
			dummyfifo[dummyfifoCnt++]=ppn;     //useful only for FIFO option
			
			 fifoLock->Release();		        //
			currentThread->space->myPageTable[vpn].physicalPage = ppn; 
			bitMapLock->Release();
		}
		else
		{
			//physical page not found
			bitMapLock->Release();
			ppn=evictApage1(vpn);
		}
		//load the swapfile
		SwapFile->ReadAt(&(machine->mainMemory[ppn*PageSize]),PageSize,readFrom);	
		ipt[ppn].physicalPage=ppn;
		ipt[ppn].virtualPage=vpn;
		
		ipt[ppn].valid = true;
		ipt[ppn].use = false;
		ipt[ppn].dirty = false;
		
		ipt[ppn].readOnly = false;
		ipt[ppn].ProcID=currentThread->currentProcID;
		iptlock->Release();
		return ppn;
	}	
}



int evictApage1(int vpn)
{
	int evictAPageProcID;
	AddrSpace* evictAPageSpace=NULL;
	int SwapFileLoc;
	int evictAPage;
	
	if(rplacementPol==0)
	{
		//fifo page replacement policy deployed
		evictAPage=fifofn();
	}
	else if(rplacementPol==1)
	{
		//random page replacement policy deployed
		evictAPage=Random()%NumPhysPages;
	}
	
	evictAPageSpace=currentThread->space; 
	IntStatus old=interrupt->SetLevel(IntOff);
	for (int i = 0; i<TLBSize; i++)
	{
		if(machine->tlb[i].physicalPage==evictAPage)
		{
			
			machine->tlb[i].valid = false;
			ipt[evictAPage].dirty=machine->tlb[i].dirty;
			
		}       
	}
	interrupt->SetLevel(old);

	if(ipt[evictAPage].dirty)
	{
		//page is dirty Save to swap file
		int currVPN=ipt[evictAPage].virtualPage;
		evictAPageSpace->myPageTable[currVPN].SwapLocation=SwapBitMap->Find();
		
		int writeTo=evictAPageSpace->myPageTable[currVPN].SwapLocation*PageSize;
		
		SwapFile->WriteAt(&(machine->mainMemory[evictAPage*PageSize]), PageSize,writeTo );		
		evictAPageSpace->myPageTable[currVPN].location=1;
		evictAPageSpace->myPageTable[currVPN].physicalPage=-1;
		bzero(&(machine->mainMemory[evictAPage*PageSize]),PageSize);
	}
	else
	{
		evictAPageSpace->myPageTable[vpn].physicalPage=-1;
		bzero(&(machine->mainMemory[evictAPage*PageSize]),PageSize);
	}
	return evictAPage;
}

int fifofn()
{
	fifoLock->Acquire();
	int pp=dummyfifo[0];
	for(int i=0;i<dummyfifoCnt;i++)
	{
		dummyfifo[i]=dummyfifo[i+1];
	}
	dummyfifo[dummyfifoCnt]=pp;
	fifoLock->Release();
	return pp;
}

