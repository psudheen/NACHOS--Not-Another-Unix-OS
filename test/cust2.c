#include "syscall.h"
#ifndef NULL
#define NULL    0
#endif
#define LOCK_BLOCK 1
#define NO_OF_OT 				1
#define NO_OF_CUST 			2
#define NO_OF_WAITERS 	1
#define NO_OF_COOKS			1
#define MIN_REQUIRED_INVENTORY 100/* = 20 customers * 5 types of food*/
/*All #Structures*/


typedef struct 
{
 int TypeOfFood;
 int Food[5];
}CustomerData_t;

CustomerData_t CustData[NO_OF_CUST];

typedef struct 
{
 int NoOfOT;
 int NoOfCust;
 int NoOfCooks;
 int NoOfWaiters;
 int NoOfMgr;
}PersonCount_t;

PersonCount_t PersonCount;

enum WorkerStatus {FREE,BUSY,WAITING};


/*Global Variables*/

int custLineLength;
int CustCount;
int OTcount;
int CookCount;
int WaiterCount;
int MoneyInRstrnt;
int TotalTables;
int CustLeft;
int AliveCustCount;
int SleepingWaiterCount;
int SleepingCookCount;
int ActiveOTCount;
int DestroyAllLockCV;

/* arrays */

int SigCustList[NO_OF_CUST];/*index gives the OT id and content gives the CustID*/
int GrabedFood[NO_OF_CUST][NO_OF_CUST];
int EatInCustWaitingForFood[NO_OF_CUST];
int ToGoCustWaitingForFood[NO_OF_CUST];
int ToGoGrabedFood[NO_OF_CUST];
int Inventory[4];/*={250,250,250,250};*/
int ManagerInstrToStop[4];
int OrderTakerStatus[NO_OF_OT];
int CookedFood[5];
int TableAllotedCustList[NO_OF_CUST];
int TblNeededCustList[NO_OF_CUST][NO_OF_CUST];
int FoodToBeCooked[4];
int FoodLevelCheckIteration[4];

/***************************************************************************************
Function prototypes
***************************************************************************************/

/* all the contents here is an equivalent for .h file*/
void Customer();
void Init_LockCV(int size);
void Init_TableSizeData(int size);
void Init_CustomerData( int TypeOfFood, int CustID);
int IsCustomerFoodReady (int CustID);
void CarlJrSimulation (void);

/***************************************************************************************
List of all locks
***************************************************************************************/
int CustDataLock;
int InventoryLock;
int MgrCookStatUpdateLock;
int NeedInventoryLock[NO_OF_CUST];
int CookQLock;
int SigCustListLock ;
int GrabedFoodLock ;
int EatInCustWaitingForFoodLock;
int ToGoCustWaitingForFoodLock;
int ToGoGrabedFoodLock ;
int MgrCookStyleLstLock;
int MonitorAmtOfMoneyLock;
int OrderTakerLock[NO_OF_OT];
int OTStatusUpdateLock;
int CookedFoodLock; 
int TableAllotedCustLock; 
int WaitersQLock; 
int TotalTablesUpdateLock;
int NeedTableLock[NO_OF_CUST];
int TblNeededCustLock; 
int NeedWaitersLock[NO_OF_CUST];
int FoodToBeCookedUpdateLock;
int CustCountLock;
int OTCountLock;
int WaiterCountLock;
int custLineLock;
int AliveCustCountLock;
int customerCountMV;
int SleepingWaiterCountLock;
int ActiveOTCountLock;

/***************************************************************************************
List of all Condition Variables
***************************************************************************************/
int NeedInventoryCV[NO_OF_CUST];
int CookQCV;  
int custWaitingCV;  
int OTWaitingCV[NO_OF_CUST];
int WaitersCV;  
int NeedTableCV[NO_OF_CUST];
int NeedWaitersCV[NO_OF_CUST];


int main()
{
	Print("Entering Customer thread\n");
	Print("WAITING FOR NETWORK SETUP\n");
	StartSimulation();
	Print("READY TO EXECUTE\n");
  CarlJrSimulation();
  
}/* end of main()*/



void CarlJrSimulation (void)
{
    /***************************************************************************************
    Intialization of Global Counters
    ***************************************************************************************/
		int NoOfEatIn=0;
    int NoOfToGo=0;
    int iInitCntr=0;
    int jInitCntr=0;
    int iOTCtr=0;
    int iCustCtr=0;

    
    PersonCount.NoOfOT=0;
    PersonCount.NoOfCust=0;
    PersonCount.NoOfCooks=0;
    PersonCount.NoOfWaiters=0;
    PersonCount.NoOfMgr=1;
    /***************************************************************************************
    Intialization of Global Variables
    ***************************************************************************************/
    custLineLength=CreateMV("CL",2);
    CustCount=CreateMV("CC",2);
    OTcount=CreateMV("OTC",3);

		/***************************************************************************************
    Creation of Global Arrays
    ***************************************************************************************/
		CustData[0].TypeOfFood=CreateMV("CD0",3);
		CustData[1].TypeOfFood=CreateMV("CD1",3);


		
		CustData[0].Food[0]=CreateMV("CD00",4);
		CustData[0].Food[1]=CreateMV("CD01",4);
		CustData[0].Food[2]=CreateMV("CD02",4);
		CustData[0].Food[3]=CreateMV("CD03",4);
		CustData[0].Food[4]=CreateMV("CD04",4);
		
		CustData[1].Food[0]=CreateMV("CDA0",4);
		CustData[1].Food[1]=CreateMV("CDA1",4);
		CustData[1].Food[2]=CreateMV("CDA2",4);
		CustData[1].Food[3]=CreateMV("CDA3",4);
		CustData[1].Food[4]=CreateMV("CDA4",4);
		
		
		

		
		
		
				
		SigCustList[0]=CreateMV("S1",2);
		

		
		
		OrderTakerStatus[0]=CreateMV("OTS0",4);
		
	
		
		SetMV(OrderTakerStatus[0],BUSY);
		
		
		
			   
		#ifdef LOCK_BLOCK
	TblNeededCustList[0][0]=CreateMV("TL00",4);
	TblNeededCustList[0][1]=CreateMV("TL01",4);


		#endif
		Print("End of createMV in cust\n");
    /***************************************************************************************
    Intialization of Global Arrays
    ***************************************************************************************/

    /*CookStyle[4][40]={"$6 Burger","$3 Burger","Veggie Burger","French Fries"};*/

    iInitCntr=0;

    /*Print("Before locks\n");*/
    /***************************************************************************************
    Intialization of locks
    ***************************************************************************************/
    CustDataLock=CreateLock("C1", 2);
    SigCustListLock=CreateLock("S1", 2);
    OTStatusUpdateLock=CreateLock("OX", 2);
    TblNeededCustLock=CreateLock("TA", 2); 
    CustCountLock=CreateLock("CJ", 2);
    OTCountLock=CreateLock("OZ", 2);
    custLineLock=CreateLock("CD",2);
    
		OrderTakerLock[0]=CreateLock("OL0",3);
		

		
	NeedTableLock[0]=CreateLock("NL0",3);
	NeedTableLock[1]=CreateLock("NL1",3);
	
		
	NeedWaitersLock[0]=CreateLock("NW0",3);
	NeedWaitersLock[1]=CreateLock("NW1",3);
	
		
    /***************************************************************************************
    Intialization of all Condition Variables
    ***************************************************************************************/

    CookQCV=CreateCondition("CQ",2);  
    custWaitingCV=CreateCondition("CB",2);
    WaitersCV=CreateCondition("WJ",2);
		
		OTWaitingCV[0]=CreateCondition("OC0",3);
		
		
	
	
	
	NeedTableCV[0]=CreateCondition("NC0",3);
	NeedTableCV[1]=CreateCondition("NC1",3);
	
		
				


	NeedWaitersCV[0]=CreateCondition("WC0",3);
	NeedWaitersCV[1]=CreateCondition("WC1",3);
	
	
	
	AliveCustCountLock=CreateLock("ALC",3);
	customerCountMV=CreateMV("CCMV",4);
		
    PersonCount.NoOfCust=NO_OF_CUST;
    PersonCount.NoOfOT=NO_OF_OT;
    PersonCount.NoOfWaiters=NO_OF_WAITERS;
    PersonCount.NoOfCooks=NO_OF_COOKS;	
	 


	 Customer();
} 

void Init_CustomerData(int TypeOfFood, int CustID)
{
	/*Print("Init Cust Data fn\n");*/
 Acquire(CustDataLock);	
 if(CustData == NULL)
    {
			Print("CustData is null");
			Release(CustDataLock);
			return;
    }
	
	if(CustID==0||CustID==1||CustID==2||CustID==3||CustID==4)
	{
		SetMV(CustData[CustID].TypeOfFood,0);
	}
	else if(CustID==5||CustID==6||CustID==7||CustID==8||CustID==9)
	{
		SetMV(CustData[CustID].TypeOfFood,1);
	}
  /*Food pattern
  Assuming all customers will have any of below food pattern*/
  switch ( CustID%5)
    {
     case 0: SetMV(CustData[CustID].Food[0],1);
             SetMV(CustData[CustID].Food[1],1);
             SetMV(CustData[CustID].Food[2],1);
             SetMV(CustData[CustID].Food[3],1);
             SetMV(CustData[CustID].Food[4],1);
             break;
     case 1: SetMV(CustData[CustID].Food[0],1);
             SetMV(CustData[CustID].Food[1],0);
             SetMV(CustData[CustID].Food[2],1);
             SetMV(CustData[CustID].Food[3],0);
             SetMV(CustData[CustID].Food[4],1);
             break;
     case 2: SetMV(CustData[CustID].Food[0],0);
             SetMV(CustData[CustID].Food[1],1);
             SetMV(CustData[CustID].Food[2],0);
             SetMV(CustData[CustID].Food[3],0);
             SetMV(CustData[CustID].Food[4],1);
             break;
     case 3: SetMV(CustData[CustID].Food[0],1);
             SetMV(CustData[CustID].Food[1],0);
             SetMV(CustData[CustID].Food[2],1);
             SetMV(CustData[CustID].Food[3],0);
             SetMV(CustData[CustID].Food[4],1);
             break;
     case 4: SetMV(CustData[CustID].Food[0],0);
             SetMV(CustData[CustID].Food[1],0);
             SetMV(CustData[CustID].Food[2],0);
             SetMV(CustData[CustID].Food[3],0);
             SetMV(CustData[CustID].Food[4],1) ;
             break;
     default: SetMV(CustData[CustID].Food[0],0);
             SetMV(CustData[CustID].Food[1],0);
             SetMV(CustData[CustID].Food[2],0);
             SetMV(CustData[CustID].Food[3],0);
             SetMV(CustData[CustID].Food[4],1);
             break;
    }
	Release(CustDataLock);
}

void Customer()
{
	int CustID=-1;
	int myOT=-1;
	int i=0;
	int j=0;
	int NoOfEatIn=0;
	Print("Customer thread\n");
	
	
	Acquire(CustCountLock);
	CustID=GetMV(CustCount);
	Print1("my customer number is %d\n",CustID);
	SetMV(CustCount,(CustID+1));
/* 	Print1("THE VALUE of cust count  AFTER SETIING  IS %d\n",GetMV(custCountMV)); */
	Release(CustCountLock);
	
	Init_CustomerData(0, CustID); 
	Acquire(custLineLock);
	/* Acquire(OTStatusUpdateLock);  */
	for(i=0;i<NO_OF_OT;i++) /* HARD CODED SINCE NUMBER OF OT ARE FIXED*/
	{
		Print("here\n");
		Print1("OT status %d\n",GetMV(OrderTakerStatus[i]));
		if(FREE==GetMV(OrderTakerStatus[i])) /*checking for a free order taker*/
		{
			SetMV(OrderTakerStatus[i],BUSY);              /*setting to busy*/
			myOT=i;
			break;
		}
	}
	i=0; 
	/* Release(OTStatusUpdateLock); */
	
	/*Here either I know who is my OT or there are no OT for me*/
	if(myOT==-1) /*get in line*/
	{
		SetMV(custLineLength,(GetMV(custLineLength)+1));
		Print1("CUSTOMER:customer line length = %d \n",GetMV(custLineLength));
		Print1("Customer[%d] has NO OrderTakers, Snoozing!\n", CustID);
		Wait(custWaitingCV,custLineLock);    
		Print1("Waiting Customer[%d] is signalled by OT\n",CustID);            
	}
   

	for(j=0;j<NO_OF_OT;j++)
	{
		if (GetMV(OrderTakerStatus[j])==WAITING)   /*checking for ot who is in wait for him*/
		{
			myOT=j;
			SetMV(OrderTakerStatus[j],BUSY);
			break;
		}
	} 
 
	Release(custLineLock);
	Acquire(OrderTakerLock[myOT]);
	SetMV(SigCustList[myOT],CustID);
	Print2("CUSTOMER[%d] IS GIVING ORDER TO ORDERTAKER[%d]\n",CustID,myOT);
  if(GetMV(CustData[CustID].Food[0]) == 0)
		Print1("CUSTOMER[%d] IS NOT ORDERING 6-DOLLAR BURGER\n", CustID);
  else
		Print1("CUSTOMER[%d] IS ORDERING 6-dollar burger\n", CustID);
  if(GetMV(CustData[CustID].Food[1]) == 0)
		Print1("CUSTOMER[%d] IS NOT ORDERING 3-DOLLAR BURGER\n", CustID);
  else
		Print1("CUSTOMER[%d] IS ORDERING 3-DOLLAR BURGER\n", CustID);
  if(GetMV(CustData[CustID].Food[2])== 0)
		Print1("CUSTOMER[%d] IS NOT ORDERING veggie BURGER\n", CustID);
  else
		Print1("CUSTOMER[%d] IS ORDERING veggie BURGER\n", CustID);
  if(GetMV(CustData[CustID].Food[3]) == 0)
		Print1("CUSTOMER[%d] IS NOT ORDERING FRENCH FRIES\n", CustID);
  else
		Print1("CUSTOMER[%d] IS ORDERING FRENCH FRIES\n", CustID);
  if(GetMV(CustData[CustID].Food[4]) == 0)
		Print1("CUSTOMER[%d] IS NOT ORDERING Soda\n", CustID);
  else
		Print1("CUSTOMER[%d] IS ORDERING Soda\n", CustID);
/* 	NoOfEatIn = (2*(NO_OF_CUST))/3;
	if(custId <= NoOfEatIn )
	{
	Print1("Customer [%d] chooses to Eat-in the food\n",custId);
	}
	else
	{
	Print1("Customer [%d] chooses to To-go the food\n",custId);
	} */
  Signal(OTWaitingCV[myOT],OrderTakerLock[myOT]);
	/*Print1("Customer[%d] is waiting for OT to respond for orders(2nd wait)\n",	CustID);*/
  Wait(OTWaitingCV[myOT],OrderTakerLock[myOT]);/* Waiting for my order to be processed*/
        
	if (GetMV(CustData[CustID].TypeOfFood) == 0)
	{ 
		Release(OrderTakerLock[myOT]);
		Acquire(TblNeededCustLock);
		SetMV(TblNeededCustList[myOT][CustID],1);    /*1 indicates that customer needs table*/
		Release(TblNeededCustLock);

		Acquire(NeedTableLock[CustID]);
		Print1("CUSTOMER %d IS WAITING FOR THE TABLE\n", CustID);
		Wait(NeedTableCV[CustID],NeedTableLock[CustID]);
		Signal(NeedTableCV[CustID],NeedTableLock[CustID]);/*Send ack to Mgr*/
		Print1("Eat IN CUSTOMER %d GOT THE TABLE\n", CustID);
		/* Manager signals only when there are free tables*/
		/* Here Customers are sure of allocated Table*/
		Release(NeedTableLock[CustID]);

		/*Customer Waits for Waiter
		Eat-in Customer whose food is ready, seated and waiting for waiters to serve them*/
		Acquire(NeedWaitersLock[CustID]);
		Print1("CUSTOMER [%d] IS WAITING FOR THE WAITER TO SERVE THE FOOD\n",CustID);                              
		Wait(NeedWaitersCV[CustID],NeedWaitersLock[CustID]);
		/*Waiters signals only when Customer Food is ready
		Here Customers are sure of served food and they must leave!*/
		Print1("CUSTOMER [%d] IS SERVED BY THE WAITER",CustID);
		Release(NeedWaitersLock[CustID]);
		Acquire(AliveCustCountLock);
		Print1("the alive customer count is====== %d \n",GetMV(customerCountMV));
		SetMV(customerCountMV,(GetMV(customerCountMV)-1));
		Print1("the alive customer count is====== %d \n",GetMV(customerCountMV));
		Release(AliveCustCountLock);
		Print1("\n CUSTOMER[%d]  IS LEAVING THE RESTAURANT AFTER HAVING FOOD\n\n\n",CustID);
	}

	else
	{
		Release(OrderTakerLock[myOT]);
		Acquire(AliveCustCountLock);
		Print1("the alive customer count is====== %d \n",GetMV(customerCountMV));
		SetMV(customerCountMV,(GetMV(customerCountMV)-1));
		Print1("the alive customer count is====== %d \n",GetMV(customerCountMV));
		Release(AliveCustCountLock);
		Print2("\n CUSTOMER[%d] IS LEAVING THE RESTAURANT AFTER ORDER TAKEr[%d] PACKED THE FOOD\n\n\n",CustID,myOT );
		
	}

	/*Print1("Customer %d is exiting\n",CustID);*/
		Exit(1);
}










