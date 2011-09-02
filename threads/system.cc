 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *timer;				// the hardware timer device,
					// for invoking context switches

#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers
#endif

#ifdef NETWORK
PostOffice *postOffice;
Lock *mailBoxLock;
int nextMailBoxNo;
int netname;
List *MsgQ;
Timer *timersend;
void resendMsgfn();
void resendTimerfn(int dummy);
unsigned long lastSendTime;
Lock *msgQLock;
bool lostPacket=false;
int extracredit;
int myClientID;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();
#ifdef USER_PROGRAM
Lock *processTableLock = new Lock("processTableLock");
procTbl_t processTable[2000] = {0};
int TotalProcessCount =1;
BitMap *PhyMemBitMap = new BitMap(NumPhysPages);
int TotalThreadCount =0;
int ActiveNoOfProcess=1;
Lock *pageTableLock[MAX_POSSIBLE_LOCKS];
Lock *bitMapLock = new Lock("bitMapLock");
#endif

//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = "";
    bool randomYield = FALSE;
		bool lostPacket =FALSE;
		int NumberOfClients=0;

#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program
	 int i;
	Lock *LockPointer;
	for ( i =0; i< MAX_POSSIBLE_LOCKS; i++)
	{
		LockPointer = new Lock("pageTableLock");
		pageTableLock[i] = LockPointer;
		//printf("%d", i);
	}
#endif
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    netname = 0;		// UNIX socket name
		mailBoxLock=new Lock("mailBoxLock");
		nextMailBoxNo=1;
		MsgQ=new List();    // Global Message Q
		msgQLock=new Lock("msgqlock");
		extracredit=0;
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	}
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
			lostPacket = true;
			extracredit=1;
			printf("Please enter the number of nachos instances required: \n");
			scanf("%d",&NumberOfClients);
	    argCount = 2;
				
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
			myClientID=atoi(*(argv + 1));
	    argCount = 2;
			
	}

#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)		// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);
	
#ifdef NETWORK	

#endif

    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 1024);
		if(lostPacket)
		{
			timersend = new Timer(resendTimerfn,200, lostPacket);
			if(NumberOfClients>2)
			KernelFn2();
			else
			KernelFn();
		}
#endif
}


// Extra credit----------------------------------------------------------------
  
#ifdef NETWORK

void resendTimerfn(int dummy)
{
	struct timeval tv;
  
	gettimeofday(&tv,NULL);

	if((tv.tv_sec-lastSendTime)>3)
	{
		lastSendTime=tv.tv_sec;
		Thread *t=new Thread("Resend Function");
		t->Fork((VoidFunctionPtr)resendMsgfn,NULL);
	}
}

void resendMsgfn()
{
  //printf("inside resend fn\n");
	char *myMsg=new char[10];
	char *TempBuf=new char[10];
	char Delimiter=';';
	int j=0;
	int ClientID;
	int msgNo;
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	List *tempList=new List();
	// Send all the message in the Q
	while(!MsgQ->IsEmpty())
	{
		myMsg=(char*)MsgQ->Remove();
		strcpy(TempBuf,myMsg);
		//printf("Q not Empty MSG %s\n",myMsg);
		tempList->Append((void*)myMsg);
		char *tempMsgno=strtok(TempBuf,";");
		char *tempClientID=strtok(NULL,";");
		msgNo=atoi(tempMsgno);
		ClientID=atoi(tempClientID);
		char *data=new char[40];
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = ClientID;		
		outPktHdr.from= 51;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
		
		
		sprintf(data,"%s%c%d","tmsg",Delimiter,msgNo);
		outMailHdr.length = strlen(data) + 1;
	//	printf("sending %s to %d client %d box\n", data,outPktHdr.to,outMailHdr.from);
		bool success = postOffice->Send(outPktHdr, outMailHdr, data);
		if ( !success ) {
			printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		}		
	}
//	printf("**********************************Q Empty MSG *****************************************\n");
	char *tempMesg=new char[40];
	while(!tempList->IsEmpty())
	{
		tempMesg=(char*)tempList->Remove();
		MsgQ->Append((void*)tempMesg);
	}
	
	delete TempBuf;
	delete tempList;
	//msgQLock->Release();
}
#endif


//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    
    Exit(0);
}