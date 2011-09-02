///////////*********************************
//
// main.cc
//  Bootstrap code to initialize the operating system kernel.
//
//  Allows direct calls into internal operating system functions,
//  to simplify debugging and testing.  In practice, the
//  bootstrap code would just initialize data structures,
//  and start a user program to print the login prompt.
//
//  Most of this file is not needed until later assignments.
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//      -s -x <nachos file> -c <consoleIn> <consoleOut>
//      -f -cp <unix file> <nachos file>
//      -p <nachos file> -r <nachos file> -l -D -t
//              -n <network reliability> -m <machine id>
//              -o <other machine id>
//              -z
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//
//  USER_PROGRAM
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -c tests the console
//
//  FILESYS
//    -f causes the physical disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system
//    -t tests the performance of the Nachos file system
//
//  NETWORK
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -o runs a simple test of the Nachos network software
//
//  NOTE -- flags are ignored until the relevant assignment.
//  Some of the flags are interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"
#include "synch.h"
#include <string>
#ifdef NETWORK
#include <time.h>
#include "post.h"
#endif

// External functions used by this file

extern void ThreadTest(void), Copy(char *unixFile, char *nachosFile);
extern void Print(char *file), PerformanceTest(void);
extern void StartProcess(char *file), ConsoleTest(char *in, char *out);
extern void MailTest(int networkID);
#ifdef NETWORK
int GroupServer(int TotalNT);
#endif
//----------------------------------------------------------------------
// main
//  Bootstrap the operating system kernel.
//
//  Check command line arguments
//  Initialize data structures
//  (optionally) Call test procedure
//
//  "argc" is the number of command line arguments (including the name
//      of the command) -- ex: "nachos -d +" -> argc = 3
//  "argv" is an array of strings, one for each command line argument
//      ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int
main(int argc, char **argv)
{
    int argCount;           // the number of arguments
                    // for a particular command

    DEBUG('t', "Entering main");
    (void) Initialize(argc, argv);
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
    argCount = 1;
        if (!strcmp(*argv, "-z"))               // print copyright
            printf (copyright);
								#ifdef THREADS
		#endif
#ifdef USER_PROGRAM

        if (!strcmp(*argv, "-x")) {         // run a user program
        ASSERT(argc > 1);
            StartProcess(*(argv + 1));
            argCount = 2;
        } else if (!strcmp(*argv, "-c")) {      // test the console
        if (argc == 1)
            ConsoleTest(NULL, NULL);
        else {
        ASSERT(argc > 2);
            ConsoleTest(*(argv + 1), *(argv + 2));
            argCount = 3;
        }
        interrupt->Halt();      // once we start the console, then
                    // Nachos will loop forever waiting
                    // for console input
    }
#endif // USER_PROGRAM
#ifdef FILESYS
    if (!strcmp(*argv, "-cp")) {        // copy from UNIX to Nachos
        ASSERT(argc > 2);
        Copy(*(argv + 1), *(argv + 2));
        argCount = 3;
    } else if (!strcmp(*argv, "-p")) {  // print a Nachos file
        ASSERT(argc > 1);
        Print(*(argv + 1));
        argCount = 2;
    } else if (!strcmp(*argv, "-r")) {  // remove Nachos file
        ASSERT(argc > 1);
        fileSystem->Remove(*(argv + 1));
        argCount = 2;
    } else if (!strcmp(*argv, "-l")) {  // list Nachos directory
            fileSystem->List();
    } else if (!strcmp(*argv, "-D")) {  // print entire filesystem
            fileSystem->Print();
    } else if (!strcmp(*argv, "-t")) {  // performance test
            PerformanceTest();
    }
#endif // FILESYS
#ifdef NETWORK
        if (!strcmp(*argv, "-o")) {
	    ASSERT(argc > 1);
            //Delay(2); 				// delay for 2 seconds
						// to give the user time to 
						// start up another nachos
            //MailTest(atoi(*(argv + 1)));
            argCount = 2;
						//printf("Calling Client\n");
        }
				if (!strcmp(*argv, "-gs")) {
					ASSERT(argc > 1);
            Delay(2); 				// delay for 2 seconds
						// to give the user time to 
						// start up another nachos
						int TotalNoNWThreads=atoi(*(argv + 1));
						DEBUG('q',"GroupServer called\n");
						DEBUG('q',"TotalNoNWThreads = %d\n",TotalNoNWThreads);
						GroupServer(TotalNoNWThreads);
            argCount = 2;
        }
#endif // NETWORK
    }

    currentThread->Finish();    // NOTE: if the procedure "main"
                // returns, then the program "nachos"
                // will exit (as any other normal program
                // would).  But there may be other
                // threads on the ready list.  We switch
                // to those threads by saying that the
                // "main" thread is finished, preventing
                // it from returning.
    return(0);          // Not reached...
}

unsigned long int getTimeStamp()
{
	struct timeval tval;  
	struct timezone tzone;  
	struct tm *tm;  
	gettimeofday(&tval, &tzone);  
	tm=localtime(&tval.tv_sec);  
	return((unsigned long int)(tval.tv_usec + tval.tv_sec));   
}
void testSortQ()
{
	List *msgQ=new List();
	//char *msg=new char[40];
	int c=0;
	for(int k=0;k<10;k=k+2)
	{
		char *msg=new char[40];
		sprintf(msg,"SampleCode%ld",getTimeStamp());
		msgQ->SortedInsert(msg,getTimeStamp());
		c++;
	}
	for(int kq=0;kq<c;kq++)
	{		
		//printf(".........%s\n",msgQ->SortedRemove(&k));
		//printf(".........RMV0   %s\n",msgQ->Remove());
		//c--;
	}
	int64_t j=getTimeStamp();
	char *msg1=new char[40];
	sprintf(msg1,"SampleCode%ld",getTimeStamp());
	msgQ->SortedInsert(msg1,getTimeStamp());
	c++;

	// char *tempMsg;
	// tempMsg=(char*)msgQ->Remove();
	// printf(".........RMV   %s\n",tempMsg);
	for(int k=0;k<c;k++)
	{		
		//printf(".........%s\n",msgQ->SortedRemove(&k));
		char *tempMsg;
		tempMsg=(char*)msgQ->Remove();
		printf(".........RMV   %s\n",tempMsg);
	}
	//printf(".........%s\n",msgQ->Remove());
	// msgQ->SortedRemove("SampleCode",j);
	// j=3;
	// msgQ->SortedInsert("SampleCode",j);
	// for(int k=0;k<10;k=k++)
	// {
		//msgQ->SortedReove("SampleCode",k);
	// }

}

#ifdef NETWORK
int GroupServer(int TotalNWthreads)
{
	int ClientID[100];
	int ThreadID[100];
	int i=0;
	char buffer[40];
	int NoOfUpComingMsg=0;
	char *SrvBuffer=new char[100];
	char *msg=new char[100];
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	for (int a=0;a<100;a++)
	{
		ClientID[a]=0;
		ThreadID[a]=0;
	}
	//Wait on the userprog client machines for recieve
	while (true)
	{
		DEBUG('q',"Waiting for message to GroupServer\n");
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		DEBUG('q',"Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from); 
		ClientID[i]=inPktHdr.from;
		ThreadID[i]=inMailHdr.from;
		i++;
		// send the reply to incoming message about number of upcoming messages
		// 5 = 1 byte of Client ID+1 byte of delimeter+2 byte of MailBox ID + 1 byte of delimeter
		NoOfUpComingMsg=divRoundUp((5*TotalNWthreads),40);
		// pack the message to client nw thread ID
		outPktHdr.to = inPktHdr.from;
		outMailHdr.to = inMailHdr.from;
		outMailHdr.from = 0;
		sprintf(SrvBuffer,"%d",NoOfUpComingMsg);
		outMailHdr.length=strlen(SrvBuffer)+1;
		DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
		if(!postOffice->Send(outPktHdr,outMailHdr,SrvBuffer)){
			printf("GroupServer Send failed!\n");
		}
		DEBUG('q',"Value of registered threads ..........%d\n",i);
		if(i==TotalNWthreads)
		{
			// All the clients are registered
			// Send the list of client ID and Thread ID to all the network threads
			// Build the message
			DEBUG('q',"Value of registered threads %d\n",i);
			for(int k=0;k<NoOfUpComingMsg;k++)
			{
				msg=new char[100];
				for(int j=0;j<7;j++)
				{
					if(ClientID[j]!=-1 && ThreadID[j]!=-1)
					{
						char *data=new char[5];
						sprintf(data,"%d;%d;",ClientID[((k*7)+j)],ThreadID[((k*7)+j)]);
						DEBUG('q',"built message = %s \n",data);
						strcat(msg,data);
					}
				}
				DEBUG('q',"packed message = %s \n",msg);
				// Send each message to all networking message!
				for(int m=0;m<TotalNWthreads;m++)
				{
					outPktHdr.to = ClientID[m];
					outMailHdr.to = ThreadID[m];
					outMailHdr.from = 0;
					outMailHdr.length=strlen(msg)+1;
					DEBUG('q',"Sending \"%s\" to %d client ID and %d mailbox\n",msg,ClientID[m],ThreadID[m]);
					if(!postOffice->Send(outPktHdr,outMailHdr,msg)){
						printf("GroupServer Send failed!\n");
					}
				}
			}
			break;
		}
		fflush(stdout);
	}
	printf("GroupServer shutting down....\n");

	interrupt->Halt();
}

#endif


