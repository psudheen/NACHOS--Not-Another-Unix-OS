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
 int CustID;
 int TypeOfFood;
 int Food[5];
}CustomerData_t;


typedef struct
{
int cookStatus;
int foodType;
}cookStruct;

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
void OrderTaker();
void Init_CustomerData( int TypeOfFood, int CustID);
int IsCustomerFoodReady (int CustID);
void CarlJrSimulation (void);
void cook();

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
int SleepingWaiterCountLock;
int ActiveOTCountLock;

/*** new addition */


int waitingCookCV;
int cookStatus;
int cookInfoLock[4];
int foodToBeCooked;
int rawMaterialMV[4];
int cookIdLock;
int InitCookLock;
int minCookedLevelMV;
int rawmaterialLock;
int waitingCookLock;
int InitCookCV;
int cookIdMV;

cookStruct cookInfoMV[5];   /*hardcoded */

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
Print("Entering OT thread\n");
Print("Waiting for network setup\n");
StartSimulation();
Print("Ready to execute\n");
  CarlJrSimulation();
  
}/* end of main()*/



void CarlJrSimulation (void)
{






CookedFood[0]=CreateMV("CF0",3);
CookedFood[1]=CreateMV("CF1",3);
CookedFood[2]=CreateMV("CF2",3);
CookedFood[3]=CreateMV("CF3",3);
CookedFood[4]=CreateMV("CF4",3);


Print("End of createMV in COOK\n");

  

       
 



	
rawMaterialMV[0]=CreateMV("raMA1",5);
rawMaterialMV[1]=CreateMV("raMA2",5);
rawMaterialMV[2]=CreateMV("raMA3",5);
rawMaterialMV[3]=CreateMV("raMA4",5);




cookInfoMV[0].cookStatus=CreateMV("ckst1",5);



cookInfoMV[0].foodType=CreateMV("ckft1",5);





cookInfoLock[0]=CreateLock("ckin1",5);

rawmaterialLock=CreateLock("rwlk",4);
CookedFoodLock=CreateLock("ckfl",4);

InitCookLock=CreateLock("InCkLk",6);
waitingCookLock=CreateLock("wtgCkL",6);

InitCookCV=CreateCondition("InitCk",6);
waitingCookCV=CreateCondition("wtgCk",5);

cookIdLock=CreateLock("ckIdLk",6);

minCookedLevelMV=CreateMV("mnCkdL",6);

cookIdMV=CreateMV("cookId",6);

Print("all the creations are done");
cook();
} 

void cook()
{
int myID,j,i;
int foodToBeCooked;
int cookStatus;
int inv;

Print("entreing cook code========================== \n");
Print1("the cookid index is %d \n",cookIdLock);
Acquire(cookIdLock);
 
myID=GetMV(cookIdMV);                /*IDing the cook*/
Print1("my cook id is ------------------ > %d \n",myID);
SetMV(cookIdMV,myID+1);
 Release(cookIdLock);

Acquire(cookInfoLock[myID]);
SetMV(cookInfoMV[myID].cookStatus,1); /*setting the information of the cook to default values */
SetMV(cookInfoMV[myID].foodType,9);
Release(cookInfoLock[myID]);

Acquire(InitCookLock);
Print("cook on wait \n");
Wait(InitCookCV,InitCookLock);
Release(InitCookLock);


while(1)
{
	Acquire(cookInfoLock[myID]);
	Print1("cookInfoLock --- cook %d\n",myID);
	if((cookStatus=GetMV(cookInfoMV[myID].cookStatus))==0)
	{
/*my status is set and my food type is set by the manager so i have to cook */
 
/* SetMV(foodtobecooked[myID],cookInfoMV[myid].foodType); */
 
	foodToBeCooked=GetMV(cookInfoMV[myID].foodType);
	Print1("cookInfoLock RE---- cook  %d\n",myID);
    Release(cookInfoLock[myID]);
	
/* 	Acquire(waitingCookLock);
	Print1("waitingCookLock ---- cook %d\n",myID);
	Signal(waitingCookCV,waitingCookLock);
	Print1("waitingCookLock RE----- cook %d\n",myID);
            Release(waitingCookLock); */
						for(i=0;i<50;i++)
					{
									  Yield();
					}

					/*updating the inventory and Raw materials --------------------------------*/
					/* for(i=0;i<4;i++)
					{
					if(foodToBecooked==i)
					{ */
					Print2("THE FOOD TO BE COOKED BY COOK %d is %d \n",myID,foodToBeCooked);
					Acquire(CookedFoodLock);
					Print1("CookedFoodLock ---- cook %d\n",myID);
					Print1("the food value entry is %d \n",GetMV(CookedFood[foodToBeCooked]));
					SetMV(CookedFood[foodToBeCooked],GetMV(CookedFood[foodToBeCooked])+25);
					Print1("THE FOOD VALUE ENTRY AFTER COOKING IS %d \n",GetMV(CookedFood[foodToBeCooked]));
					Print1("CookedFoodLock RE --- cook %d\n",myID);
					Release(CookedFoodLock);
					Acquire(rawmaterialLock);
					Print1("rawmaterialLock ---- cook %d\n",myID);
					SetMV(rawMaterialMV[foodToBeCooked],GetMV(rawMaterialMV[foodToBeCooked])-2);
					Print("THE COOK HAS UPDATED THE INVENTORY\n");
					Release(rawmaterialLock);
					/* break; */
					/* } */
					/* } */
					/* for(i=0;i<4;i++)
					{ */
					
					Acquire(CookedFoodLock);
					Print("CookedFoodLock ---- cook \n");
					if((inv=GetMV(CookedFood[foodToBeCooked]))>=GetMV(minCookedLevelMV))
					{
					Print("CookedFoodLock RE---- cook \n");
					Release(CookedFoodLock);
					Acquire(cookInfoLock[myID]);
					Print("cookInfoLock ---- cook \n");
					SetMV(cookInfoMV[myID].foodType,9);
					SetMV(cookInfoMV[myID].cookStatus,1);
					Print("cookInfoLock RE---- cook \n");
					Release(cookInfoLock[myID]); 
					Acquire(InitCookLock);
					Print("InitCookLock ---- cook \n");
					Wait(InitCookCV,InitCookLock);
					Print("InitCookLock RE ---- cook \n");
					Release(InitCookLock);
					}
					else 
					{
					Print("CookedFoodLock RE ---- cook \n");
					
					Release(CookedFoodLock);
					}
					/* } */
	}
	else
	{
	Print("cookInfoLock RE---- cook \n");
		Release(cookInfoLock[myID]);
		Acquire(InitCookLock);
		Print(" InitCookLock ---- cook \n");
		Wait(InitCookCV,InitCookLock);
		Print(" InitCookLock RE---- cook \n");
		Release(InitCookLock);
	}

}
Exit(0);
}