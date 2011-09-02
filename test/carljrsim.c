/***********************************************************************
* Carl Jr simulation data
*
***************************************************************************************
*/
/*All #defines*/
#include "syscall.h"
#ifndef NULL
#define NULL    0
#endif
#define MAX_SIZE 100 /* Maximum possible customers/OT/Waiters/cook */
#define MIN_REQUIRED_INVENTORY 100/* = 20 customers * 5 types of food*/
#define COOKEDFOOD_MAX ((COOKEDFOOD_MIN*3)+1)

/*All #Structures*/
typedef struct 
{
 int CustID;
 int TypeOfFood;
 int Food[5];
}CustomerData_t;

CustomerData_t CustData[MAX_SIZE];

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

int SigCustList[MAX_SIZE];/*index gives the OT id and content gives the CustID*/
int GrabedFood[MAX_SIZE][MAX_SIZE];
int EatInCustWaitingForFood[MAX_SIZE];
int ToGoCustWaitingForFood[MAX_SIZE];
int ToGoGrabedFood[MAX_SIZE];
int MgrCookStyleLst[MAX_SIZE][MAX_SIZE];
char CookStyle[4][40];/*={"$6 Burger","$3 Burger","Veggie Burger","French Fries"};*/
int Inventory[4];/*={250,250,250,250};*/
int ManagerInstrToStop[4];
int OrderTakerStatus[MAX_SIZE];
int CookedFood[5];
int TableAllotedCustList[MAX_SIZE];
int TblNeededCustList[MAX_SIZE][MAX_SIZE];
int FoodToBeCooked[4];
int FoodLevelCheckIteration[4];






/***************************************************************************************
Function prototypes
***************************************************************************************/

/* all the contents here is an equivalent for .h file*/
void Customer();
void OrderTaker();
void Waiter();
void Manager();
void Cook();
void Init_LockCV(int size);
void Init_TableSizeData(int size);
void Init_CustomerData( int TypeOfFood, int CustID);
int IsCustomerFoodReady (int CustID);
void CarlJrSimulation (void);
void SimiluationMenu();
int DestroyAllLocksCV();
/*void Init_CustomerData(CustomerData_t *CustData, int TypeOfFood, int CustID);
int IsCustomerFoodReady (CustomerData_t *CustData);*/

/***************************************************************************************
List of all locks
***************************************************************************************/
int CustDataLock;
int InventoryLock;
int MgrCookStatUpdateLock;
int NeedInventoryLock[MAX_SIZE];
int CookQLock;
int SigCustListLock ;
int GrabedFoodLock ;
int EatInCustWaitingForFoodLock;
int ToGoCustWaitingForFoodLock;
int ToGoGrabedFoodLock ;
int MgrCookStyleLstLock;
int MonitorAmtOfMoneyLock;
int OrderTakerLock[MAX_SIZE];
int OTStatusUpdateLock;
int CookedFoodLock; 
int TableAllotedCustLock; 
int WaitersQLock; 
int TotalTablesUpdateLock;
int NeedTableLock[MAX_SIZE];
int TblNeededCustLock; 
int NeedWaitersLock[MAX_SIZE];
int FoodToBeCookedUpdateLock;
int CustCountLock;
int OTCountLock;
int WaiterCountLock;
int custLineLock;
int AliveCustCountLock;
int SleepingWaiterCountLock;
int ActiveOTCountLock;

/***************************************************************************************
List of all Condition Variables
***************************************************************************************/
int NeedInventoryCV[MAX_SIZE];
int CookQCV;  
int custWaitingCV;  
int OTWaitingCV[MAX_SIZE];
int WaitersCV;  
int NeedTableCV[MAX_SIZE];
int NeedWaitersCV[MAX_SIZE];





int main()
{

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
	custLineLength=0;
	CustCount=0;
	OTcount=0;
	CookCount=0;
	WaiterCount=0;
	MoneyInRstrnt=0;
	TotalTables=25;
	CustLeft=0;
	SleepingWaiterCount=0;
	ActiveOTCount=0;
	DestroyAllLockCV=1;

	/***************************************************************************************
	Intialization of Global Arrays
	***************************************************************************************/

	for(iInitCntr=0;iInitCntr<MAX_SIZE;iInitCntr++)
	{
	  SigCustList[iInitCntr]=EatInCustWaitingForFood[iInitCntr]=ToGoCustWaitingForFood[iInitCntr]=\
	  ToGoGrabedFood[iInitCntr]=TableAllotedCustList[iInitCntr]=-1;
		OrderTakerStatus[iInitCntr]=FREE;
	}
	iInitCntr=0;
	for(iInitCntr=0;iInitCntr<MAX_SIZE;iInitCntr++)
	{
	  for(jInitCntr=0;jInitCntr<MAX_SIZE;jInitCntr++)
	  {
			GrabedFood[iInitCntr][jInitCntr]=MgrCookStyleLst[iInitCntr][jInitCntr]=TblNeededCustList[iInitCntr][jInitCntr]=-1;
	  }
	}
	iInitCntr=0;
	jInitCntr=0;
	/*CookStyle[4][40]={"$6 Burger","$3 Burger","Veggie Burger","French Fries"};*/

	ManagerInstrToStop[4];
	CookedFood[5]=100000000;
	for(iInitCntr=0;iInitCntr<5;iInitCntr++)
	{
	  FoodToBeCooked[iInitCntr]=FoodLevelCheckIteration[iInitCntr]=0;
		Inventory[iInitCntr]=250;
		CookedFood[iInitCntr]=100;
	}
	
	iInitCntr=0;

	/*Print("Before locks\n");*/
	/***************************************************************************************
	Intialization of locks
	***************************************************************************************/
	CustDataLock=CreateLock("CustDataLock", 12);
	InventoryLock=CreateLock("InventoryLock", 13);
	MgrCookStatUpdateLock=CreateLock("MgrCookStatUpdateLock", 21);
	CookQLock=CreateLock("InventoryLock", 13);
	SigCustListLock=CreateLock("SigCustListLock", 15);
	GrabedFoodLock=CreateLock("GrabedFoodLock", 14);
	EatInCustWaitingForFoodLock=CreateLock("EatInCustWaitingForFoodLock", 27);
	ToGoCustWaitingForFoodLock=CreateLock("ToGoCustWaitingForFoodLock", 26);
	ToGoGrabedFoodLock=CreateLock("ToGoGrabedFoodLock", 18);
	MgrCookStyleLstLock=CreateLock("MgrCookStyleLstLock", 19);
	MonitorAmtOfMoneyLock=CreateLock("MonitorAmtOfMoneyLock", 21);
	OTStatusUpdateLock=CreateLock("OTStatusUpdateLock", 18);
	CookedFoodLock=CreateLock("CookedFoodLock", 14); 
	TableAllotedCustLock=CreateLock("TableAllotedCustLock", 20); 
	WaitersQLock=CreateLock("WaitersQLock", 12); 
	TotalTablesUpdateLock=CreateLock("TotalTablesUpdateLock", 21);
	TblNeededCustLock=CreateLock("TblNeededCustLock", 17); 
	FoodToBeCookedUpdateLock=CreateLock("FoodToBeCookedUpdateLock", 24);
	CustCountLock=CreateLock("CustCountLock", 13);
	OTCountLock=CreateLock("OTCountLock", 11);
	WaiterCountLock=CreateLock("WaiterCountLock",15);
	custLineLock=CreateLock("custLineLock",12);
	AliveCustCountLock=CreateLock("AliveCustCountLock",18);
	SleepingWaiterCountLock=CreateLock("SleepingWaiterCountLock",23);
	ActiveOTCountLock=CreateLock("ActiveOTCountLock",17);
	

	iInitCntr=0;
	for(iInitCntr=0;iInitCntr<MAX_SIZE;iInitCntr++)
	{
	  NeedInventoryLock[iInitCntr]=CreateLock("NeedInventoryLock",17);
	  OrderTakerLock[iInitCntr]=CreateLock("OrderTakerLock",14);
	  NeedTableLock[iInitCntr]=CreateLock("NeedTableLock",12);
	  NeedWaitersLock[iInitCntr]=CreateLock("NeedWaitersLock",15);
	}
	/*Print1("MAX LOCK COUNT =%d\n",NeedWaitersLock[iInitCntr]);*/
	iInitCntr=0;

	/***************************************************************************************
	Intialization of all Condition Variables
	***************************************************************************************/

	CookQCV=CreateCondition("CookQCV",7);  
	custWaitingCV=CreateCondition("custWaitingCV",13);
	WaitersCV=CreateCondition("WaitersCV",9);
	iInitCntr=0;
	for(iInitCntr=0;iInitCntr<MAX_SIZE;iInitCntr++)
	{
	  OTWaitingCV[iInitCntr]=CreateCondition("OTWaitingCV",11);
	  NeedInventoryCV[iInitCntr]=CreateCondition("NeedInventoryCV",15); 
	  NeedTableCV[iInitCntr]=CreateCondition("NeedTableCV",11);
	  NeedWaitersCV[iInitCntr]=CreateCondition("NeedWaitersCV",13);
	}
	iInitCntr=0;

	/*Print("Please enter the number of Customers:");
	PersonCount.NoOfCust=Scan("%d");
	Print("Please enter the number of Order Takers:");
	PersonCount.NoOfOT=Scan("%d");
	Print("Please enter the number of Waiters:");
	PersonCount.NoOfWaiters=Scan("%d");
	Print("Please enter the number of Cooks:");
	PersonCount.NoOfCooks=Scan("%d");*/
	PersonCount.NoOfCust=20;
	PersonCount.NoOfOT=4;
	PersonCount.NoOfWaiters=3;
	PersonCount.NoOfCooks=4;
	
	Acquire(AliveCustCountLock);
	AliveCustCount=PersonCount.NoOfCust;
	Release(AliveCustCountLock);
	Acquire(ActiveOTCountLock);
	ActiveOTCount=PersonCount.NoOfOT;
	Release(ActiveOTCountLock);
	/* create OrderTaker threads */
	for(iOTCtr=0; iOTCtr<PersonCount.NoOfOT; iOTCtr++)
	{
	  Fork(OrderTaker);
	}
	/* create Customer threads */
	for(iCustCtr=0; iCustCtr<PersonCount.NoOfCust; iCustCtr++)
	{
	  /*Logic to determine eat-in or To-go type*/
	  NoOfEatIn = (2*(PersonCount.NoOfCust))/3;
	  NoOfToGo = (PersonCount.NoOfCust) - NoOfEatIn;
	  if(iCustCtr <= NoOfEatIn )
		{
		  Init_CustomerData(0, iCustCtr);
		}
	  else
		{
		  Init_CustomerData(1, iCustCtr);
		}
	   Fork(Customer);
	}
	/*Create Waiters*/
	iInitCntr=0;
	for (iInitCntr=0; iInitCntr<PersonCount.NoOfWaiters; iInitCntr++)
	{
		Fork(Waiter);
	}
	iInitCntr=0;
	/*Create Cooks*/
	for (iInitCntr=0; iInitCntr<PersonCount.NoOfCooks; iInitCntr++)
	{
		/*Fork(Cook);*/
	}
	iInitCntr=0;
	/*Create Manager*/
	Fork(Manager);
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
 CustData[CustID].TypeOfFood = TypeOfFood;
  /*Food pattern
  Assuming all customers will have any of below food pattern*/
  switch ( CustID%5)
    {
     case 0: CustData[CustID].Food[0] = 1;
             CustData[CustID].Food[1] = 1;
             CustData[CustID].Food[2] = 1;
             CustData[CustID].Food[3] = 1;
             CustData[CustID].Food[4] = 1;
             break;
     case 1: CustData[CustID].Food[0] = 1;
             CustData[CustID].Food[1] = 0;
             CustData[CustID].Food[2] = 1;
             CustData[CustID].Food[3] = 0;
             CustData[CustID].Food[4] = 1;
             break;
     case 2: CustData[CustID].Food[0] = 0;
             CustData[CustID].Food[1] = 1;
             CustData[CustID].Food[2] = 0;
             CustData[CustID].Food[3] = 0;
             CustData[CustID].Food[4] = 1;
             break;
     case 3: CustData[CustID].Food[0] = 1;
             CustData[CustID].Food[1] = 0;
             CustData[CustID].Food[2] = 1;
             CustData[CustID].Food[3] = 0;
             CustData[CustID].Food[4] = 1;
             break;
     case 4: CustData[CustID].Food[0] = 0;
             CustData[CustID].Food[1] = 0;
             CustData[CustID].Food[2] = 0;
             CustData[CustID].Food[3] = 0;
             CustData[CustID].Food[4] = 1;
             break;
     default: CustData[CustID].Food[0] = 0;
             CustData[CustID].Food[1] = 0;
             CustData[CustID].Food[2] = 0;
             CustData[CustID].Food[3] = 0;
             CustData[CustID].Food[4] = 1;
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

	Acquire(CustCountLock);
	CustID = CustCount++;
	Release(CustCountLock);
	Acquire(custLineLock);
	Acquire(OTStatusUpdateLock);
	for(i=0;i<PersonCount.NoOfOT;i++)
    {
      if(FREE == OrderTakerStatus[i]) /*checking for a free order taker*/
      {
          OrderTakerStatus[i] = BUSY;/*make ith (my) OT busy*/
          myOT=i;
          break;
      }
    }
		i=0;
	Release(OTStatusUpdateLock);
	
	/*Here either I know who is my OT or there are no OT for me*/
	if(-1==myOT) /*get in line*/
    {
		    custLineLength++;
        /*Print1("Customer[%d] has NO OrderTakers, Snoozing!\n", CustID);*/
        Wait(custWaitingCV,custLineLock);	
				/*Print1("Waiting Customer[%d] is signalled by OT\n",	CustID);*/			
    }
	Release(custLineLock);
	/*Get a waiting OT*/
	Acquire(OTStatusUpdateLock);
	for(j=0;j<PersonCount.NoOfOT;j++)
	{
	  if (WAITING == OrderTakerStatus[j])
	  {
			myOT=j;
			OrderTakerStatus[j]=BUSY;
			break;
	  }
	}
	j=0;
	Release(OTStatusUpdateLock);

	 /*Allow OT to manipluate CustLineLength*/
     /*By here, Customer is aware of his OT and ready to order his/her food*/
	
	Acquire(SigCustListLock);
  SigCustList[CustID]=myOT;
  Release(SigCustListLock);

  Print2("Customer[%d] is giving order to OrderTaker[%d]\n",CustID,myOT);
	Acquire(CustDataLock);
  if(CustData[CustID].Food[0] == 0)
  Print1("Customer[%d] is NOT ordering 6-dollar burger\n", CustID);
  else
  Print1("Customer[%d] is ordering 6-dollar burger\n", CustID);
  if(CustData[CustID].Food[1] == 0)
  Print1("Customer[%d] is NOT ordering 3-dollar burger\n", CustID);
  else
  Print1("Customer[%d] is ordering 3-dollar burger\n", CustID);
  if(CustData[CustID].Food[2] == 0)
  Print1("Customer[%d] is NOT ordering veggie burger\n", CustID);
  else
  Print1("Customer[%d] is ordering veggie burger\n", CustID);
  if(CustData[CustID].Food[3] == 0)
  Print1("Customer[%d] is NOT ordering French fries\n", CustID);
  else
  Print1("Customer[%d] is ordering French fries\n", CustID);
  if(CustData[CustID].Food[4] == 0)
  Print1("Customer[%d] is NOT ordering Soda\n", CustID);
  else
  Print1("Customer[%d] is ordering Soda\n", CustID);
	Release(CustDataLock);

	  NoOfEatIn = (2*(PersonCount.NoOfCust))/3;
	  if(CustID <= NoOfEatIn )
		{
		  Print1("Customer [%d] chooses to Eat-in the food\n",CustID);
		}
	  else
		{
		 Print1("Customer [%d] chooses to To-go the food\n",CustID);
		}
  /*Wake-up the WAITING OT who is waiting for my order.*/
  Acquire(OrderTakerLock[myOT]);
  Signal(OTWaitingCV[myOT],OrderTakerLock[myOT]);
	/*Print1("Customer[%d] is waiting for OT to respond for orders(2nd wait)\n",	CustID);*/
  Wait(OTWaitingCV[myOT],OrderTakerLock[myOT]);/* Waiting for my order to be processed*/
  Print3("Customer [%d] is given token number [%d] by the OrderTaker [%d]\n",CustID,CustID,myOT);
  Release(OrderTakerLock[myOT]);

  /*Customer-Manager interaction
    If Customer is Eat-in type
    Wait for Manager to check if there is table for me
      Need lock and CV to go on wait*/
	Acquire(CustDataLock);
  if (CustData[CustID].TypeOfFood == 0)
    {
			Release(CustDataLock);
      Acquire(TblNeededCustLock);
      TblNeededCustList[myOT][CustID]=1;	/*1 indicates that customer needs table*/
      Release(TblNeededCustLock);

      Acquire(NeedTableLock[CustID]);
      Print1("Customer %d is waiting for the table\n", CustID);
      Wait(NeedTableCV[CustID],NeedTableLock[CustID]);
      Signal(NeedTableCV[CustID],NeedTableLock[CustID]);/*Send ack to Mgr*/
      Print1("Eat IN Customer %d got the table\n", CustID);
      /* Manager signals only when there are free tables*/
        /* Here Customers are sure of allocated Table*/
      Release(NeedTableLock[CustID]);

      /*Customer Waits for Waiter
        Eat-in Customer whose food is ready, seated and waiting for waiters to serve them*/
      Acquire(NeedWaitersLock[CustID]);
      Print1("Customer [%d] is waiting for the waiter to serve the food\n",CustID);							  
      Wait(NeedWaitersCV[CustID],NeedWaitersLock[CustID]);
      /*Waiters signals only when Customer Food is ready
        Here Customers are sure of served food and they must leave!*/
	 Print1("Customer [%d] is served by the waiter",CustID);
      Release(NeedWaitersLock[CustID]);
      Print1("\nCustomer[%d] order is leaving the restaurant after having food\n\n\n",CustID );
    }
    else
      {
				 Release(CustDataLock);
         Print2("\nCustomer[%d] is leaving the restaurant after Order Taker[%d] packed the food\n\n\n",CustID,myOT );
      }
  CustLeft++;
	Acquire(AliveCustCountLock);
	AliveCustCount--;
	Release(AliveCustCountLock);
	/*Print1("Customer %d is exiting\n",CustID);*/
  Exit(1);	

}

void OrderTaker()
{
  int MyCustID = -1;
  int CustFoodIsReady =-1;
  int Poped_CID = -1;
  int Poped_OT = -1;
	int i=0;
	int myOTId=0;
	
  Acquire(OTCountLock);
	myOTId = OTcount++;
	Release(OTCountLock);
	/*Print1("Created %d OT\n", myOTId);*/
  while(1)
   {
		Acquire(AliveCustCountLock);
		if(AliveCustCount==0)
		{
			Acquire(ActiveOTCountLock);
			ActiveOTCount--;
			Release(ActiveOTCountLock);
			Release(AliveCustCountLock);
			Exit(1);
		}
		else
			Release(AliveCustCountLock);
    Acquire(custLineLock);
    /*Are there any waiting customers?*/
    if(custLineLength > 0)
      {
        Signal(custWaitingCV,custLineLock);
        custLineLength--;
        Release(custLineLock);
        Acquire(OTStatusUpdateLock);
        OrderTakerStatus[myOTId]=WAITING;
        Release(OTStatusUpdateLock);
      }
    /*Is there any food to bag?*/
	else
	{
	  Release(custLineLock);
	  Acquire(EatInCustWaitingForFoodLock);
	  for(i=0;i<MAX_SIZE;i++)
	  {
			if(EatInCustWaitingForFood[i]==1)
			{
				EatInCustWaitingForFood[i]=0;
				MyCustID=i;
				break;
			}
	  }
		i=0;
	 
	/*For (each) this customer, check if his order is ready for each type of food*/
	if(MyCustID!=-1)
		{
			CustFoodIsReady = IsCustomerFoodReady (MyCustID);
			 /* if CustFoodIsReady = 1, Food is ready and can be grabed*/
			 if (CustFoodIsReady==1)
				{
				 Acquire(GrabedFoodLock);
				 GrabedFood[myOTId][MyCustID]=1;/* 1 means food is grabed and 0 means otherwise*/
				 Release(GrabedFoodLock);
				 Print2("OrderTaker[%d] packed the food for Customer[%d]\n",myOTId, MyCustID);
				 MyCustID=-1;
				 Release(EatInCustWaitingForFoodLock);
				}
			 else
				{
				 /*list of customers to whom token should be handled but shouldnt grab the food*/
				 /*this Q is as good as saying who gave token number to whom when they wanna wait*/
				  EatInCustWaitingForFood[MyCustID]=1;
				  Print1("OrderTaker[%d] gives token number to Customer[%d]\n", MyCustID);
					MyCustID=-1;
					Release(EatInCustWaitingForFoodLock);
				}
		}
	
	 else
		{
			Release(EatInCustWaitingForFoodLock);
			Acquire(ToGoCustWaitingForFoodLock);
			i=0;
			for(i=0;i<MAX_SIZE;i++)
			{
				if(ToGoCustWaitingForFood[i]==1)
				{
					ToGoCustWaitingForFood[i]=0;
					MyCustID=i;
					break;
				}
			}
			i=0;
			
			if(MyCustID!=-1)
			{
				CustFoodIsReady = IsCustomerFoodReady (MyCustID);
				 /* if CustFoodIsReady = 1, Food is ready and can be grabed*/
				 if (CustFoodIsReady==1)
					{
					 Acquire(GrabedFoodLock);
					 GrabedFood[myOTId][MyCustID]=1;/* 1 means food is grabed and 0 means otherwise*/
					 Release(GrabedFoodLock);
					 Print2("OrderTaker[%d] gives food to the Customer[%d]\n",myOTId, MyCustID);
					 MyCustID=-1;
					 Release(ToGoCustWaitingForFoodLock);
					}
				 else
					{
					 /*list of customers to whom token should be handled but shouldnt grab the food
					 /*this Q is as good as saying who gave token number to whom when they wanna wait*/
					  ToGoCustWaitingForFood[MyCustID]=1;
					  Print2("OrderTaker[%d] gives token number to Customer[%d]\n",myOTId, MyCustID);
						MyCustID=-1;
						Release(ToGoCustWaitingForFoodLock);
					}
			}
			else
			{
				Release(ToGoCustWaitingForFoodLock);
				/*Nothing to do
				/*Set OT status to FREE*/
				Acquire(OTStatusUpdateLock);
				OrderTakerStatus[myOTId]=FREE;
				Release(OTStatusUpdateLock);
				Acquire(AliveCustCountLock);
				if(AliveCustCount==0)
				{
					Acquire(ActiveOTCountLock);
					ActiveOTCount--;
					Release(ActiveOTCountLock);
					Release(AliveCustCountLock);
					Exit(1);
				}
				else
					Release(AliveCustCountLock);
			}
		}
	}

  /*Now, OT is ready to take order
  /* Take interaction lock*/
  Acquire(OrderTakerLock[myOTId]);
  /*release hold on CustLineLen
  /*OT should wait until customer signals him with orders*/
	/*Print1(" %d OT waiting for customers to signal\n", myOTId);*/
  Wait(OTWaitingCV[myOTId],OrderTakerLock[myOTId]);
	/*Print1(" %d OT waiting for customers is siganlled!! Yay!!\n", myOTId);*/
  /*When I'm here, Customer has signaled that his order is ready*/
  /*Compute the money and handle the token number.*/

  /*check which customer signalled me?*/
  Acquire(SigCustListLock);
	/*Print2("Acquiried SigCustListLock%d by %d\n",SigCustListLock,myOTId);*/
  i=0;
  for(i=0;i<MAX_SIZE;i++)
  { 
		/*Print("Inside For loop\n");*/
		if(SigCustList[i]==myOTId)
		{
			MyCustID=i;
			SigCustList[i]=-1;
			break;
		}
  }
	i=0;
	/*Print2("Releasing SigCustListLock%d by %d\n",SigCustListLock,myOTId);*/
  
	/*Print1("Cust ID after I poped out %d\n",MyCustID);*/

  Acquire(MonitorAmtOfMoneyLock);
	Acquire(CustDataLock);
  MoneyInRstrnt = (6 * CustData[MyCustID].Food[0] + 3 * CustData[MyCustID].Food[1] + 4 * CustData[MyCustID].Food[2] \
                  + 2 * CustData[MyCustID].Food[3] + CustData[MyCustID].Food[4]);
	
  Release(MonitorAmtOfMoneyLock);
	Release(SigCustListLock);
	Release(CustDataLock);
	
  Print2("OrderTaker[%d] is taking order of the Customer [%d]\n", myOTId, MyCustID);

/*Order Processing
  /* Check the customer style of eating - eat in or to-go?*/
	Acquire(CustDataLock);
  if( CustData[MyCustID].TypeOfFood == 0)
    {
			Release(CustDataLock);
     /*Eat - in customers
     /*If type of food is only Soda, token is handled
     /*and add to GrabedFood*/
     if( MyCustID%5==4)
        {
          /*Update the list which waiter will look to serve customers*/
			Acquire(GrabedFoodLock);
			GrabedFood[myOTId][MyCustID]=1;
			Release(GrabedFoodLock);
        }
      /* Customers with multiple orders*/
      else
        {
         /*For (each) this customer, check if his order is ready for each type of food*/
          CustFoodIsReady = IsCustomerFoodReady (MyCustID);
         /* if CustFoodIsReady = 1, Food is ready and can be grabed*/
         if (CustFoodIsReady==1)
            {
							Acquire(GrabedFoodLock);
							GrabedFood[myOTId][MyCustID]=1;/* 1 means food is grabed and 0 means otherwise*/
							Release(GrabedFoodLock);
							Print2("OrderTaker[%d] gives token number to Customer[%d]\n",myOTId, MyCustID);
            }
        else
            {
             /*list of customers to whom token should be handled but shouldnt grab the food
             /*this Q is as good as saying who gave token number to whom when they wanna wait*/
              Acquire(EatInCustWaitingForFoodLock);
              EatInCustWaitingForFood[MyCustID]=1;
              Release(EatInCustWaitingForFoodLock);
              Print2("OrderTaker[%d] gives token number to Customer[%d]\n",myOTId, MyCustID);
            }


        }

    }
  /* Customers of type togo*/
  else
    {
			Release(CustDataLock);
     if( MyCustID%5 == 4)/* If customer needs only Soda*/
       {
        /*Signal this waiting customer and "wait" for him to acknowledge*/
        /*If acknowledged, signal the waiting customer and Customer threads finishes*/
       }
     /* Customers with multiple orders*/
     else
       {
        /*For (each) this customer, check if his order is ready for each type of food*/
        CustFoodIsReady = IsCustomerFoodReady (MyCustID);
         /* if CustFoodIsReady = 1, Food is ready and can be grabed*/
        if (CustFoodIsReady==1)
           {
            Acquire(ToGoGrabedFoodLock);
            ToGoGrabedFood[MyCustID]=1;
            Release(ToGoGrabedFoodLock);
            Print2("OrderTaker[%d] packed the food for Customer[%d]\n",myOTId, MyCustID);
            /*Signal(OTWaitingCV[myOTId],OrderTakerLock[myOTId]);
            Release(OrderTakerLock[myOTId]);
            continue;*/
           }
        else
           {
            /*list of customers to whom token should be handled but shouldnt grab the food*/
            /*this Q is as good as saying who gave token number to whom when they wanna wait*/
             Acquire(ToGoCustWaitingForFoodLock);
             ToGoCustWaitingForFood[MyCustID]=1;
             Release(ToGoCustWaitingForFoodLock);
             Print2("OrderTaker[%d] gives token number to Customer[%d]\n",myOTId, MyCustID);
           }
       }

    }
    /*Order processed Signal the customer to get out of sleep!*/
    Signal(OTWaitingCV[myOTId],OrderTakerLock[myOTId]);

    Release(OrderTakerLock[myOTId]);
		Acquire(custLineLock);
		/*Are there any waiting customers?*/
		if(custLineLength==0)
		{
			Release(custLineLock);
			Acquire(ActiveOTCountLock);
			ActiveOTCount--;
			Release(ActiveOTCountLock);
			/*Print1("\t\t\t\n\nOT %d is exiting\n\n\n\n",myOTId);*/
			Exit(1);
		}
		else
			Release(custLineLock);

   }/*end of while()*/
	
}/*end of OrderTaker()*/

int IsCustomerFoodReady (int CID)
{
	int CustFoodIsReady = 0;
	int CustID = CID;
	Acquire(CustDataLock);
	Acquire(CookedFoodLock);
	Acquire(FoodToBeCookedUpdateLock);
	switch(CustID%5)
				{
				case 0: if ( CustData[CustID].Food[0] < CookedFood[0] && \
								CustData[CustID].Food[1] < CookedFood[1] &&\
								CustData[CustID].Food[2] < CookedFood[2] && \
								CustData[CustID].Food[3] < CookedFood[3] && \
								CustData[CustID].Food[4] < CookedFood[4])
										{
											CustFoodIsReady = 1;
											CookedFood[0]--;
											CookedFood[1]--;
											CookedFood[2]--;
											CookedFood[3]--;
											CookedFood[4]--;
										}
								else
										{
											FoodToBeCooked[0]++;
											FoodToBeCooked[1]++;
											FoodToBeCooked[2]++;
											FoodToBeCooked[3]++;
										}
								break;
				case 1: if ( CustData[CustID].Food[0] < CookedFood[0]&&\
								CustData[CustID].Food[2] < CookedFood[2]&&\
								CustData[CustID].Food[4] < CookedFood[4])
										{
										 CustFoodIsReady = 1;
										 CookedFood[0]--;
										 CookedFood[2]--;
										 CookedFood[4]--;
										}
								 else
										{
											FoodToBeCooked[0]++;
											FoodToBeCooked[2]++;
										}
								break;
				case 2: if( CustData[CustID].Food[1]<CookedFood[1]&&\
										CustData[CustID].Food[4] < CookedFood[4])
										{
										 CustFoodIsReady = 1;
										 CookedFood[1]--;
										 CookedFood[4]--;
										}
								else
										{
											FoodToBeCooked[1]++;
										}
								break;
				case 3: if( CustData[CustID].Food[0] <CookedFood[0]&&\
								CustData[CustID].Food[2] < CookedFood[2]&&\
								CustData[CustID].Food[4] < CookedFood[4])
										{
										 CustFoodIsReady = 1;
										 CookedFood[0]--;
										 CookedFood[2]--;
										 CookedFood[4]--;
										}
								else
										{
											FoodToBeCooked[0]++;
											FoodToBeCooked[2]++;
										}
								break;
				default: CustFoodIsReady = -1;
								 break;

				}/*end of switch*/
	Release(CookedFoodLock);
	Release(FoodToBeCookedUpdateLock);
	Release(CustDataLock);
	return(CustFoodIsReady);

}

void Waiter()
{
  int MyWaiterID=0;
  int NoOfCustServed=0;
	int i=0;
	int j=0;
	int k=0;
	Acquire(WaiterCountLock);
	MyWaiterID = WaiterCount++;
	Release(WaiterCountLock);
  /*Print1("Created %d Waiter\n", MyWaiterID);*/
   while(1)
     {
        /*Check if there are customers seated and thier food is ready
       Looping myself */
       Acquire(GrabedFoodLock);
       /*See for which customers food is grabed	 */
			 for(i=0;i<MAX_SIZE;i++)
			 {
				if(NoOfCustServed>0)
				{
					NoOfCustServed=0;
					break;
				}
				for(j=0;j<MAX_SIZE;j++)
				{
					if(GrabedFood[i][j]==1)
					{
						if(NoOfCustServed>0)
						{
							break;
						}
						
						Acquire(TableAllotedCustLock);
						for(k=0;k<MAX_SIZE;k++)		/* see if that customer is seated */
						{
							if(TableAllotedCustList[k]==j)
							{
								Acquire(NeedWaitersLock[j]);				/*Customer is ready to be served*/
								Signal(NeedWaitersCV[j],NeedWaitersLock[j]);
								Release(NeedWaitersLock[j]);
								Print3("Waiter [%d] got token number [%d] for Customer [%d]\n",MyWaiterID,j,j);
								Print2("Waiter [%d] validates the token number for Customer [%d]\n",MyWaiterID,j);
								Print2("Waiter [%d] serves food to Customer [%d]\n",MyWaiterID,j);
								TableAllotedCustList[k]=-1;
								/*Print2("TableAllotedCustList[%d]=%d\n",k,TableAllotedCustList[k]);*/
								GrabedFood[i][j]=-1;
								NoOfCustServed++;
								Acquire(TotalTablesUpdateLock);
								TotalTables++;
								Release(TotalTablesUpdateLock);
								break;
							}
						}
						Release(TableAllotedCustLock);
					}
				}
				if(NoOfCustServed>0)
				{
					NoOfCustServed=0;
					break;
				}
			}
			i=j=k=0;
			Release(GrabedFoodLock);
			for( i=0; i<20;i++)
			{
					/*currentThread->Yield();*/
			}

		/* There are no customers waiting, Waiters go on break */
			Acquire(WaitersQLock);
			Print1("Waiter[%d] going on break\n", MyWaiterID);
			Acquire(SleepingWaiterCountLock);
			SleepingWaiterCount++;
			Release(SleepingWaiterCountLock);
			Wait(WaitersCV,WaitersQLock);
			Acquire(SleepingWaiterCountLock);
			SleepingWaiterCount--;
			Release(SleepingWaiterCountLock);
			Release(WaitersQLock);
			Acquire(AliveCustCountLock);
			/*Print1("AliveCustCount%d\n",AliveCustCount);*/
			if(AliveCustCount==0)
			{
				/*Print1("Waiter %d is exiting\n",MyWaiterID);*/
				Release(AliveCustCountLock);
				Exit(1);
			}
			else
			{
				Print1(" Waiter[%d] returned from break\n", MyWaiterID);
				Release(AliveCustCountLock);
			}
     }/*end of while()*/
} /*end of waiter()	*/

void Manager()
{
  int MyMgrID=1;
  int MyCustID;
  int MyCookSpec; 
  int MyCookStat;
  int RmvedCookCnt=0;
  int RmvedCook[200]; 
	int a=0;
	int b=0;
	int c=0;
	int d=0;
	int i=0;
	int myOTId=PersonCount.NoOfOT;
	int CustFoodIsReady=0;
  while (1)
  {

    /*Manager should not take customers untill OT finished interaction with them*/
    Acquire(TblNeededCustLock);
		for(a=0;a<MAX_SIZE;a++)	   /*a-OTID b-CUSTID*/
		{
			for(b=0;b<MAX_SIZE;b++)
			{
				/*Print2("a=[%d]b=[%d] in for loop\n",a,b);
				Print1("TblNeededCustList[a][b]=%d outside for loop\n",TblNeededCustList[a][b]);*/
				if (TblNeededCustList[a][b]==1)
				{
					/*There are atleast some customer either waiting for food to be prepared or with food prepared and waiting for table*/
					Acquire(TotalTablesUpdateLock);
					MyCustID=b;
					/* There is a table for Customers
					Alert the customer about this*/
					if ( TotalTables>0 )
					{
						 TotalTables--;
						 Release(TotalTablesUpdateLock);
						 /*Update this table so that waiter is aware of seated customers*/
						 Acquire(TableAllotedCustLock);
						 for(c=0;c<MAX_SIZE;c++)
						 {
							if(TableAllotedCustList[c]==-1)
							{
								TableAllotedCustList[c]=MyCustID;
								TblNeededCustList[a][b]=0;
								/* There is a table for Customers
								Alert the customer about this*/
								Acquire(NeedTableLock[MyCustID]);
								Print2("Customer [%d] receives token number [%d] from the Manager\n",MyCustID,MyCustID);
								Print1("Customer[%d] is informed by the Manager - the restaurant is NOT full\n",MyCustID);
								Print1("Customer [%d] is seated at table\n",MyCustID);
								Signal(NeedTableCV[MyCustID],NeedTableLock[MyCustID]);
								Wait(NeedTableCV[MyCustID],NeedTableLock[MyCustID]);
								Release(NeedTableLock[MyCustID]);
								break;
							}
						 }
						Release(TableAllotedCustLock);
					}
					 /* if there are no tables, just give them the token number*/
					else
					 {
						/*pritnf("Customer [%d] receives token number [%d] from the Manager\n",MyCustID,MyCustID);*/
						Release(TotalTablesUpdateLock);
						Print1("Customer[%d] is informed by the Manager-the restaurant is  full\n",MyCustID);
						Print1("Customer [%d] is waiting to sit on the table\n", MyCustID);
					 }
				}
			}
    }
		a=b=c=0;
    Release(TblNeededCustLock);
        


		/*************************************************************
		* Manager as an OT START
		**************************************************************/
		i=0;
		for(i=0;i<200;i++)
		{
			Acquire(custLineLock);
			if(custLineLength>3*PersonCount.NoOfOT)
			{
				
			}
			Release(custLineLock);
		}
		
		/*************************************************************
		* Manager as an OT END
		**************************************************************/
		/* Manager as food packer*/
	
	
	Acquire(EatInCustWaitingForFoodLock);
	i=0;
	for(i=0;i<MAX_SIZE;i++)
	{
		if(EatInCustWaitingForFood[i]==1)
		{
			EatInCustWaitingForFood[i]=0;
			MyCustID=i;
			break;
		}
	}
	i=0;

	/*For (each) this customer, check if his order is ready for each type of food*/
	if(MyCustID!=-1)
		{
			CustFoodIsReady = IsCustomerFoodReady (MyCustID);
			 /* if CustFoodIsReady = 1, Food is ready and can be grabed*/
			 if (CustFoodIsReady==1)
				{
				 Acquire(GrabedFoodLock);
				 GrabedFood[myOTId][MyCustID]=1;/* 1 means food is grabed and 0 means otherwise*/
				 Release(GrabedFoodLock);
				 Print1("Manager packed food for Customer[%d]\n", MyCustID);
				 MyCustID=-1;
				 Release(EatInCustWaitingForFoodLock);
				}
			 else
				{
				 /*list of customers to whom token should be handled but shouldnt grab the food*/
				 /*this Q is as good as saying who gave token number to whom when they wanna wait*/
				  EatInCustWaitingForFood[MyCustID]=1;
					Print1("Manager gives token number to Customer[%d]\n", MyCustID);
					MyCustID=-1;
					Release(EatInCustWaitingForFoodLock);
				}
		}
	
	 else
		{
			Release(EatInCustWaitingForFoodLock);
			Acquire(ToGoCustWaitingForFoodLock);
			i=0;
			for(i=0;i<MAX_SIZE;i++)
			{
				if(ToGoCustWaitingForFood[i]==1)
				{
					ToGoCustWaitingForFood[i]=0;
					MyCustID=i;
					break;
				}
			}
			i=0;

			if(MyCustID!=-1)
			{
				CustFoodIsReady = IsCustomerFoodReady (MyCustID);
				 /* if CustFoodIsReady = 1, Food is ready and can be grabed*/
				 if (CustFoodIsReady==1)
					{
					 Acquire(GrabedFoodLock);
					 GrabedFood[myOTId][MyCustID]=1;/* 1 means food is grabed and 0 means otherwise*/
					 Release(GrabedFoodLock);
					 Print1("Manager packed the food for Customer[%d]\n", MyCustID);
					 MyCustID=-1;
					 Release(ToGoCustWaitingForFoodLock);
					}
				 else
					{
					 /*list of customers to whom token should be handled but shouldnt grab the food
					 /*this Q is as good as saying who gave token number to whom when they wanna wait*/
					  ToGoCustWaitingForFood[MyCustID]=1;
					  Print1("Manager gives token number to Customer[%d]\n",MyCustID);
						MyCustID=-1;
						Release(ToGoCustWaitingForFoodLock);
					}
			}
			else
			{
					Release(ToGoCustWaitingForFoodLock);
				/*Nothing to do
				/*Set OT status to FREE*/
			}
		}
			/*End of Manager as food packer*/

    
  /* Manager - Waiter Interaction */
  Acquire(TableAllotedCustLock);
		for(d=0;d<MAX_SIZE;d++)
		{
			if(-1!=TableAllotedCustList[d])	  /*List is not empty*/
			{
				Acquire(WaitersQLock);
				Broadcast(WaitersCV,WaitersQLock);
				Print("Manager calls back all Waiters from break\n");
				/*Print(" Manager broadcasted\n");*/
				Release(WaitersQLock);
				break;
			}
		}
		d=0;
    Release(TableAllotedCustLock);
		i=0;
		for (i=0;i<200;i++)
		{
			/*Print1("Manager yeilding%d\n",i);*/
			Yield();
		}
		i=0;
	/* Manager - Waiter Interaction End*/
	
	
		
		Acquire(AliveCustCountLock);
		if(AliveCustCount==0)
		{
			Release(AliveCustCountLock);
			Acquire(SleepingWaiterCountLock);
			/*Print1("Above while SleepingWaiterCount%d\n",SleepingWaiterCount);*/
			if (SleepingWaiterCount!=0)
			{
				while(SleepingWaiterCount!=0)
				{
					/*Print("inside while");*/
					Acquire(WaitersQLock);
					Broadcast(WaitersCV,WaitersQLock);
					Release(WaitersQLock);
					Release(SleepingWaiterCountLock);
					/*Print("Manager Broadcast\n");*/
					for (i=0;i<200;i++)
					{
						/*Print1("Manager yeilding%d\n",i);*/
						Yield();
					}
					Acquire(SleepingWaiterCountLock);
				}
			}
			Release(SleepingWaiterCountLock);
		}
		else
			Release(AliveCustCountLock);
			
		Acquire(AliveCustCountLock);
		if(AliveCustCount==0)
		{		
			Release(AliveCustCountLock);
			Acquire(ActiveOTCountLock);
			if(ActiveOTCount!=0)
			{
				/*Print1("Mgr waiting for OT, AcOT=%d\n",ActiveOTCount);*/
				while(ActiveOTCount!=0)
				{
					Release(ActiveOTCountLock);
					for (i=0;i<200;i++)
					{
						/*Print1("Manager yeilding%d\n",i);*/
						Yield();
					}
					
					Acquire(ActiveOTCountLock);
				}
			}
			Release(ActiveOTCountLock);
			if(DestroyAllLockCV==1)
			{
				/*Print1("SleepingWaiterCount%d\n",SleepingWaiterCount);*/
				Print("End of Simulation!!\n");
				Exit(1);
			}
			else
			{
				for (i=0;i<200;i++)
				{
					/*Print1("Manager yeilding%d\n",i);*/
					Yield();
				}
			}
		}
		else
		{
			Release(AliveCustCountLock);
		}
	}

    
    
}


int DestroyAllLocksCV()
{
	int iInitCntr=0;
	/*Delete all used locks and CVs*/
	DestroyLock(CustDataLock);
	DestroyLock(InventoryLock);
	DestroyLock(MgrCookStatUpdateLock);
	DestroyLock(InventoryLock);
	DestroyLock(SigCustListLock);
	DestroyLock(GrabedFoodLock);
	DestroyLock(EatInCustWaitingForFoodLock);
	DestroyLock(ToGoCustWaitingForFoodLock);
	DestroyLock(ToGoGrabedFoodLock);
	DestroyLock(MgrCookStyleLstLock);
	DestroyLock(MonitorAmtOfMoneyLock);
	DestroyLock(OTStatusUpdateLock);
	DestroyLock(CookedFoodLock); 
	DestroyLock(TableAllotedCustLock); 
	DestroyLock(WaitersQLock); 
	DestroyLock(TotalTablesUpdateLock);
	DestroyLock(TblNeededCustLock); 
	DestroyLock(FoodToBeCookedUpdateLock);
	DestroyLock(CustCountLock);
	DestroyLock(OTCountLock);
	DestroyLock(WaiterCountLock);
	DestroyLock(custLineLock);
	DestroyLock(AliveCustCountLock);
	DestroyLock(SleepingWaiterCountLock);
	DestroyLock(ActiveOTCountLock);
	

	iInitCntr=0;
	for(iInitCntr=0;iInitCntr<MAX_SIZE;iInitCntr++)
	{
	  DestroyLock(NeedInventoryLock[iInitCntr]);
	  DestroyLock(OrderTakerLock[iInitCntr]);
	  DestroyLock(NeedTableLock[iInitCntr]);
	  DestroyLock(NeedWaitersLock[iInitCntr]);
	}
	/*Print1("MAX LOCK COUNT =%d\n",NeedWaitersLock[iInitCntr]);*/
	iInitCntr=0;

	/***************************************************************************************
	Intialization of all Condition Variables
	***************************************************************************************/

	DestroyCondition(CookQCV);  
	DestroyCondition(custWaitingCV);
	DestroyCondition(WaitersCV);
	iInitCntr=0;
	for(iInitCntr=0;iInitCntr<MAX_SIZE;iInitCntr++)
	{
	  DestroyCondition(OTWaitingCV[iInitCntr]);
	  DestroyCondition(NeedInventoryCV[iInitCntr]); 
	  DestroyCondition(NeedTableCV[iInitCntr]);
	  DestroyCondition(NeedWaitersCV[iInitCntr]);
	}
	iInitCntr=0;
	return 1;
	
}