


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
//	For instance, accessing memory that doesn't exist, arithmetic errors ,
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

#define MAXADDR 1048576
#define MAXLOCKS 1000
#define MAXCVS 1000
#define MAXLOCKS 1000
#define MAXCV 1000
#define MAXMV 2000 


using namespace std;
Lock *TblUpdateLock =new Lock("TblUpdateLock");

void networkThread(int myID);
unsigned long int getTheTimeStamp();

typedef struct mailBox_t
{
	int MailBoxNo;
	unsigned long int TimeStamp;
};

typedef struct LastTimeStampTable_t
{
	unsigned long int TimeStamp;
};

//Project 3 code start
#define INVALID 	-100

enum Status{BUSY=0,FREE,ACTIVE,VOID};

typedef struct SvrLockData_t
{
	int IsOkToDestroy;
	int UsageCntr;
	char* LockName;
	int LockOwner;
	int LockThreadID;
	int Status;
};

typedef struct SvrCVData_t
{
	int IsOkToDestroy;
	int UsageCntr;
	char* CVName;
	int CVOwner;
	int CVThreadID;
	int Status;
};

typedef struct SvrMVData_t
{
	int IsOkToDestroy;
	int UsageCntr;
	char* MVName;
	int MVValue;
	int MVOwner;
	int MVThreadID;
	int Status;
};
// Declaration of all functions
bool isHeldByCurrentThread(int LockID,int ClientID,int ThreadID,SvrLockData_t *SvrLocks);
void RespondToClient(char *SrvBuffer, int ClientID, int ThreadID,int ntThreadMailboxID);
void initialize(SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter);
void DecodeMsg(char *buffer, SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerCreateLock(int ClientID, int ThreadID,char *LockName, SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerAcquireLock(int ClientID, int ThreadID,int LockID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerReleaseLock(int ClientID, int ThreadID,int LockID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerDestroyLock(int ClientID, int ThreadID,int LockID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerCreateCondition(int ClientID, int ThreadID,char *CvName,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerWait(int ClientID, int ThreadID,int LockID,int CvID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerDestroyCondition(int ClientID, int ThreadID,int CvID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerSignal(int ClientID, int ThreadID,int LockID,int CvID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerBroadcast(int ClientID, int ThreadID,int LockID,int CvID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerCreateMV(int ClientID, int ThreadID,char *MVName,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerDestroyMV(int ClientID, int ThreadID,int MVID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerSetMV(int ClientID, int ThreadID,int MVID, int MVValue,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
int ServerGetMV(int ClientID, int ThreadID,int MVID,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ, int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID);
const int SUCCESS	= 1;
const int FAILURE	=-1;


//Global Variables for the server Kernel
int SignalledClientID=-1;
bool HideMessage;
int ReleaseCalledFromWait;

//Project 3 code end


//Project 4 code Start

Lock *ntThreadLock;
//Project 4 code end


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

//Code added for Project 2

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
    if (!buf) {
	printf("%s","Can't allocate kernel buffer in CreateLock\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to CreateLock\n");
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
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock Name should come from User Program
		// request_type;lock name
		sprintf(data,"%d;%s",SC_CreateLock,buf);
		DEBUG('q', "Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d client and %d mailbox\n",data,netname,outMailHdr.to);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"UPThread %d Recieved \"%s\" from %d client and %d mailbox\n",currentThread->getMailboxNO(),buffer,inPktHdr.from,inMailHdr.from);
		fflush(stdout);
		delete []buf;
		delete []data;
		return atoi(buffer);
	#endif
}
   
void DestroyLock_Syscall(int LockPos)
{
	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}
    // Destroy the lock associated with id LockPos.  WITH error reporting.
#ifdef NETWORK
		//printf("Inside destry lock.............\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock Name should come from User Program
		// request_type;lock id; client ID, threadID=0
		sprintf(data,"%d;%d",SC_DestroyLock,LockPos); 
		DEBUG('q',"Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		delete []data;
		return ;
	#endif
}


void AcquireLock_Syscall(int LockPos)
{
	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}
    // Acquire the lock associated with id LockPos.  WITH error reporting
		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock ID should come from User Program
		// request_type;lock ID; client ID, threadID=0
		sprintf(data,"%d;%d",SC_Acquire,LockPos);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		delete []data;
		return;
		#endif

}

  
void ReleaseLock_Syscall(int LockPos)
{
	if((LockPos<0)||(LockPos>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}
     // Release the lock associated with id LockPos.  WITH error reporting
		 		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock ID should come from User Program		
		// request_type;lock ID; client ID, threadID=0
		sprintf(data,"%d;%d",SC_Release,LockPos);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		delete []data;
		return;
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
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock Name should come from User Program
		sprintf(data,"%d;%s",SC_CreateCondition,buf);
		DEBUG('q',"Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d client and %d mailbox\n",data,netname,outMailHdr.to);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
				DEBUG('q',"UPThread %d Recieved \"%s\" from %d client and %d mailbox\n",currentThread->getMailboxNO(),buffer,inPktHdr.from,inMailHdr.from);
		fflush(stdout);
		delete[] buf;
		delete []data;
		return atoi(buffer);
	#endif
 
}


void DestroyCondition_Syscall(int ConditionPos)
{
    // Destroy the ConditionVariable associated with id ConditionPos.  WITH error reporting
	if((ConditionPos<0)||(ConditionPos>MAXCVS))
	{
		printf("ERROR: Invalid CV Index\n");
		return;
	}
		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock Name should come from User Program
		sprintf(data,"%d;%d",SC_DestroyCondition,ConditionPos);
		DEBUG('q',"Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		//delete[] buf;
		delete []data;
		return;
		#endif
}

void Wait_Syscall(int ConditionPos,int LockPos)
{
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
	#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock ID should come from User Program
		sprintf(data,"%d;%d;%d",SC_Wait,LockPos,ConditionPos);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
 	delete []data;
		return;
	#endif


}


void Signal_Syscall(int ConditionPos,int LockPos)
{
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

		#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock ID should come from User Program
		sprintf(data,"%d;%d;%d",SC_Signal,LockPos,ConditionPos);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
		fflush(stdout);
		delete []data;
		return;
	#endif

}

void Broadcast_Syscall(int ConditionPos,int LockPos)
{
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
				#ifdef NETWORK
		//printf("Inside NW Cl\n");
		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *data=new char[100];
		char buffer[MaxMailSize];
		outPktHdr.to = netname;
		outMailHdr.to = (currentThread->getMailboxNO()+1);
		outMailHdr.from = currentThread->getMailboxNO();
		//Lock ID should come from User Program
		sprintf(data,"%d;%d;%d",SC_Broadcast,LockPos,ConditionPos);
		//printf("Data:%s\n",data);
		outMailHdr.length = strlen(data)+1;
		DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
		if(!postOffice->Send(outPktHdr,outMailHdr,data)){
			printf("AcquireLock Send failed. You must have Server nachos running. Nachos Terminating\n");
			interrupt->Halt();
		}
		postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Recieved \"%s\" from %d\n",buffer,inPktHdr.from);
		fflush(stdout);
		delete []data;
		return;
	#endif

}

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
	buf[len]='\0';
	#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = netname;
	outMailHdr.to = (currentThread->getMailboxNO()+1);
	outMailHdr.from = currentThread->getMailboxNO();
	//Lock Name should come from User Program
	// request_type;lock name; client ID, threadID=0
	sprintf(data,"%d;%s;%d;%d",SC_CreateMV,buf,netname,0);
	DEBUG('q',"Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("CreateLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
	DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	delete[] buf;
	delete []data;
	return atoi(buffer);
	#endif
}
   
void DestroyMV_Syscall(int MVID)
{

	if((MVID<0)||(MVID>MAXLOCKS))
	{
		printf("ERROR: Invalid Lock Index\n");
		return;
	}
	#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = netname;
	outMailHdr.to = (currentThread->getMailboxNO()+1);
	outMailHdr.from = currentThread->getMailboxNO();
	//Lock Name should come from User Program
	// request_type;lock id; client ID, threadID=0
	sprintf(data,"%d;%d;%d;%d",SC_DestroyMV,MVID,netname,0); 
	DEBUG('q',"Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
	DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	delete []data;
	return ;
	#endif
}


int GetMV_Syscall(int MVID)
{

	if((MVID<0)||(MVID>MAXMV))
	{
		printf("ERROR: Invalid MV Index\n");
		return -1;
	}
#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = netname;
	outMailHdr.to = (currentThread->getMailboxNO()+1);
	outMailHdr.from = currentThread->getMailboxNO();
	//Lock Name should come from User Program
	// request_type;lock id; client ID, threadID=0
	sprintf(data,"%d;%d;%d;%d",SC_GetMV,MVID,netname,0); 
	DEBUG('q',"Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
	DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	delete []data;
	return atoi(buffer);
		#endif
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
#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *data=new char[100];
	char buffer[MaxMailSize];
	outPktHdr.to = netname;
	outMailHdr.to = (currentThread->getMailboxNO()+1);
	outMailHdr.from = currentThread->getMailboxNO();
	//Lock Name should come from User Program
	// request_type;lock id; client ID, threadID=0
	sprintf(data,"%d;%d;%d;%d;%d",SC_SetMV,MVID,MVValue,netname,0); 
	DEBUG('q',"Data:%s\n",data);
	outMailHdr.length = strlen(data)+1;
	DEBUG('q',"Sending \"%s\" to %d\n",data,netname);
	if(!postOffice->Send(outPktHdr,outMailHdr,data)){
		printf("DestroyLock Send failed. You must have Server nachos running. Nachos Terminating\n");
		interrupt->Halt();
	}
	postOffice->Receive(currentThread->getMailboxNO(), &inPktHdr, &inMailHdr, buffer);
	DEBUG('q',"Recieved \"%s\" from %d\n",data,inPktHdr.from);
	fflush(stdout);
	delete []data;
	return ;
		#endif
}


void Yield_Syscall()
{
    //pageTableLock[TotalProcessCount]->Acquire();
    currentThread->Yield();
    //pageTableLock[TotalProcessCount]->Release();
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
				//printf("Last thread of last process with PID %d exit\n",currentThread->currentThreadID);
				processTableLock->Release();
				//Shutdown OS
				interrupt->Halt();
			}
			else
			{
				//printf("Last thread of PID %d exit\n",currentThread->currentProcID);
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
			//printf("PID %d Thread ID %d Stack Top %d\n",currentThread->currentProcID, currentThread->currentThreadID,myStackTop);
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
	//printf("Entering KernelFunc\n");
//setup all registers and then switch to user mode to run the user program
  //write to the register PCReg the virtual address.
  machine->WriteRegister(PCReg, vaddr);
  //write virtualaddress + 4 in NextPCReg.
  machine->WriteRegister(NextPCReg,vaddr+4);
  //call Restorestate function inorder to prevent information loss while context switching.
  currentThread->space->RestoreState();    
	//get value from process table write to the stack register , the starting postion of the stack for this thread.

	//printf("MyPID%d\n",j);
	processTableLock->Acquire();
	int saddr = processTable[currentThread->currentProcID].StackLoc[currentThread->currentThreadID];   //this is the stack address in process table for current thread
	//printf("Stack Address%d\n",saddr);
	machine->WriteRegister(StackReg, saddr);    //write the current thread's stack address to stackReg
	processTableLock->Release();
	machine->Run();
}

void Fork_Syscall( unsigned int vaddr)
{
	//printf("Entering Fork%d\n",vaddr);
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
	//printf("currentPID%d\n",currentPID);

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
	TranslationEntry *PageTable = new TranslationEntry[numPages];;
	PageTable=currentThread->space->GetpageTable();
	TranslationEntry *newPageTable = new TranslationEntry[numPages+8];
	 
	 //copy old to new pagetable
	 for(int i = 0; i < numPages; i++){
		newPageTable[i].virtualPage = PageTable[i].virtualPage;
		newPageTable[i].physicalPage = PageTable[i].physicalPage;		
		newPageTable[i].valid = PageTable[i].valid;
		newPageTable[i].use = PageTable[i].use;  //Hardware sets this whenever page is referenced or modified
		newPageTable[i].dirty = PageTable[i].dirty;
		newPageTable[i].readOnly = PageTable[i].readOnly;  
	 }
	 
	// initialize the new created pagetable entry
	 for(int i = numPages; i < numPages+8; i++ ){
		newPageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		bitMapLock->Acquire();
		int pp = PhyMemBitMap->Find();	//Find() will find the free bit and also sets the locn it found
		
		if( pp == -1 ){
			printf("No more memory available!!Execution stops now!\n");
			interrupt->Halt();
		}
		//executable->ReadAt(pp*PageSize, PageSize, noffH.code.inFileAddr+(i*PageSize));
		newPageTable[i].physicalPage = pp;
		bitMapLock->Release();
		newPageTable[i].valid = TRUE;
		newPageTable[i].use = FALSE;  //Hardware sets this whenever page is referenced or modified
		newPageTable[i].dirty = FALSE;
		newPageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
	 }
	 
	 delete[] PageTable;
	 numPages=numPages+8;
	 currentThread->space->DelPageTable();
	 currentThread->space->SetnumPages(numPages);	 
	 currentThread->space->SetpageTable(newPageTable);
	 // PageTable = newPageTable;

	 machine->pageTable=newPageTable;
	 machine->pageTableSize = numPages;
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
		//printf("kernel exec fn\n");
		DEBUG('q',"kernel exec fn...................\n");
    machine->Run();
		printf("Serious error!!\n");
}

SpaceId Exec_Syscall(unsigned int fileName,int len)
{
	//printf("len %d\n",len);
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
		printf("buf %s\n",buf);
    executable = fileSystem->Open(buf);
		delete buf;
		//printf("executable %d\n",executable);
    if(executable==0)
		{
			printf("File not found\n");
      return -1;
    }
    
		AddrSpace *myProcAddrSpace=new AddrSpace(executable);
		delete executable;
		
		//Update proc table
		processTableLock->Acquire();
		ActiveNoOfProcess++;
		processTable[TotalProcessCount].myProcAddrSpace=myProcAddrSpace;
		processTable[TotalProcessCount].PID=TotalProcessCount;
		processTable[TotalProcessCount].TotalThreads=2;
		processTable[TotalProcessCount].AliveThreadCount=2;
		
		//Get the Stack Address
		int StackTop=((myProcAddrSpace->GetnumPages())*PageSize)-16;
		processTable[TotalProcessCount].StackLoc[0]=StackTop;
		Thread *t = new Thread("ChildThread");
		t->space=myProcAddrSpace;
		t->currentThreadID=0;
		t->currentProcID=TotalProcessCount;
		TotalProcessCount++;
		//Added for Project 4
		t->setMailboxNo(nextMailBoxNo);
		nextMailBoxNo++;
		Thread *ntThread = new Thread("ntThread");
		ntThread->setMailboxNo(nextMailBoxNo);
		ntThread->space=myProcAddrSpace;
		ntThread->currentThreadID=1;
		nextMailBoxNo++;
		//end added for Project 4
		processTableLock->Release();
		
		//Start new process created as a thread
		t->Fork((VoidFunctionPtr)ExecFunc,0);
		ntThread->Fork((VoidFunctionPtr)networkThread,(nextMailBoxNo-1));
		DEBUG('q',"Forked networkThread...................");
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

void StartSimulation()
{
	DEBUG('q',"Inside StartSimulation\n");
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char ServerRespbuffer[40];
	int myMailBoxNo=0;
	myMailBoxNo = currentThread->getMailboxNO();
	DEBUG('q',"StartSimulation...........mailbox ID %d\n",myMailBoxNo);
	DEBUG('q',"StartSimulation Waiting for message from NTThread\n");
	postOffice->Receive(myMailBoxNo, &inPktHdr, &inMailHdr, ServerRespbuffer);
	DEBUG('q',"Got \"%s\" from %d, box %d ** User prog to start simulation now!**\n",ServerRespbuffer,inPktHdr.from,inMailHdr.from); 
	fflush(stdout);
}

int NameGen_Syscall(unsigned int vaddr1, int i, int len, unsigned int vadd)
{
   printf("inside name gen\n");
 char *buf = new char[len+1];
 char *s = new char[1];
 int buff_len;
 
	 if( copyin(vaddr1,len,buf) == -1 )                              // copying from virtual adrress to buf
	 {
		printf("%s","Bad pointer passed to CreateCondition\n");
		delete[] buf;
		return -1;
	 }
	   sprintf(s,"%d",i);                         
	   strcat(buf,s);                                               // concatinating buf with the index
	   buff_len=strlen(buf);
	   
      if ( copyout(vadd, buff_len, buf) == -1 )                   // copying from buff back to virtual address
	  {
			printf("%s","Bad pointer passed to Read: data not copied\n");
			delete[] buf;
	        return -1;
      }
	  return 1;
   
}
void ExceptionHandler(ExceptionType which)
 {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall
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
		case SC_StartSimulation:
		DEBUG('a', "StartSimulation syscall.\n");
		StartSimulation();
		break;
		case SC_NameGen:
		DEBUG('a', "Print3 syscall.\n");
		NameGen_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
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
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }

}


void networkThread(int ntThreadMailboxID)
{
	DEBUG('q',"Inside networkThread\n");
	int SrvLockCounter=0;
	int SrvCVCounter=0;
	int SrvMVCounter=0;
	SvrLockData_t SvrLocks[MAXLOCKS];
	SvrCVData_t SvrCV[MAXCV];
	SvrMVData_t SvrMV[MAXMV];
	int WaitingLockInCV[MAXCV];
	List *ServerCVWaitQ[MAXCV];
	List *SvrLockWaitQ[MAXLOCKS];
	Table *RegTable=new Table(100);
	int ClientID[100];
	int ThreadID[100];
	int i=0;
	char *buffer=new char[40];
	int NoOfUpComingMsg=0;
	char *msg=new char[5];
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *ServerRespbuffer=new char[40];
	char *ntRespBuffer=new char[40];
	int myMailBoxNo=0;
	LastTimeStampTable_t LastTimeStampTable[100];
	List *msgQ;
	msgQ=new List();
	int index=0;
	int TotalNWThreads=0;
	int Counter=0;
	//Project 4 code Start
	initialize(SvrLocks,SvrCV,SvrMV, WaitingLockInCV, ServerCVWaitQ, SvrLockWaitQ,&SrvLockCounter, &SrvCVCounter,&SrvMVCounter);
	//Project 4 code end	    
	ntThreadLock->Acquire();
	for (int a=0;a<100;a++)
	{
		ClientID[a]=0;
		ThreadID[a]=0;
		LastTimeStampTable[a].TimeStamp=0;
	}
	ntThreadLock->Release();
	outPktHdr.to = 0;
	outMailHdr.to = 0;
	outMailHdr.from=myMailBoxNo=ntThreadMailboxID;
	msg="1";
	outMailHdr.length=strlen(msg)+1;
	DEBUG('q',"Sending \"%s\" to %d\n",msg,outPktHdr.to);
	DEBUG('q', "Network Thread ID %d of client ID %d registration\n", outMailHdr.from,netname);
	if(!postOffice->Send(outPktHdr,outMailHdr,msg)){
		printf("networkThread registration Send failed!\n");
	}

	DEBUG('q',"%d mailbox Waiting for message from GroupServer\n",myMailBoxNo);
	postOffice->Receive((int)ntThreadMailboxID, &inPktHdr, &inMailHdr, &buffer[0]);
	DEBUG('q',"Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from); 
	fflush(stdout);
	NoOfUpComingMsg=atoi(buffer);	
	DEBUG('q',"NoOfUpComingMsg = %d \n",NoOfUpComingMsg);
	//delete []buffer;
	for(int k=0;k<NoOfUpComingMsg;k++)
	{
		for(int ic=0;ic<40;ic++)
		{
			ServerRespbuffer[ic]='\0';
		}
		DEBUG('q',"[inside] %d mailbox Waiting for message from GroupServer\n",myMailBoxNo);
		postOffice->Receive(myMailBoxNo, &inPktHdr, &inMailHdr, ServerRespbuffer);
		DEBUG('q',"[inside]Got \"%s\" from %d, box %d\n",ServerRespbuffer,inPktHdr.from,inMailHdr.from); 
		fflush(stdout);
		//Write a logic to decode the 'ServerRespbuffer' and get the list of Client ID and MailBoxID
		
		char c;
		DEBUG('q',"Network thread started to decode the message %s\n",ServerRespbuffer);
		int n=0;
		char *cltID=strtok(ServerRespbuffer,";"); 
		while(index<1000)
		{
			if(cltID!=NULL)
			ClientID[index]=atoi(cltID);
			if(ClientID[index]==0)
			{
				break;
			}
			DEBUG('q', "the client id ClientID[%d] is %d \n",index,ClientID[index]);
			char *cThreadID=strtok(NULL,";");
			if(cThreadID!=NULL)
			ThreadID[index]=atoi(cThreadID);
			DEBUG('q', "the thread id ThreadID[%d] is %d \n",index,ThreadID[index]);
			index++; 
			cltID=strtok(NULL,";");
		}
	}
	//delete []ServerRespbuffer;
	TotalNWThreads=index;
	DEBUG('q',"TotalNWThreads----------------------- %d\n",TotalNWThreads); 
	 
	//Create table for future use
	for(int m=0;m<TotalNWThreads;m++)
	{
		char *buff=new char[40];
		sprintf(buff,"%d%d",ClientID[m],ThreadID[m]);
		RegTable->Put(buff);
		//delete []buff;
	}
	
	//Send message to StartSimulation function
	outPktHdr.to = netname;
	outMailHdr.to = (ntThreadMailboxID-1);	
	msg="1";
	outMailHdr.length=strlen(msg)+1;
	outMailHdr.from = ntThreadMailboxID;
	DEBUG('q',"Sending \"%s\" to my user prg thread mail box at %d\n",msg,outPktHdr.to);
	DEBUG('q', "****Group formation complete******\n");
	if(!postOffice->Send(outPktHdr,outMailHdr,msg)){
		printf("networkThread StartSimulation Send failed!\n");
	}
	fflush(stdout);
		//delete []msg;
	//Wait on the userprog client machines for recieve
	while (true)
 	{ 
		for(int i=0;i<100;i++)
		{
			if(LastTimeStampTable[i].TimeStamp!=0)
			DEBUG('q',"LastTimeStampTable[%d].TimeStamp=%d\n",i,LastTimeStampTable[i].TimeStamp); 
		}
		DEBUG('q',"..ntThread %d of %d client Waiting for message to address from other networking threads\n",ntThreadMailboxID,netname);
		postOffice->Receive(ntThreadMailboxID, &inPktHdr, &inMailHdr, &ntRespBuffer[0]);
		DEBUG('q',"Got \"%s\" from %d, box %d\n",ntRespBuffer,inPktHdr.from,inMailHdr.from); 
		int ClientNo=inPktHdr.from;
		int MailBoxNo=inMailHdr.from;
		if(ClientNo==netname && MailBoxNo==(ntThreadMailboxID-1))
		{
			//Pack the message
			//Get the request from User Prog thread
			//ntRespBuffer will contain request type
			char *cMyTimeStamp=new char[40];
			unsigned long int myTimeStamp;
			DEBUG('q',"ntThread %d got message from its own upThread %d\n", ntThreadMailboxID,MailBoxNo);
			//message = ReqType+other parameters aiding ReqType+TimeStamp
			//STAMP the message
			myTimeStamp=getTheTimeStamp();
			//DEBUG('q',"Current time is %ld\n", myTimeStamp);
			sprintf(cMyTimeStamp,"%ld;",myTimeStamp);
			strcat(cMyTimeStamp,ntRespBuffer);
			DEBUG('q',"%s msg is sent to %d ntThreads\n",cMyTimeStamp, TotalNWThreads);
			//for each ntThreadSend send this message!
			for(int m=0;m<TotalNWThreads;m++)
			{
				outPktHdr.to = ClientID[m];
				outMailHdr.to = ThreadID[m];
				outMailHdr.length=strlen(cMyTimeStamp)+1;
				outMailHdr.from = ntThreadMailboxID;
				//DEBUG('q',"Sending \"%s\" to %d client ID and %d mailbox\n",cMyTimeStamp,ClientID[m],ThreadID[m]);
				if(!postOffice->Send(outPktHdr,outMailHdr,cMyTimeStamp)){
					printf("NetworkThread %d of ClientID %d Send failed!\n",ThreadID[m],ClientID[m]);
				}
			}
			//delete []cMyTimeStamp;
		}
		
		// This message is from newtowrking threads including my NT thread itself!
		else
		{
			char *otherParam=new char[40];
			//char *cTS=NULL;
			unsigned long int myTimeStamp=0;
			unsigned long int TimeStamp=0;
			char *myntBuff=new char[40];
			char *tempNtBuff=new char[40];
			char *tempTS=new char[40];
			DEBUG('q',"ntThread %d got message from other newtowrking threads %d\n", ntThreadMailboxID,MailBoxNo);
			//
			//message = t+TimeStamp
			//Copy the input message to temp buff
			strcpy(tempNtBuff,ntRespBuffer);
			//get the first message in packet data
			char *cTS=strtok(tempNtBuff,";");
			//DEBUG('q',"cTS =..........................................................%s\n",cTS);
			//Maintain the heartbeat if this is not a 'heartbeat' message itself
			// if cTS=t, it is a heart beat
			if(strcmp(cTS,"t")!=0)
			{
				// If the message is not heartbeat, send the heartbeat again and process any message!
				myTimeStamp=getTheTimeStamp();
				char t='t';
				//DEBUG('q',"Current time is %ld\n", myTimeStamp);
				//message = t+TimeStamp
				sprintf(tempTS,"%c;%ld;",t,myTimeStamp);
				DEBUG('q',"%s msg is sent to %d ntThreads\n",tempTS, TotalNWThreads);
				//for each ntThreadSend send this message!
				//printf("Before TS send currentMailBoxID=%d\n", ntThreadMailboxID);
				int myMB=ntThreadMailboxID;
				for(int m=0;m<TotalNWThreads;m++)
				{
					outPktHdr.to = ClientID[m];
					outMailHdr.to = ThreadID[m];
					outMailHdr.length=strlen(tempTS)+1;
					outMailHdr.from =myMB;
					DEBUG('q',"Sending1600 \"%s\" to %d client ID and %d mailbox\n",tempTS,ClientID[m],ThreadID[m]);
					if(!postOffice->Send(outPktHdr,outMailHdr,tempTS)){
						printf("NetworkThread %d of ClientID %d Send failed!\n",ThreadID[m],ClientID[m]);
					}

					//printf("In send currentMailBoxID=%d\n", ntThreadMailboxID);
				}
				//printf("After TS send currentMailBoxID=%d\n", ntThreadMailboxID);
				//Here, ClientNo and MailBoxNo are the details of incoming NT thread
				// We are not (to be) aware of incomimg thread's 
				//corresponding UPthread mac n Mailbox deatils

				char delim=';';
				strcpy(myntBuff,ntRespBuffer);
				//extract the Time_STAMP
				cTS=strtok(myntBuff,";");

				//DEBUG('q',"extracted TimeStamp %s\n",cTS);
				TimeStamp=strtol(cTS,NULL,0);
				DEBUG('q',"extracted myTimeStamp %ld\n",TimeStamp); 
				//Update my LastTimeStampTable for the incoming thread
				char *pos=new char[10];
				sprintf(pos,"%d%d",ClientNo,MailBoxNo);
				DEBUG('q',"pos======= %s\n",pos);
				for(int iC=0;iC<TotalNWThreads;iC++)
				{
					char *RegPos=(char*)RegTable->Get(iC);
					DEBUG('q',"RegPos======= %s\n",RegPos);
					if(strcmp(RegPos,pos)==0)
					{
						LastTimeStampTable[iC].TimeStamp=TimeStamp;
						DEBUG('q',"LST update 1..................................&&&&&&&>>>>>>>>>>>>>>>>> %ld\n",LastTimeStampTable[iC].TimeStamp);						
						break;
					}
				}	
				//Insert the msg into my Msg Q in timestamp order
				sprintf(otherParam,"%d;%d;",ClientNo,MailBoxNo);
				//DEBUG('q',"before otherParam=%s \n",otherParam);
				strcat(otherParam,ntRespBuffer);
				//DEBUG('q'," otherParam=%s \n",otherParam);
				DEBUG('q',"insertin %s to the sorted Q\n",otherParam);
				msgQ->SortedInsert1(otherParam, TimeStamp); 
				//delete []otherParam;
			}	
			else if (strcmp(cTS,"t")==0)
			{
				//just update my LastTimeStampTable and get out!
				//get the time stamp from incoming message of format t;timestamp
				//message = t+TimeStamp
				char *currTS=strtok(NULL,";");
				myTimeStamp=strtol(currTS,NULL,0);
				char *pos=new char[10];
				sprintf(pos,"%d%d",ClientNo,MailBoxNo);
				for(int iC=0;iC<TotalNWThreads;iC++)
				{
					char *RegPos=(char*)RegTable->Get(iC);
					if(strcmp(RegPos,pos)==0)
					{
						LastTimeStampTable[iC].TimeStamp=myTimeStamp;
						DEBUG('q',"LST update 2..................................&&&&&&&>>>>>>>>>>>>>>>>> %ld\n",LastTimeStampTable[iC].TimeStamp);						
					}
				}	
			}

			//Get the least time stamp in my table
			unsigned long int minTS=LastTimeStampTable[0].TimeStamp;
			for(int iC=0;iC<TotalNWThreads;iC++)
			{ 
				if(LastTimeStampTable[iC].TimeStamp<minTS)
					minTS=LastTimeStampTable[iC].TimeStamp;
			}

			DEBUG('q',"Minimum TS in LastTimeStampTable is %d\n", minTS);

			//process "ALL" 'myMsg' in timestamp order with timestamp<=myTimeStamp
			//Decode and Process of myMsg
			//Extract the earlier timestamp value from my table
			if(minTS!=0)
			{		
				while(true)
				{				
					//Get the TS in the msgQ
					char *myMsg=new char[40]; 
					char *tempMsg;
					tempMsg=(char*)msgQ->Remove();
					if(tempMsg==NULL)
					break;
					strcpy(myMsg,tempMsg);
					char *currentTS=strtok(tempMsg,";");
					currentTS=strtok(NULL,";");
					currentTS=strtok(NULL,";");
					unsigned long int myTS=strtol(currentTS,NULL,0);
					DEBUG('q',"extracted message from the sorted Q %s\n",myMsg);
					//if the TS in the msg list<= time in  my LST
					if(myTS<=minTS)
					{
						ntThreadLock->Acquire();
							int currentMailBoxID=ntThreadMailboxID;
						//printf("Before decode  message currentMailBoxID=%d\n", currentMailBoxID);
						DEBUG('q',"inside ------------- extracted message from the sorted Q %s and ntThreadMailboxID %d\n",myMsg,ntThreadMailboxID);
						DecodeMsg(myMsg,SvrLocks,SvrCV,SvrMV, WaitingLockInCV, ServerCVWaitQ, SvrLockWaitQ,&SrvLockCounter,&SrvCVCounter,&SrvMVCounter,ntThreadMailboxID);	
						ntThreadLock->Release();
					}
					else 
					{
						msgQ->Append((void*)myMsg);
						DEBUG('q',"Appended message to the Q %s\n",myMsg);
						break; 
						
					} 
					//delete []myMsg;
				} 
			}
				//return;
/* 			delete []tempNtBuff;
			delete []tempTS;
			delete []myntBuff; */
		}
		//End of Total Event ordering!
	} 
}

unsigned long int getTheTimeStamp()
{
	struct timeval tval;  
	struct timezone tzone;  
	struct tm *tm;  
	
	gettimeofday(&tval, NULL);
	return((unsigned long int)(tval.tv_usec + tval.tv_sec*1000000));  
	//return((unsigned long int)((tval.tv_sec - 1290829673) * 16384+(tval.tv_usec/100)));  
}

void DecodeMsg(char *buffer, SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, \
List **ServerCVWaitQ, List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{			

	//Add code to decode the string that is passed from the client
	//Get the syscall type and call that particular syscall
	char *BrokenStr=NULL;
	char *cSCtype=NULL;
	int iSCtype=-1;
	int ClientID=-1;
	int ThreadID=-1;
	char *FirstParam=NULL;//Which could be LockName or ID
	char *SecondParam=NULL;//Which Could be CV name or ID
	char *Temp=NULL;
	int LockID=-1;
	int CvID=-1;
	int MVID=-1;
	int MVValue=-1;
	//Incoming msg structure
	//clientID+MailBoxID+timestamp+req_type+other_parameters
	//Remove TS as it is not required here
	DEBUG('q',"INSIDE DECODE: message to procesed is %s\n",buffer);
   DEBUG('q',"INSIDE DECODE: message to procesed is %s\n",buffer);
	char *cTS=strtok(buffer,";");
	ClientID=atoi(cTS);	
	cTS=strtok(NULL,";");
	ThreadID=atoi(cTS);	
	cSCtype=strtok(NULL,";");
	cSCtype=strtok(NULL,";");
/* 	printf("%s\n",cSCtype);
	FirstParam=strtok(NULL,";");
	SecondParam=strtok(NULL,";");
	Temp=strtok(NULL,";");
	ClientID=atoi(Temp);
	Temp=strtok(NULL,";");
	ThreadID=atoi(Temp);
	printf("%s\n",LockName);
	CVName=strtok(NULL,";"); */

	iSCtype=atoi(cSCtype);
	DEBUG('q',"request type .....................iSCtype %d\n",iSCtype);
	switch(iSCtype)
	{
		case SC_CreateLock:	FirstParam=strtok(NULL,";");
												ServerCreateLock(ClientID,ThreadID,FirstParam,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
												ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
												break;
												
		case SC_Acquire:  	FirstParam=strtok(NULL,";");
												LockID=atoi(FirstParam);
												ServerAcquireLock(ClientID,ThreadID,LockID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
												ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
												break;	
												
		case SC_Release:  	FirstParam=strtok(NULL,";");
												LockID=atoi(FirstParam);
												ServerReleaseLock(ClientID,ThreadID,LockID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
												ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
												break;
												
		case SC_DestroyLock:  FirstParam=strtok(NULL,";");
													LockID=atoi(FirstParam);
													ServerDestroyLock(ClientID,ThreadID,LockID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
													ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
													break;	
													
		case SC_CreateCondition:FirstParam=strtok(NULL,";");
														ServerCreateCondition(ClientID,ThreadID,FirstParam,SvrLocks,SvrCV,SvrMV,\
														WaitingLockInCV,ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
														break;	
														
		case SC_DestroyCondition: FirstParam=strtok(NULL,";");
															CvID=atoi(FirstParam);
															ServerDestroyCondition(ClientID,ThreadID,CvID,SvrLocks,SvrCV,SvrMV,\
															WaitingLockInCV,ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
															break;
															
		case SC_Wait:  	FirstParam=strtok(NULL,";");
										SecondParam=strtok(NULL,";");
										LockID=atoi(FirstParam);
										CvID=atoi(SecondParam);
										ServerWait(ClientID,ThreadID,LockID,CvID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,ServerCVWaitQ,\
										SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
										break;	
		case SC_Signal: FirstParam=strtok(NULL,";");
										SecondParam=strtok(NULL,";");
										LockID=atoi(FirstParam);
										CvID=atoi(SecondParam);
										ServerSignal(ClientID,ThreadID,LockID,CvID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
										ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
										break;		
		case SC_Broadcast:  FirstParam=strtok(NULL,";");
												SecondParam=strtok(NULL,";");
												LockID=atoi(FirstParam);
												CvID=atoi(SecondParam);
												ServerBroadcast(ClientID,ThreadID,LockID,CvID,SvrLocks,SvrCV,SvrMV,\
												WaitingLockInCV,ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
												break;	
		case SC_CreateMV:		FirstParam=strtok(NULL,";");
												ServerCreateMV(ClientID,ThreadID,FirstParam,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
												ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
												break;
		case SC_DestroyMV:	FirstParam=strtok(NULL,";");
												ServerDestroyMV(ClientID,ThreadID,MVID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
												ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
												break;
		case SC_SetMV:		FirstParam=strtok(NULL,";");
											SecondParam=strtok(NULL,";");
											MVID=atoi(FirstParam);
											MVValue=atoi(SecondParam);
											ServerSetMV(ClientID,ThreadID,MVID,MVValue,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,\
											ServerCVWaitQ,SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
											break;
		case SC_GetMV:		FirstParam=strtok(NULL,";");
											MVID=atoi(FirstParam); 
											ServerGetMV(ClientID,ThreadID,MVID,SvrLocks,SvrCV,SvrMV,WaitingLockInCV,ServerCVWaitQ,\
											SvrLockWaitQ,SrvLockCounter, SrvCVCounter, SrvMVCounter,ntThreadMailboxID);
											break;								
		default: printf("Server unable to handle the system calls\n");
						 break;
	}

}

int ServerCreateLock(int ClientID, int ThreadID,char *LockName,SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,\
SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ,int *SrvLockCounter, \
int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	char *SrvBuffer=new char[100];
	//printf("%s\n",LockName);
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	for(int i=0;i<(*SrvLockCounter);i++)
	{
		
		if(SvrLocks[i].LockName!=NULL)
		{
			//printf("ForLoop:SvrLocks[%d].LockName=%s\n",i,SvrLocks[i].LockName);
			if(strcmp(SvrLocks[i].LockName,LockName)==0)
			{
				//printf("%s locks already exists in the server!\n",LockName);
				//Just return the lock ID to the client- No lock creating stuff
				//or throwing error
				sprintf(SrvBuffer,"%d",i);
				SvrLocks[i].UsageCntr++;
				RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
				delete []SrvBuffer;
				return 1;
			}
		}
	}
	if((*SrvLockCounter)<MAXLOCKS)
	{
		SvrLocks[(*SrvLockCounter)].UsageCntr++;
		SvrLocks[(*SrvLockCounter)].Status=FREE;
		strcpy(SvrLocks[(*SrvLockCounter)].LockName,LockName);
		//SvrLocks[SrvLockCounter].LockName=LockName;
		DEBUG('q',"%s lock successfully created!\n",LockName);
		sprintf(SrvBuffer,"%d",(*SrvLockCounter));
		(*SrvLockCounter)++;
	}
	else
	{
		printf("Error:Reached maximum locks!\n");
		//sprintf(SrvBuffer,"Error:Reached maximum locks!");
		sprintf(SrvBuffer,"%d",FAILURE);
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	delete []SrvBuffer;
	return 1;
}


int ServerAcquireLock(int ClientID, int ThreadID,int LockID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	if(LockID<0 || LockID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid Lock ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(isHeldByCurrentThread(LockID,ClientID,ThreadID,SvrLocks)==true)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("Present %d ClientID and %d ThreadID msg processing.!!\n",netname,ntThreadMailboxID);
		printf("ERROR!!%d ClientID and %d ThreadID already own the lock!!\n",ClientID,ThreadID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	} 
	
	if((LockID>=0)&&(LockID<MAXLOCKS))
	{
		if(SvrLocks[LockID].Status!=VOID)
		{
			//If lock is free
			if(SvrLocks[LockID].Status==FREE)
			{
				// make lock busy
				SvrLocks[LockID].Status=BUSY;
				//SvrLocks[LockID].UsageCntr++;
				//making currentThread lock owner
				SvrLocks[LockID].LockOwner=ClientID;
				SvrLocks[LockID].LockThreadID=ThreadID;
				//Pack the message for Client
				sprintf(SrvBuffer,"%d",SUCCESS);
				//sDEBUG('q',SrvBuffer,"LockID %d is successfully acquired",LockID);
				DEBUG('q',"LockID %d is successfully acquired\n",LockID);
				RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			}
			else
			{
				//If not same thread, put the incoming thread on Q
				//SvrLocks[LockID].UsageCntr++;				
				SvrLockWaitQ[LockID]->Append((void *)ClientID);
				SvrLockWaitQ[LockID]->Append((void *)ThreadID);
			}
		}
		else
		{
			ErrorFlag=1;
		}
	}
	else
	{
		ErrorFlag=1;
	}
	if(ErrorFlag)
	{

		ErrorFlag=ErrorFlag-2;//To return -1
		sprintf(SrvBuffer,"%d",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		printf("ERROR!!AcquireLock Failed ErrorID:%d\n",ErrorFlag);
		//printf("Sending \"%s\" to %d\n",SrvBuffer,outMailHdr.from);

	}		
	
	delete []SrvBuffer;
	return ErrorFlag;
}


int ServerReleaseLock(int ClientID, int ThreadID,int LockID,SvrLockData_t *SvrLocks, \
SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, \
List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	int nextMacNo;
	int nextMailBoxNO;

	if(LockID<0 || LockID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid Lock ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(!(isHeldByCurrentThread(LockID,ClientID,ThreadID,SvrLocks)))
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		sprintf(SrvBuffer,"%d",FAILURE);
		//sprintf(SrvBuffer,"ERROR!!Releasing a lock that I do not own!!  Lock ID %d",LockID);
		printf("Present %d ClientID and %d ThreadID msg processing.!!\n",netname,ntThreadMailboxID);
		printf("ERROR!!%d ClientID and %d ThreadID DONOT own the lock!!\n",ClientID,ThreadID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	} 


	if((LockID>=0)&&(LockID<MAXLOCKS))
	{
		if(SvrLocks[LockID].Status!=VOID)
		{
			//make this new mac ID and Mail Box the new owner of the lock
			if(!(SvrLockWaitQ[LockID]->IsEmpty()))
			{
				//sprintf(SrvBuffer,"LockID %d is successfully released in Q",LockID);
				sprintf(SrvBuffer,"%d",SUCCESS);
				DEBUG('q',"LockID %d is successfully released (in Q)\n",LockID);
				nextMacNo=(int)SvrLockWaitQ[LockID]->Remove();
				nextMailBoxNO=(int)SvrLockWaitQ[LockID]->Remove();
				// outPktHdr.to = *nextMacNo;
				// outMailHdr.to = *nextMailBoxNO;
				SvrLocks[LockID].LockOwner=nextMacNo;
				SvrLocks[LockID].LockThreadID=nextMailBoxNO;
				SvrLocks[LockID].Status=BUSY;
				//SvrLocks[LockID].UsageCntr--;					
				RespondToClient(SrvBuffer,nextMacNo,nextMailBoxNO,ntThreadMailboxID);
			}
			else  
			{
				//SvrLocks[LockID].UsageCntr--;
				SvrLocks[LockID].Status=FREE;
				SvrLocks[LockID].LockOwner=-1;
				SvrLocks[LockID].LockThreadID=-1;
				sprintf(SrvBuffer,"%d",SUCCESS);
				//printf("released lock for wait call...........\n");
			  //sprintf(SrvBuffer,"LockID %d is successfully released",LockID);
				DEBUG('q',"LockID %d is successfully released \n",LockID);
				//RespondToClient(SrvBuffer,ClientID,ThreadID);
			}
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		}
		else
		{
			ErrorFlag=1;
		}
	}
	else
	{
		ErrorFlag=1;
	}
	if(ErrorFlag)
	{

		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Release lock failed for Lock ID %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Release lock failed for Lock ID %d",LockID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	}			
	delete []SrvBuffer;
	return ErrorFlag;
}

int ServerDestroyLock(int ClientID, int ThreadID,int LockID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, \
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;

	if(LockID<0 || LockID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid Lock ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if((SvrLocks[LockID].UsageCntr==1) && (SvrLocks[LockID].Status==FREE))
	{
		SvrLocks[LockID].UsageCntr=0;
		SvrLocks[LockID].LockOwner=-1;
		SvrLocks[LockID].LockName=NULL;
		SvrLocks[LockID].Status=VOID;
		//sprintf(SrvBuffer,"LockID %d is successfully destroyed\n",LockID);
		sprintf(SrvBuffer,"%d",SUCCESS);
		DEBUG('q',"LockID %d is successfully destroyed\n",LockID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return SUCCESS;
	}
	
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting to delete BUSY lock:%d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to delete BUSY lock or a lock which is not valid:%d\n",ErrorFlag);
		if(SvrLocks[LockID].Status!=BUSY)
		   SvrLocks[LockID].UsageCntr--;
		   
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
}

/******************************************************************
Create Condition Varaible SERVER STUB
*******************************************************************/
int ServerCreateCondition(int ClientID, int ThreadID,char *CvName,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	char *SrvBuffer=new char[100];
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	//printf("%s\n",CVName);
	for(int i=0;i<(*SrvCVCounter);i++)
	{
		if(SvrCV[i].CVName!=NULL)
		{
			if(strcmp(SvrCV[i].CVName,CvName)==0)
			{
				DEBUG('q',"%s CV already exists in the server!\n",CvName);
				//Just return the lock ID to the client- No lock creating stuff
				//or throwing error
				sprintf(SrvBuffer,"%d",i);
				RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
				SvrCV[i].UsageCntr++;
				delete []SrvBuffer;
				return 1;
			}
		}
	}
	if((*SrvCVCounter)<MAXCV)
	{
		SvrCV[(*SrvCVCounter)].UsageCntr++;
		SvrCV[(*SrvCVCounter)].Status=FREE;
		strcpy(SvrCV[(*SrvCVCounter)].CVName,CvName);
		DEBUG('q',"%s CV successfully created!\n",CvName);
		sprintf(SrvBuffer,"%d",(*SrvCVCounter));
		(*SrvCVCounter)++;
	}
	else
	{
		//sprintf(SrvBuffer,"Error:Reached maximum CV!");
		printf("Error:Reached maximum CV!\n");
		sprintf(SrvBuffer,"%d",FAILURE);
		SrvBuffer="-1";
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	delete []SrvBuffer;
	return 1;
}

/******************************************************************
Destroy Condition Varaible SERVER STUB
*******************************************************************/
int ServerDestroyCondition(int ClientID, int ThreadID,int CvID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;

	if(CvID<0 || CvID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//printf(SrvBuffer,"ERROR!!Invalid CV ID%d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid CV ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if((SvrCV[CvID].UsageCntr==1) && (SvrCV[CvID].Status==FREE))
	{
		SvrCV[CvID].UsageCntr=0;
		SvrCV[CvID].CVName="";
		SvrCV[CvID].Status=VOID;
		SvrCV[CvID].CVOwner=-1;
		DEBUG('q',"Deleted CV ID %d successfully\n",CvID);
		sprintf(SrvBuffer,"%d",SUCCESS);
		//sprintf(SrvBuffer,"%d",FAILURE);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return SUCCESS;
	}
	
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting to delete BUSY CV ID%d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to delete BUSY CV or invalid cv:%d\n",ErrorFlag);
		SvrCV[CvID].UsageCntr--;
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
}
/******************************************************************
 Condition Varaible WAIT SERVER STUB
*******************************************************************/
int ServerWait(int ClientID, int ThreadID,int LockID,int CvID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	int IsLockAcquired=0;
	int IsCVCreated=0;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	int nextClientID;
	int nextThreadID;
	
	if(LockID<0 || LockID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid LOCK %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Waiting on Invalid LOCK %d",LockID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(CvID<0 || CvID>MAXCV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid CV %d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Waiting on Invalid CV %d",CvID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	
	if(isHeldByCurrentThread(LockID,ClientID,ThreadID,SvrLocks)!=true)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR: Cannot go on wait as i do not own the lock the ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	} 
	 //Is this the first thread calling the wait?
	if(WaitingLockInCV[CvID]<0)
	{
		//First thread to call wait; Save the lock
		WaitingLockInCV[CvID]=LockID;
	}
	//Is input conditionLock matches the saved lock?
	if(WaitingLockInCV[CvID]!=LockID)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		sprintf(SrvBuffer,"%d",FAILURE);
		//sprintf(SrvBuffer,"ERROR: In wait,input conditionLock do not matches wirh the saved lock");
		printf("ERROR!!In wait,input conditionLock do not matches wirh the saved lock:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(WaitingLockInCV[CvID]==LockID && SvrCV[CvID].Status!=VOID)
	{
		//Everything is OK to be waiter
    //Add thread to condition wait queue	
		// char *wBuf=new char[100];
		// sprintf(wBuf,"%d;%d;%d;%d",ClientID,ThreadID,LockID,CvID);
		ServerCVWaitQ[CvID]->Append((void *)ClientID);
		ServerCVWaitQ[CvID]->Append((void *)ThreadID);
		ServerCVWaitQ[CvID]->Append((void*)LockID);
		//ServerCVWaitQ[CvID]->Append(CvID);
		//SvrCV[CvID].UsageCntr++;
		DEBUG('q',"Trying to release Releasing the lock %d in wait\n",LockID);
		if(!(SvrLockWaitQ[LockID]->IsEmpty()))
		{
			sprintf(SrvBuffer,"%d",SUCCESS);
			DEBUG('q',"LockID %d is successfully released",LockID);
			nextClientID=(int)SvrLockWaitQ[LockID]->Remove();
			nextThreadID=(int)SvrLockWaitQ[LockID]->Remove();
			// outPktHdr.to = nextClientID;
			// outMailHdr.to = nextThreadID;
			SvrLocks[LockID].LockOwner=nextClientID;
			SvrLocks[LockID].LockThreadID=nextThreadID;
			SvrLocks[LockID].Status=BUSY;
			DEBUG('q',"LockID %d is Acquired in WAIT by cl:%d MB%d\n",LockID,ClientID,ThreadID);
			//SvrLocks[LockID].UsageCntr--;					
			RespondToClient(SrvBuffer,nextClientID,nextThreadID,ntThreadMailboxID);
		}
		else
		{
			//SvrLocks[LockID].UsageCntr--;
			DEBUG('q',"Releasing the lock %d in wait\n",LockID);
			SvrLocks[LockID].Status=FREE;
			SvrLocks[LockID].LockOwner=-1;
			SvrLocks[LockID].LockThreadID=-1;
			SvrCV[CvID].Status=BUSY;
			//printf("Temporarily LockID %d is  released in wait...............",LockID);
		}
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		sprintf(SrvBuffer,"%d",FAILURE);
		//sprintf(SrvBuffer,"ERROR!!Attempting wait on invalid lock or CV");
		printf("ERROR!!Attempting wait on invalid lock or CV ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	}
	////////////
	
	delete []SrvBuffer;
	return ErrorFlag;
}



/******************************************************************
 Condition Varaible Signal SERVER STUB
*******************************************************************/
int ServerSignal(int ClientID, int ThreadID,int LockID,int CvID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	int IsLockAcquired=0;
	int IsCVCreated=0;
	int nextClientID;
	int nextThreadID;
	// outPktHdr.to = inPktHdr.from;
	// outMailHdr.to = inMailHdr.from;
	
	if(LockID<0 || LockID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid LOCK %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting wait on invalid LOCK %d",LockID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(CvID<0 || CvID>MAXCV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid CV %d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting wait on invalid %d",CvID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	
	//Is input conditionLock matches the saved lock?
	if(WaitingLockInCV[CvID]!=LockID)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR: In Signal,input conditionLock do not matches wirh the saved lock");
		sprintf(SrvBuffer,"%d",FAILURE);
		//printf("ERROR: In Signal,input conditionLock do not matches wirh the saved lock  ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(ServerCVWaitQ[CvID]->IsEmpty())
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"No Client is waiting to signal!");
		sprintf(SrvBuffer,"%d",FAILURE);
		/*printf("ERROR!!No Client is waiting to signal ErrorID:%d\n",ErrorFlag);*/
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(WaitingLockInCV[CvID]==LockID && SvrCV[CvID].Status!=VOID)
	{
    //We know there are threads waiting in the queue
    //Wakeup one waiter
    //Remove one thread from Condition Wait queue
		int nextLockID;
		nextClientID=(int)ServerCVWaitQ[CvID]->Remove();
		nextThreadID=(int)ServerCVWaitQ[CvID]->Remove();
		nextLockID=(int)ServerCVWaitQ[CvID]->Remove();
		//SvrCV[CvID].UsageCntr--;
		if(isHeldByCurrentThread(nextLockID,nextClientID,nextThreadID,SvrLocks)==true)
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
			sprintf(SrvBuffer,"%d",FAILURE);
			printf("ERROR!!I already own the lock!! ErrorID:%d\n",ErrorFlag);
			RespondToClient(SrvBuffer,nextClientID,nextThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return ErrorFlag;
		} 
		if(SvrLocks[nextLockID].Status!=VOID)
		{
			SvrCV[CvID].Status=FREE;
			//If lock is free
			if(SvrLocks[nextLockID].Status==FREE)
			{
				// make lock busy
				SvrLocks[nextLockID].Status=BUSY;
				//SvrLocks[nextLockID].UsageCntr++;
				//making currentThread lock owner
				SvrLocks[nextLockID].LockOwner=nextClientID;
				SvrLocks[nextLockID].LockThreadID=nextThreadID;
				//Pack the message for Client
				//sprintf(SrvBuffer,"LockID %d is successfully acquired",LockID);
				sprintf(SrvBuffer,"%d",SUCCESS);
				RespondToClient(SrvBuffer,nextClientID,nextThreadID,ntThreadMailboxID);
				//printf("In signal...sending response to %d\n",nextClientID);
			}
			else
			{
				//If not same thread, put the incoming thread on Q
				//SvrLocks[nextLockID].UsageCntr++;				
				SvrLockWaitQ[nextLockID]->Append((void *)nextClientID);
				SvrLockWaitQ[nextLockID]->Append((void *)nextThreadID);
				//printf("In signal...HOLDING response to %d\n",nextClientID);
			//	RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			}
		}
	/* 	if(ServerCVWaitQ[CvID]->IsEmpty())
		{
			ServerCVWaitQ[CvID]=NULL;			
		} */
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting Signal on invalid CV or Lock ID!");
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting wait on invalid CV ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	//printf(" in signal............\n");
	RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	delete []SrvBuffer;
	return ErrorFlag;	
}

/******************************************************************
 Condition Varaible Broadcast SERVER STUB
*******************************************************************/
int ServerBroadcast(int ClientID, int ThreadID,int LockID,int CvID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, \
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	// while there are threads, wake them up!
	//Leave all validation to sognal
	//Good Luck Signal - Make me proud!!
		int ErrorFlag=0;
		int iSuccess=0;
		char *SrvBuffer=new char[100];
		int LockOwnerID;
		int IsLockAcquired=0;
		int IsCVCreated=0;
		int nextClientID;
		int nextThreadID;
	
		if(LockID<0 || LockID>MAXLOCKS)
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid LOCK %d",LockID);
			sprintf(SrvBuffer,"%d",FAILURE);
			printf("ERROR!!Attempting wait on invalid LOCK %d",LockID);
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return ErrorFlag;
		}
		
		if(CvID<0 || CvID>MAXCV)
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid CV %d",CvID);
			sprintf(SrvBuffer,"%d",FAILURE);
			printf("ERROR!!Attempting wait on invalid %d",CvID);
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return ErrorFlag;
		}
 	// char *SrvBuffer1=new char[100];
  // sprintf(SrvBuffer1,"%d",SUCCESS);
	// RespondToClient(SrvBuffer1,ClientID,ThreadID,ntThreadMailboxID);

	
	while(!ServerCVWaitQ[CvID]->IsEmpty())
	{
		printf("Processing broadcast+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
		//Is input conditionLock matches the saved lock?
		if(WaitingLockInCV[CvID]!=LockID)
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"ERROR: In Signal,input conditionLock do not matches wirh the saved lock");
			sprintf(SrvBuffer,"%d",FAILURE);
			printf("ERROR: In Signal,input conditionLock do not matches wirh the saved lock  ErrorID:%d\n",ErrorFlag);
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return ErrorFlag;
		}
		if(ServerCVWaitQ[CvID]->IsEmpty())
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"No Client is waiting to signal!");
			sprintf(SrvBuffer,"%d",FAILURE);
			/*printf("ERROR!!No Client is waiting to signal ErrorID:%d\n",ErrorFlag);*/
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return ErrorFlag;
		}
		if(WaitingLockInCV[CvID]==LockID && SvrCV[CvID].Status!=VOID)
		{
			//We know there are threads waiting in the queue
			//Wakeup one waiter
			//Remove one thread from Condition Wait queue
			int nextLockID;
			nextClientID=(int)ServerCVWaitQ[CvID]->Remove();
			nextThreadID=(int)ServerCVWaitQ[CvID]->Remove();
			nextLockID=(int)ServerCVWaitQ[CvID]->Remove();
			//SvrCV[CvID].UsageCntr--;
			if(isHeldByCurrentThread(nextLockID,nextClientID,nextThreadID,SvrLocks)==true)
			{
				ErrorFlag=ErrorFlag-2;//To return -1
				//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
				sprintf(SrvBuffer,"%d",FAILURE);
				printf("ERROR!!I already own the lock!! ErrorID:%d\n",ErrorFlag);
				RespondToClient(SrvBuffer,nextClientID,nextThreadID,ntThreadMailboxID);
				delete []SrvBuffer;
				return ErrorFlag;
			} 
			if(SvrLocks[nextLockID].Status!=VOID)
			{
				SvrCV[CvID].Status=FREE;
				//If lock is free
				if(SvrLocks[nextLockID].Status==FREE)
				{
					// make lock busy
					SvrLocks[nextLockID].Status=BUSY;
					//SvrLocks[nextLockID].UsageCntr++;
					//making currentThread lock owner
					SvrLocks[nextLockID].LockOwner=nextClientID;
					SvrLocks[nextLockID].LockThreadID=nextThreadID;
					//Pack the message for Client
					//sprintf(SrvBuffer,"LockID %d is successfully acquired",LockID);
					sprintf(SrvBuffer,"%d",SUCCESS);
					RespondToClient(SrvBuffer,nextClientID,nextThreadID,ntThreadMailboxID);
				
				}
				else
				{
					//If not same thread, put the incoming thread on Q
					//SvrLocks[nextLockID].UsageCntr++;				
					SvrLockWaitQ[nextLockID]->Append((void *)nextClientID);
					SvrLockWaitQ[nextLockID]->Append((void *)nextThreadID);
					
					RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
				}
			}
			if(ServerCVWaitQ[CvID]->IsEmpty())
			{
				ServerCVWaitQ[CvID]=NULL;
				break;
			}
		}
		else
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"ERROR!!Attempting Signal on invalid CV or Lock ID!");
			sprintf(SrvBuffer,"%d",FAILURE);
			printf("ERROR!!Attempting wait on invalid CV ErrorID:%d\n",ErrorFlag);
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return ErrorFlag;
		}
		//RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	return 1;
}



bool isHeldByCurrentThread(int LockID,int MacID,int MailBoxID,SvrLockData_t *SvrLocks)
{
  bool status=false;
	if(SvrLocks[LockID].LockOwner==MacID && SvrLocks[LockID].LockThreadID==MailBoxID)
		status=true;
	return status;
}

void RespondToClient(char *SrvBuffer, int MacID, int MailBoxID,int ntThreadMailboxID)
{
	//Code added to project 4- START
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	int currentClientID=netname;
	int currentMailBoxID=ntThreadMailboxID;
	DEBUG('q',"RespondToClient currentMailBoxID=%d\n", currentMailBoxID);
	int myUsrPrgMailBoxID=currentMailBoxID-1;

	if(currentClientID==MacID && currentMailBoxID==MailBoxID)
	{
		outPktHdr.to=MacID;
		outMailHdr.to=myUsrPrgMailBoxID; 
		outMailHdr.from=currentMailBoxID;
		int PacketLen=strlen(SrvBuffer)+1;
		outMailHdr.length=PacketLen;
		DEBUG('q',"Sending \"%s\" to %d\n",SrvBuffer,MacID);
		if(!postOffice->Send(outPktHdr,outMailHdr,SrvBuffer)){
			printf("ServerAcquireLock Send failed!\n");
		}
		fflush(stdout);
	}
	else
	{
		//Donothing 
		//Assume all table data is updated before I'm called.
	}
	//Code added to project 4- END

}

int ServerCreateMV(int ClientID, int ThreadID,char *MVName,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, 
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	char *SrvBuffer=new char[100];
	//printf("%s\n",LockName);
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	for(int i=0;i<(*SrvMVCounter);i++)
	{
		//printf("ForLoop:SvrLocks[%d].LockName=%s\n",i,SvrLocks[i].LockName);
		//if(SvrLocks[i].LockName)
		if(strcmp(SvrMV[i].MVName,MVName)==0)
		{
			DEBUG('q',"%s MV already exists in the server!\n",MVName);
			//Just return the lock ID to the client- No lock creating stuff
			//or throwing error
			sprintf(SrvBuffer,"%d",i);
			SvrMV[i].UsageCntr++;
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return (*SrvMVCounter);
		}
	}
	if((*SrvMVCounter)<MAXMV)
	{
		SvrMV[(*SrvMVCounter)].UsageCntr++;
		//printf(".....Usuage Counter =%d\n",SvrMV[SrvMVCounter].UsageCntr);
		SvrMV[(*SrvMVCounter)].Status=ACTIVE;
		strcpy(SvrMV[(*SrvMVCounter)].MVName,MVName);
		SvrMV[(*SrvMVCounter)].MVValue=0;
		SvrMV[(*SrvMVCounter)].MVOwner=ClientID;
		SvrMV[(*SrvMVCounter)].MVThreadID=ThreadID;
		DEBUG('q',"%s MV successfully created!\n",MVName);
		sprintf(SrvBuffer,"%d",(*SrvMVCounter));
		(*SrvMVCounter)++;
	}
	else
	{
		printf("Error:Reached maximum MV!\n");
		//sprintf(SrvBuffer,"Error:Reached maximum MV!");
		sprintf(SrvBuffer,"%d",FAILURE);
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
	delete []SrvBuffer;
	return (*SrvMVCounter);
}

int ServerDestroyMV(int ClientID, int ThreadID,int MVID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int MVOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;

	if( MVID>MAXMV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MV ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(SvrMV[MVID].Status==VOID)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to delete non-existant MVID:%d\n",MVID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(SvrMV[MVID].UsageCntr==1)
	{
		SvrMV[MVID].UsageCntr=0;
		SvrMV[MVID].MVOwner=-1;
		SvrMV[MVID].MVThreadID=-1;
		SvrMV[MVID].MVName="";
		SvrMV[MVID].MVValue=-1;
		SvrMV[MVID].Status=VOID;
		//sprintf(SrvBuffer,"MVID %d is successfully destroyed\n",MVID);
		sprintf(SrvBuffer,"%d",SUCCESS);
		printf("MVID %d is successfully destroyed\n",MVID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return SUCCESS;
	}
	
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting to delete BUSY lock:%d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to delete BUSY MV:%d\n MV UsuageCounter = %d\n",MVID,SvrLocks[MVID].UsageCntr);
		SvrMV[MVID].UsageCntr--;
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}	
}

int ServerSetMV(int ClientID, int ThreadID,int MVID, int MVValue,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	DEBUG('q',"..........Recieved MV value =%d\n",MVValue);

	if(MVID<0 || MVID>MAXMV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(MVValue<0 || MVID>65536)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Too large or less value for MVID :%d\n",MVID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(SvrMV[MVID].Status==VOID)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to Set non-existant MVID:%d\n",MVID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if((MVID>=0)&&(MVID<MAXMV))
	{
		if(SvrMV[MVID].Status!=VOID)
		{
			sprintf(SrvBuffer,"%d",MVValue);
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			SvrMV[MVID].MVValue=MVValue;
			delete []SrvBuffer;
			DEBUG('q',"..........Updated MV value =%d\n",SvrMV[MVID].MVValue);
			return SUCCESS;
		}
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	return ErrorFlag; 
}

int ServerGetMV(int ClientID, int ThreadID,int MVID,SvrLockData_t *SvrLocks,\
 SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ,\
 List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter,int ntThreadMailboxID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];

	if(MVID<0 || MVID>MAXMV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if((MVID>=0)&&(MVID<MAXMV))
	{
		if(SvrMV[MVID].Status!=VOID)
		{
			sprintf(SrvBuffer,"%d",SvrMV[MVID].MVValue);
			//printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
			//printf("In get.....Usuage Counter =%d\n",SvrMV[MVID].UsageCntr);
			RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
			delete []SrvBuffer;
			return SvrMV[MVID].MVValue;
		}
	}
			if(SvrMV[MVID].Status==VOID)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to get non-existant MVID:%d\n",MVID);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID,ntThreadMailboxID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
}

void initialize(SvrLockData_t *SvrLocks, SvrCVData_t *SvrCV,SvrMVData_t  *SvrMV, int *WaitingLockInCV, List **ServerCVWaitQ, List **SvrLockWaitQ,int *SrvLockCounter, int *SrvCVCounter, int *SrvMVCounter)
{
	ntThreadLock=new Lock("ntThreadLock");
	ntThreadLock->Acquire();
	HideMessage=false;
	ReleaseCalledFromWait=-1;
	for(int i=0;i<MAXLOCKS;i++)
	{
		SvrLocks[i].IsOkToDestroy=VOID;
		SvrLocks[i].UsageCntr=0;
		SvrLocks[i].LockName=new char[10];
		SvrLocks[i].LockOwner=-1;
		SvrLocks[i].LockThreadID=-1;
		SvrLocks[i].Status=VOID;
		SvrLockWaitQ[i]=new List;
	}
	for(int i=0;i<MAXCV;i++)
	{
		SvrCV[i].UsageCntr=0;
		SvrCV[i].CVName=new char[10];
		SvrCV[i].CVOwner=-1;
		SvrCV[i].CVThreadID=-1;
		SvrCV[i].Status=VOID;
		ServerCVWaitQ[i]=new List;
		WaitingLockInCV[i]=-1;
	}
	for(int i=0;i<MAXMV;i++)
	{
		SvrMV[i].UsageCntr=0;
		SvrMV[i].IsOkToDestroy=-1;
		SvrMV[i].MVName=new char[10];
		SvrMV[i].MVOwner=-1;
		SvrMV[i].MVThreadID=-1;
		SvrMV[i].Status=VOID;
		SvrMV[i].MVValue=-1;
	}
	ntThreadLock->Release();
}
