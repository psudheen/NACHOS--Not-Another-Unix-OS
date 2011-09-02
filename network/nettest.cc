// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <stdlib.h>
#include <string.h>


#define MAXLOCKS 256
#define MAXCV 256
#define MAXMV 256
#define SC_Halt		        0
#define SC_Exit		        1
#define SC_Exec	        	2
#define SC_Join		        3
#define SC_Create	        4
#define SC_Open		        5
#define SC_Read		        6
#define SC_Write	        7
#define SC_Close	        8
#define SC_Fork		        9
#define SC_Yield	        10
#define SC_CreateLock       11
#define SC_DestroyLock      12
#define SC_CreateCondition  13
#define SC_DestroyCondition 14
#define SC_Acquire          15
#define SC_Release          16
#define SC_Wait             17
#define SC_Signal           18
#define SC_Broadcast        19
#define SC_Print						20
#define SC_Print1						21
#define SC_Print2						22
#define SC_Print3						23
#define SC_Scan							24
#define SC_CreateMV			25
#define SC_DestroyMV		26
#define SC_SetMV			27
#define SC_GetMV			28

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
void Server(int ServerID);
void DecodeMsg();
int ServerCreateLock(int ClientID, int ThreadID,char *LockName);
int ServerAcquireLock(int ClientID, int ThreadID,int LockID);
int ServerReleaseLock(int ClientID, int ThreadID,int LockID);
int ServerDestroyLock(int ClientID, int ThreadID,int LockID);
int ServerCreateCondition(int ClientID, int ThreadID,char *CvName);
int ServerWait(int ClientID, int ThreadID,int LockID,int CvID);
int ServerDestroyCondition(int ClientID, int ThreadID,int CvID);
int ServerSignal(int ClientID, int ThreadID,int LockID,int CvID);
int ServerBroadcast(int ClientID, int ThreadID,int LockID,int CvID);
int ServerCreateMV(int ClientID, int ThreadID,char *MVName);
int ServerDestroyMV(int ClientID, int ThreadID,int MVID);
int ServerSetMV(int ClientID, int ThreadID,int MVID, int MVValue);
int ServerGetMV(int ClientID, int ThreadID,int MVID);
const int SUCCESS	= 1;
const int FAILURE	=-1;
bool isHeldByCurrentThread(int LockID,int ClientID,int ThreadID);
void RespondToClient(char *SrvBuffer, int ClientID, int ThreadID);



//Global Variables for the server Kernel
PacketHeader outPktHdr, inPktHdr;
MailHeader outMailHdr, inMailHdr;
char *data=new char[100];
char buffer[MaxMailSize];
char MirrorBuffer[MaxMailSize];
static int SrvLockCounter=0;
static int SrvCVCounter=0;
static int SrvMVCounter=0;
static SvrLockData_t SvrLocks[MAXLOCKS];
static SvrCVData_t SvrCV[MAXCV];
static SvrMVData_t SvrMV[MAXMV];
static int WaitingLockInCV[MAXCV];
int SignalledClientID=-1;
bool HideMessage;
int ReleaseCalledFromWait;
List *ServerCVWaitQ[MAXCV];

List *SvrLockWaitQ[MAXLOCKS];




void Server(int dummy)
{
	HideMessage=false;
	ReleaseCalledFromWait=-1;
	for(int i=0;i<MAXLOCKS;i++)
	{
		SvrLocks[i].IsOkToDestroy=VOID;
		SvrLocks[i].UsageCntr=0;
		SvrLocks[i].LockName=new char[100];
		SvrLocks[i].LockOwner=-1;
		SvrLocks[i].LockThreadID=-1;
		SvrLocks[i].Status=VOID;
		SvrLockWaitQ[i]=new List;
	}
	for(int i=0;i<MAXCV;i++)
	{
		SvrCV[i].UsageCntr=0;
		SvrCV[i].CVName=new char[100];
		SvrCV[i].CVOwner=-1;
		SvrCV[i].CVThreadID=-1;
		SvrCV[i].Status=VOID;
		ServerCVWaitQ[i]=new List;
	}
	for(int i=0;i<MAXMV;i++)
	{
		SvrMV[i].UsageCntr=0;
		SvrMV[i].IsOkToDestroy=-1;
		SvrMV[i].MVName=new char[100];
		SvrMV[i].MVOwner=-1;
		SvrMV[i].MVThreadID=-1;
		SvrMV[i].Status=VOID;
		SvrMV[i].MVValue=-1;
	}
	while (true)
	{
		// outPktHdr.to = farAddr;
		// outMailHdr.to = 0;
		//outMailHdr.from = 1;
		// outMailHdr.length = strlen(data)+1;
		printf("Waiting for message to address: %d\n",netname);
		postOffice->Receive(0,&inPktHdr,&inMailHdr,buffer);
		printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from); 
		//MirrorBuffer=buffer;
		fflush(stdout);
		DecodeMsg();
	}

}

void DecodeMsg()
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

	cSCtype=strtok(buffer,";");
	//printf("%s\n",cSCtype);
	// FirstParam=strtok(NULL,";");
	// SecondParam=strtok(NULL,";");
	// Temp=strtok(NULL,";");
	// ClientID=atoi(Temp);
	// Temp=strtok(NULL,";");
	// ThreadID=atoi(Temp);
	//printf("%s\n",LockName);
	//CVName=strtok(NULL,";");

	iSCtype=atoi(cSCtype);
	// printf("iSCtype %d\n",iSCtype);
	switch(iSCtype)
	{
		case SC_CreateLock:		FirstParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
												ServerCreateLock(ClientID,ThreadID,FirstParam);
												break;
		case SC_Acquire:  	FirstParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
												LockID=atoi(FirstParam);
												ServerAcquireLock(ClientID,ThreadID,LockID);
												break;	
		case SC_Release:  	FirstParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
												LockID=atoi(FirstParam);
												ServerReleaseLock(ClientID,ThreadID,LockID);
												break;
		case SC_DestroyLock:  FirstParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
												LockID=atoi(FirstParam);
													ServerDestroyLock(ClientID,ThreadID,LockID);
													break;	
		case SC_CreateCondition:FirstParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
												LockID=atoi(FirstParam);
														ServerCreateCondition(ClientID,ThreadID,FirstParam);
												break;	
		case SC_DestroyCondition:  FirstParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
												LockID=atoi(FirstParam);
													CvID=atoi(FirstParam);
												ServerDestroyCondition(ClientID,ThreadID,CvID);
												break;	
		case SC_Wait:  	FirstParam=strtok(NULL,";");
										SecondParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
										LockID=atoi(FirstParam);
										CvID=atoi(SecondParam);
												ServerWait(ClientID,ThreadID,LockID,CvID);
												break;	
		case SC_Signal:  	FirstParam=strtok(NULL,";");
										SecondParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
										LockID=atoi(FirstParam);
										CvID=atoi(SecondParam);
											ServerSignal(ClientID,ThreadID,LockID,CvID);
												break;		
		case SC_Broadcast:  FirstParam=strtok(NULL,";");
										SecondParam=strtok(NULL,";");
												//Temp=strtok(NULL,";");
												ClientID=inPktHdr.from;
												//Temp=strtok(NULL,";");
												ThreadID=inMailHdr.from;
										LockID=atoi(FirstParam);
										CvID=atoi(SecondParam);
												ServerBroadcast(ClientID,ThreadID,LockID,CvID);
												break;	
		case SC_CreateMV:		FirstParam=strtok(NULL,";");
										//Temp=strtok(NULL,";");
										ClientID=inPktHdr.from;
										//Temp=strtok(NULL,";");
										ThreadID=inMailHdr.from;
										ServerCreateMV(ClientID,ThreadID,FirstParam);
										break;
		case SC_DestroyMV:		FirstParam=strtok(NULL,";");
								//Temp=strtok(NULL,";");
								MVID=atoi(FirstParam);
								ClientID=inPktHdr.from;
								//Temp=strtok(NULL,";");
								ThreadID=inMailHdr.from;
								ServerDestroyMV(ClientID,ThreadID,MVID);
								break;
		case SC_SetMV:		FirstParam=strtok(NULL,";");
							SecondParam=strtok(NULL,";");
								//Temp=strtok(NULL,";");
								MVID=atoi(FirstParam);
								MVValue=atoi(SecondParam);
								//printf("%d	%d\n",MVID,MVValue);
								ClientID=inPktHdr.from;
								//Temp=strtok(NULL,";");
								ThreadID=inMailHdr.from;
								ServerSetMV(ClientID,ThreadID,MVID,MVValue);
								break;
		case SC_GetMV:		FirstParam=strtok(NULL,";");
								//Temp=strtok(NULL,";");
								MVID=atoi(FirstParam); 
								ClientID=inPktHdr.from;
								//Temp=strtok(NULL,";");
								ThreadID=inMailHdr.from;
								ServerGetMV(ClientID,ThreadID,MVID);
break;								
		default: printf("Server unable to handle the system calls\n");
						 break;
	}

}


int ServerCreateLock(int ClientID, int ThreadID,char *LockName)
{
	char *SrvBuffer=new char[100];
	//printf("%s\n",LockName);
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	for(int i=0;i<SrvLockCounter;i++)
	{
		
		if(SvrLocks[i].LockName!=NULL)
		{
			//printf("ForLoop:SvrLocks[%d].LockName=%s\n",i,SvrLocks[i].LockName);
			if(strcmp(SvrLocks[i].LockName,LockName)==0)
			{
				printf("%s locks already exists in the server!\n",LockName);
				//Just return the lock ID to the client- No lock creating stuff
				//or throwing error
				sprintf(SrvBuffer,"%d",i);
				SvrLocks[i].UsageCntr++;
				RespondToClient(SrvBuffer,ClientID,ThreadID);
				delete []SrvBuffer;
				return SrvLockCounter;
			}
		}
	}
	if(SrvLockCounter<MAXLOCKS)
	{
		SvrLocks[SrvLockCounter].UsageCntr++;
		SvrLocks[SrvLockCounter].Status=FREE;
		strcpy(SvrLocks[SrvLockCounter].LockName,LockName);
		//SvrLocks[SrvLockCounter].LockName=LockName;
		printf("%s lock successfully created!\n",LockName);
		sprintf(SrvBuffer,"%d",SrvLockCounter);
		SrvLockCounter++;
	}
	else
	{
		printf("Error:Reached maximum locks!\n");
		//sprintf(SrvBuffer,"Error:Reached maximum locks!");
		sprintf(SrvBuffer,"%d",FAILURE);
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID);
	delete []SrvBuffer;
	return SrvLockCounter;
}


int ServerAcquireLock(int ClientID, int ThreadID,int LockID)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(isHeldByCurrentThread(LockID,ClientID,ThreadID)==true)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!I already own the lock!! ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
				//sprintf(SrvBuffer,"LockID %d is successfully acquired",LockID);
				printf("LockID %d is successfully acquired\n",LockID);
				RespondToClient(SrvBuffer,ClientID,ThreadID);
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
		sprintf(SrvBuffer,"%d %d",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		printf("ERROR!!AcquireLock Failed ErrorID:%d\n",ErrorFlag);
		//printf("Sending \"%s\" to %d\n",SrvBuffer,outMailHdr.from);

	}		
	printf(" in Acquire............\n");
	for (int i=0;i<SrvLockCounter;i++)
	{
		printf("SvrLocks[%d].Status=%d\n",i,SvrLocks[i].Status);
	}	
	delete []SrvBuffer;
	return ErrorFlag;
}


int ServerReleaseLock(int ClientID, int ThreadID,int LockID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int LockOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	int nextMacNo;
	int nextMailBoxNo;

	if(LockID<0 || LockID>MAXLOCKS)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid Lock ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(!(isHeldByCurrentThread(LockID,ClientID,ThreadID)))
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		sprintf(SrvBuffer,"%d",FAILURE);
		//sprintf(SrvBuffer,"ERROR!!Releasing a lock that I do not own!!  Lock ID %d",LockID);
		printf("ERROR!!Releasing a lock that I do not own!! ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
				printf("LockID %d is successfully released (in Q)\n",LockID);
				nextMacNo=(int)SvrLockWaitQ[LockID]->Remove();
				nextMailBoxNo=(int)SvrLockWaitQ[LockID]->Remove();
				// outPktHdr.to = *nextMacNo;
				// outMailHdr.to = *nextMailBoxNo;
				SvrLocks[LockID].LockOwner=nextMacNo;
				SvrLocks[LockID].LockThreadID=nextMailBoxNo;
				SvrLocks[LockID].Status=BUSY;
				//SvrLocks[LockID].UsageCntr--;					
				RespondToClient(SrvBuffer,nextMacNo,nextMailBoxNo);
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
				printf("LockID %d is successfully released (normal)\n",LockID);
				//RespondToClient(SrvBuffer,ClientID,ThreadID);
			}
			RespondToClient(SrvBuffer,ClientID,ThreadID);
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
	}			
	delete []SrvBuffer;
	return ErrorFlag;
}

int ServerDestroyLock(int ClientID, int ThreadID,int LockID)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
		printf("LockID %d is successfully destroyed\n",LockID);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return SUCCESS;
	}
	
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting to delete BUSY lock:%d",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to delete BUSY lock:%d\n",ErrorFlag);
		SvrLocks[LockID].UsageCntr--;
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
}

/******************************************************************
Create Condition Varaible SERVER STUB
*******************************************************************/
int ServerCreateCondition(int ClientID, int ThreadID,char *CvName)
{
	char *SrvBuffer=new char[100];
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	//printf("%s\n",CVName);
	for(int i=0;i<SrvCVCounter;i++)
	{
		if(SvrCV[i].CVName!=NULL)
		{
			if(strcmp(SvrCV[i].CVName,CvName)==0)
			{
				printf("%s CV already exists in the server!\n",CvName);
				//Just return the lock ID to the client- No lock creating stuff
				//or throwing error
				sprintf(SrvBuffer,"%d",i);
				RespondToClient(SrvBuffer,ClientID,ThreadID);
				SvrCV[i].UsageCntr++;
				delete []SrvBuffer;
				return SrvCVCounter;
			}
		}
	}
	if(SrvCVCounter<MAXCV)
	{
		SvrCV[SrvCVCounter].UsageCntr++;
		SvrCV[SrvCVCounter].Status=ACTIVE;
		strcpy(SvrCV[SrvCVCounter].CVName,CvName);
		printf("%s CV successfully created!\n",CvName);
		sprintf(SrvBuffer,"%d",SrvCVCounter);
		SrvCVCounter++;
	}
	else
	{
		//sprintf(SrvBuffer,"Error:Reached maximum CV!");
		printf("Error:Reached maximum CV!\n");
		sprintf(SrvBuffer,"%d",FAILURE);
		SrvBuffer="-1";
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID);
	delete []SrvBuffer;
	return SrvCVCounter;
}

/******************************************************************
Destroy Condition Varaible SERVER STUB
*******************************************************************/
int ServerDestroyCondition(int ClientID, int ThreadID,int CvID)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if((SvrCV[CvID].UsageCntr==1) && (SvrCV[CvID].Status==FREE))
	{
		SvrCV[CvID].UsageCntr=0;
		SvrCV[CvID].CVName="";
		SvrCV[CvID].Status=VOID;
		SvrCV[CvID].CVOwner=-1;
		printf("Deleted CV ID %d successfully\n",CvID);
		sprintf(SrvBuffer,"%d",SUCCESS);
		//sprintf(SrvBuffer,"%d",FAILURE);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return SUCCESS;
	}
	
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting to delete BUSY CV ID%d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting to delete BUSY CV:%d\n",ErrorFlag);
		SvrCV[CvID].UsageCntr--;
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
}
/******************************************************************
 Condition Varaible WAIT SERVER STUB
*******************************************************************/
int ServerWait(int ClientID, int ThreadID,int LockID,int CvID)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(CvID<0 || CvID>MAXCV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid CV %d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Waiting on Invalid CV %d",CvID);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	// if((LockID>=0)&&(LockID<MAXLOCKS))
	// {
		// if(SvrLocks[LockID].LockOwner==inPktHdr.from)
		// {
			// IsLockAcquired=1;
		// }
	// }
	// if(CvID>=0&&CvID<MAXCV)
	// {
		// if(SvrCV[CvID].CVName!=NULL)
		// {
			// IsCVCreated=1;
		// }
	// }
	
	// if((IsLockAcquired!=1)||(IsCVCreated!=1))
	// {
		// ErrorFlag=ErrorFlag-2;//To return -1
		// sprintf(SrvBuffer,"ERROR!!Attempting wait on invalid lock or CV. WAIT FAILED!!");
		// printf("ERROR!!Attempting wait on invalid lock or CV:%d\n",ErrorFlag);
		// RespondToClient(SrvBuffer,ClientID,ThreadID);
		// return ErrorFlag;
	// }
	if(isHeldByCurrentThread(LockID,ClientID,ThreadID)!=true)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR: Cannot go on wait as i do not own the lock the ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(WaitingLockInCV[CvID]==LockID && SvrCV[CvID].Status==ACTIVE)
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
		printf("Releasing the lock %d in wait\n",LockID);
		if(!(SvrLockWaitQ[LockID]->IsEmpty()))
		{
			//sprintf(SrvBuffer,"LockID %d is successfully released",LockID);
			sprintf(SrvBuffer,"%d",SUCCESS);
			printf("LockID %d is successfully released",LockID);
			nextClientID=(int)SvrLockWaitQ[LockID]->Remove();
			nextThreadID=(int)SvrLockWaitQ[LockID]->Remove();
			// outPktHdr.to = nextClientID;
			// outMailHdr.to = nextThreadID;
			SvrLocks[LockID].LockOwner=nextClientID;
			SvrLocks[LockID].LockThreadID=nextThreadID;
			SvrLocks[LockID].Status=BUSY;
			//SvrLocks[LockID].UsageCntr--;					
			RespondToClient(SrvBuffer,ClientID,ThreadID);
		}
		else
		{
			//SvrLocks[LockID].UsageCntr--;
			SvrLocks[LockID].Status=FREE;
			SvrLocks[LockID].LockOwner=-1;
			SvrLocks[LockID].LockThreadID=-1;
			SvrCV[CvID].Status=BUSY;
			printf("Temporarily LockID %d is  released in wait...............",LockID);
		}
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		sprintf(SrvBuffer,"%d",FAILURE);
		//sprintf(SrvBuffer,"ERROR!!Attempting wait on invalid lock or CV");
		printf("ERROR!!Attempting wait on invalid lock or CV ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
	}
	////////////
	printf(" in WAIT............\n");
	for (int i=0;i<SrvLockCounter;i++)
	{
		printf("SvrLocks[%d].Status=%d\n",i,SvrLocks[i].Status);
	}
	delete []SrvBuffer;
	return ErrorFlag;
}



/******************************************************************
 Condition Varaible Signal SERVER STUB
*******************************************************************/
int ServerSignal(int ClientID, int ThreadID,int LockID,int CvID)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	if(CvID<0 || CvID>MAXCV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Waiting on Invalid CV %d",CvID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting wait on invalid %d",CvID);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	
	// if((LockID>=0)&&(LockID<MAXLOCKS))
	// {
		//check if the lock is actually created
		// for(int i=0;i<MAXLOCKS;i++)
		// {
			// if(SvrLocks[i].LockOwner==ClientID)
			// {
				// IsLockAcquired=1;
				// break;
			// }
		// }
	// }
	// if(CvID>=0&&CvID<MAXCV)
	// {
		//check if the CV is actually created
		// for(int i=0;i<MAXCV;i++)
		// {
			// if(SvrCV[i].CVName!=NULL)
			// {
				// IsCVCreated=1;
				// break;
			// }
		// }
	// }
	
	// if((IsLockAcquired!=1)||(IsCVCreated!=1))
	// {
		// ErrorFlag=ErrorFlag-2;//To return -1
		// sprintf(SrvBuffer,"ERROR!!Attempting wait on invalid lock or CV. WAIT FAILED!!");
		// printf("ERROR!!Attempting wait on invalid lock or CV ErrorID:%d\n",ErrorFlag);
		// RespondToClient(SrvBuffer,ClientID,ThreadID);
		// return ErrorFlag;
	// }
	//Is input conditionLock matches the saved lock?
	if(WaitingLockInCV[CvID]!=LockID)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR: In Signal,input conditionLock do not matches wirh the saved lock");
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR: In Signal,input conditionLock do not matches wirh the saved lock  ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(ServerCVWaitQ[CvID]->IsEmpty())
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"No Client is waiting to signal!");
		sprintf(SrvBuffer,"%d",FAILURE);
		/*printf("ERROR!!No Client is waiting to signal ErrorID:%d\n",ErrorFlag);*/
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(WaitingLockInCV[CvID]==LockID && SvrCV[CvID].Status==ACTIVE)
	{
    //We know there are threads waiting in the queue
    //Wakeup one waiter
    //Remove one thread from Condition Wait queue
		int nextLockID;
		nextClientID=(int)ServerCVWaitQ[CvID]->Remove();
		nextThreadID=(int)ServerCVWaitQ[CvID]->Remove();
		nextLockID=(int)ServerCVWaitQ[CvID]->Remove();
		//SvrCV[CvID].UsageCntr--;
		if(isHeldByCurrentThread(nextLockID,nextClientID,nextThreadID)==true)
		{
			ErrorFlag=ErrorFlag-2;//To return -1
			//sprintf(SrvBuffer,"ERROR!! Lock ID %d already owns the lock",LockID);
			sprintf(SrvBuffer,"%d",FAILURE);
			printf("ERROR!!I already own the lock!! ErrorID:%d\n",ErrorFlag);
			RespondToClient(SrvBuffer,nextClientID,nextThreadID);
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
				RespondToClient(SrvBuffer,nextClientID,nextThreadID);
				printf("In signal...sending response to %d\n",nextClientID);
			}
			else
			{
				//If not same thread, put the incoming thread on Q
				//SvrLocks[nextLockID].UsageCntr++;				
				SvrLockWaitQ[nextLockID]->Append((void *)nextClientID);
				SvrLockWaitQ[nextLockID]->Append((void *)nextThreadID);
				printf("In signal...HOLDING response to %d\n",nextClientID);
				RespondToClient(SrvBuffer,ClientID,ThreadID);
			}
		}
		if(ServerCVWaitQ[CvID]->IsEmpty())
		{
			ServerCVWaitQ[CvID]=NULL;
		}
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Attempting Signal on invalid CV or Lock ID!");
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Attempting wait on invalid CV ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	printf(" in signal............\n");
	for (int i=0;i<SrvLockCounter;i++)
	{
		printf("SvrLocks[%d].Status=%d\n",i,SvrLocks[i].Status);
	}
	delete []SrvBuffer;
	return ErrorFlag;	
}

/******************************************************************
 Condition Varaible Broadcast SERVER STUB
*******************************************************************/
int ServerBroadcast(int ClientID, int ThreadID,int LockID,int CvID)
{
	// while there are threads, wake them up!
	//Leave all validation to sognal
	//Good Luck Signal - Make me proud!! 
	while(!ServerCVWaitQ[CvID]->IsEmpty())
	{
		ServerSignal(ClientID,ThreadID,LockID,CvID);
	}
}



bool isHeldByCurrentThread(int LockID,int MacID,int MailBoxID)
{
  bool status=false;
	if(SvrLocks[LockID].LockOwner==MacID && SvrLocks[LockID].LockThreadID==MailBoxID)
		status=true;
	return status;
}

void RespondToClient(char *SrvBuffer, int MacID, int MailBoxID)
{
	outPktHdr.to=MacID;
	outMailHdr.to=MailBoxID;
	int PacketLen=strlen(SrvBuffer)+1;
	outMailHdr.length=PacketLen;
	printf("Sending \"%s\" to %d\n",SrvBuffer,MacID);
	if(!postOffice->Send(outPktHdr,outMailHdr,SrvBuffer)){
		printf("ServerAcquireLock Send failed!\n");
	}
	fflush(stdout);
}

int ServerCreateMV(int ClientID, int ThreadID,char *MVName)
{
	char *SrvBuffer=new char[100];
	//printf("%s\n",LockName);
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;
	for(int i=0;i<SrvMVCounter;i++)
	{
		//printf("ForLoop:SvrLocks[%d].LockName=%s\n",i,SvrLocks[i].LockName);
		//if(SvrLocks[i].LockName)
		if(strcmp(SvrMV[i].MVName,MVName)==0)
		{
			printf("%s MV already exists in the server!\n",MVName);
			//Just return the lock ID to the client- No lock creating stuff
			//or throwing error
			sprintf(SrvBuffer,"%d",i);
			SvrMV[i].UsageCntr++;
			RespondToClient(SrvBuffer,ClientID,ThreadID);
			delete []SrvBuffer;
			return SrvMVCounter;
		}
	}
	if(SrvMVCounter<MAXMV)
	{
		SvrMV[SrvMVCounter].UsageCntr++;
		//printf(".....Usuage Counter =%d\n",SvrMV[SrvMVCounter].UsageCntr);
		SvrMV[SrvMVCounter].Status=ACTIVE;
		strcpy(SvrMV[SrvMVCounter].MVName,MVName);
		SvrMV[SrvMVCounter].MVValue=0;
		SvrMV[SrvMVCounter].MVOwner=ClientID;
		SvrMV[SrvMVCounter].MVThreadID=ThreadID;
		printf("%s lock successfully created!\n",MVName);
		sprintf(SrvBuffer,"%d",SrvMVCounter);
		SrvMVCounter++;
	}
	else
	{
		printf("Error:Reached maximum MV!\n");
		//sprintf(SrvBuffer,"Error:Reached maximum MV!");
		sprintf(SrvBuffer,"%d",FAILURE);
	}
	RespondToClient(SrvBuffer,ClientID,ThreadID);
	delete []SrvBuffer;
	return SrvMVCounter;
}

int ServerDestroyMV(int ClientID, int ThreadID,int MVID)
{
	int ErrorFlag=0;
	int iSuccess=0;
	char *SrvBuffer=new char[100];
	int MVOwnerID;
	// outPktHdr.to = ClientID;
	// outMailHdr.to = ThreadID;

	if(MVID<0 || MVID>MAXMV)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid Lock ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MV ID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}	
}

int ServerSetMV(int ClientID, int ThreadID,int MVID, int MVValue)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if(MVValue<0 || MVID>65536)
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Too large or less value for MVID :%d\n",MVID);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
	if((MVID>=0)&&(MVID<MAXMV))
	{
		if(SvrMV[MVID].Status!=VOID)
		{
			sprintf(SrvBuffer,"%d",SUCCESS);
			//printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
			RespondToClient(SrvBuffer,ClientID,ThreadID);
			SvrMV[MVID].MVValue=MVValue;
			delete []SrvBuffer;
			//printf("In set.....Usuage Counter =%d\n",SvrMV[MVID].UsageCntr);
			return SUCCESS;
		}
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
}

int ServerGetMV(int ClientID, int ThreadID,int MVID)
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
		RespondToClient(SrvBuffer,ClientID,ThreadID);
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
			RespondToClient(SrvBuffer,ClientID,ThreadID);
			delete []SrvBuffer;
			return SvrMV[MVID].MVValue;
		}
	}
	else
	{
		ErrorFlag=ErrorFlag-2;//To return -1
		//sprintf(SrvBuffer,"ERROR!!Invalid MV ID %d",MVID);
		sprintf(SrvBuffer,"%d",FAILURE);
		printf("ERROR!!Invalid MVID ErrorID:%d\n",ErrorFlag);
		RespondToClient(SrvBuffer,ClientID,ThreadID);
		delete []SrvBuffer;
		return ErrorFlag;
	}
}