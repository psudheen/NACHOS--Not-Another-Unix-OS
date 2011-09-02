#include "syscall.h"
#ifndef NULL
#define NULL    0
#endif
#define LOCK_BLOCK 1
#define NO_OF_OT 				3
#define NO_OF_CUST 			10
#define NO_OF_WAITERS 	2
#define NO_OF_COOKS			4
#define MIN_REQUIRED_INVENTORY 100/* = 20 customers * 5 types of food*/


/*All #Structures*/
typedef struct 
{
 int CustID;
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

typedef struct
{
int cookStatus;
int foodType;
}cookStruct;


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
void waiter();

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
    /***************************************************************************************
    Intialization of Global Counters
    ***************************************************************************************/
int NoOfEatIn=0;
    int NoOfToGo=0;
    int iInitCntr=0;
    int jInitCntr=0;
    int iOTCtr=0;
    int iCustCtr=0;


    WaiterCount=CreateMV("WC",2);
   
    TotalTables=CreateMV("TT",2);


/***************************************************************************************
    Creation of Global Arrays
    ***************************************************************************************/


	TableAllotedCustList[0]=CreateMV("TAC0",4);
	TableAllotedCustList[1]=CreateMV("TAC1",4);
	TableAllotedCustList[2]=CreateMV("TAC2",4);
	TableAllotedCustList[3]=CreateMV("TAC3",4);
	TableAllotedCustList[4]=CreateMV("TAC4",4);
	TableAllotedCustList[5]=CreateMV("TAC5",4);
	TableAllotedCustList[6]=CreateMV("TAC6",4);
	TableAllotedCustList[7]=CreateMV("TAC7",4);
	TableAllotedCustList[8]=CreateMV("TAC8",4);
	TableAllotedCustList[9]=CreateMV("TAC9",4);


		GrabedFood[0][0]=CreateMV("GF00",4);
		GrabedFood[0][1]=CreateMV("GF01",4);
		GrabedFood[0][2]=CreateMV("GF02",4);
		GrabedFood[0][3]=CreateMV("GF03",4);
		GrabedFood[0][4]=CreateMV("GF04",4);
		GrabedFood[0][5]=CreateMV("GF05",4);
		GrabedFood[0][6]=CreateMV("GF06",4);
		GrabedFood[0][7]=CreateMV("GF07",4);
		GrabedFood[0][8]=CreateMV("GF08",4);
		GrabedFood[0][9]=CreateMV("GF09",4);
		
		GrabedFood[1][0]=CreateMV("GF10",4);
		GrabedFood[1][1]=CreateMV("GF11",4);
		GrabedFood[1][2]=CreateMV("GF12",4);
		GrabedFood[1][3]=CreateMV("GF13",4);
		GrabedFood[1][4]=CreateMV("GF14",4);
		GrabedFood[1][5]=CreateMV("GF15",4);
		GrabedFood[1][6]=CreateMV("GF16",4);
		GrabedFood[1][7]=CreateMV("GF17",4);
		GrabedFood[1][8]=CreateMV("GF18",4);
		GrabedFood[1][9]=CreateMV("GF19",4);
		
		GrabedFood[2][0]=CreateMV("GF20",4);
		GrabedFood[2][1]=CreateMV("GF21",4);
		GrabedFood[2][2]=CreateMV("GF22",4);
		GrabedFood[2][3]=CreateMV("GF23",4);
		GrabedFood[2][4]=CreateMV("GF24",4);
		GrabedFood[2][5]=CreateMV("GF25",4);
		GrabedFood[2][6]=CreateMV("GF26",4);
		GrabedFood[2][7]=CreateMV("GF27",4);
		GrabedFood[2][8]=CreateMV("GF28",4);
		GrabedFood[2][9]=CreateMV("GF29",4);
		
		
	NeedWaitersLock[0]=CreateLock("NW0",3);
	NeedWaitersLock[1]=CreateLock("NW1",3);
	NeedWaitersLock[2]=CreateLock("NW2",3);
	NeedWaitersLock[3]=CreateLock("NW3",3);
	NeedWaitersLock[4]=CreateLock("NW4",3);
	NeedWaitersLock[5]=CreateLock("NW5",3);
	NeedWaitersLock[6]=CreateLock("NW6",3);
	NeedWaitersLock[7]=CreateLock("NW7",3);
	NeedWaitersLock[8]=CreateLock("NW8",3);
	NeedWaitersLock[9]=CreateLock("NW9",3);
	
	NeedWaitersCV[0]=CreateCondition("WC0",3);
	NeedWaitersCV[1]=CreateCondition("WC1",3);
	NeedWaitersCV[2]=CreateCondition("WC2",3);
	NeedWaitersCV[3]=CreateCondition("WC3",3);
	NeedWaitersCV[4]=CreateCondition("WC4",3);
	NeedWaitersCV[5]=CreateCondition("WC5",3);
	NeedWaitersCV[6]=CreateCondition("WC6",3);
	NeedWaitersCV[7]=CreateCondition("WC7",3);
	NeedWaitersCV[8]=CreateCondition("WC8",3);
	NeedWaitersCV[9]=CreateCondition("WC9",3);

Print("End of createMV in OT\n");
    /***************************************************************************************
    Intialization of Global Arrays
    ***************************************************************************************/






    Print("Before locks\n");
    /***************************************************************************************
    Intialization of locks
    ***************************************************************************************/
   
    GrabedFoodLock=CreateLock("G1", 2);
  
    WaitersQLock=CreateLock("WQ", 2); 
	 WaitersCV=CreateCondition("WJ",2);
    TotalTablesUpdateLock=CreateLock("TJ",2);

    WaiterCountLock=CreateLock("WZ",2);


Print("End of createLocks in OT\n");
    /***************************************************************************************
    Intialization of all Condition Variables
    ***************************************************************************************/

    Print("End of createCC in OT\n");    
    PersonCount.NoOfCust=NO_OF_CUST;
    PersonCount.NoOfOT=NO_OF_OT;
    PersonCount.NoOfWaiters=NO_OF_WAITERS;
    PersonCount.NoOfCooks=NO_OF_COOKS;
	
/* 	
	SetMV(GrabedFood[0][0],0);
	SetMV(GrabedFood[0][1],0);
	SetMV(GrabedFood[0][3],0);
	SetMV(GrabedFood[0][4],0); */
	
	
	/* SetMV(TableAllotedCustList[0],1);
	SetMV(TableAllotedCustList[1],3);
	SetMV(TableAllotedCustList[2],4);
	SetMV(TableAllotedCustList[3],0); */
	waiter();
} 


void waiter()
{
				int MyWaiterID=0;
				int NoOfCustServed=0;
				int i=0;
				int j=0;
				int k=0;
				Print("waiter is starting \n");
				Acquire(WaiterCountLock);
				MyWaiterID = GetMV(WaiterCount);
				SetMV(WaiterCount,MyWaiterID+1);
				Release(WaiterCountLock);
				  /*Print1("Created %d Waiter\n", MyWaiterID);*/
				while(1)
				{
						/*Check if there are customers seated and thier food is ready
					   Looping myself */
					   
					   /*See for which customers food is grabed */
						for(i=0;i<3;i++)
						{
								if(NoOfCustServed>0)
								{
									NoOfCustServed=0;
									break;
								}
								for(j=0;j<10;j++)
								{
									/* Print3("GetMV(GrabedFood[%d][%d])=%d \n",i,j,GetMV(GrabedFood[i][j])); */
									Acquire(GrabedFoodLock);
									if(GetMV(GrabedFood[i][j])==1)
									{
										Release(GrabedFoodLock);
										if(NoOfCustServed>0)
										{
											break;
										}
										
										for(k=0;k<10;k++) /* see if that customer is seated */
										{
											  Print2("the value of GetMV(TableAllotedCustList[%d]) = %d \n",k,GetMV(TableAllotedCustList[k]));  
											Acquire(TableAllotedCustLock);
											if(GetMV(TableAllotedCustList[k])==j)
											{
												Print1("the k value is %d \n",k);
												SetMV(TableAllotedCustList[k],99);
												Release(TableAllotedCustLock);
												Acquire(NeedWaitersLock[j]); /*Customer is ready to be served*/
												Signal(NeedWaitersCV[j],NeedWaitersLock[j]);
												Release(NeedWaitersLock[j]);
												
												Print3("WAITER [%d] GOT TOKEN NUMBER [%d] FOR CUSTOMER [%d]\n",MyWaiterID,j,j);
												Print2("WAITER [%d] VALIDATES THE TOKEN NUMBER FOR CUSTOMER [%d]\n",MyWaiterID,j);
												Print2("Waiter [%d] SERVES FOOD TO CUSTOMER [%d]\n",MyWaiterID,j);
												
												/*Print2("TableAllotedCustList[%d]=%d\n",k,TableAllotedCustList[k]);*/
												Acquire(GrabedFoodLock);
												SetMV(GrabedFood[i][j],99);
												Release(GrabedFoodLock);
												NoOfCustServed++;
												Acquire(TotalTablesUpdateLock);
												SetMV(TotalTables,GetMV(TotalTables)+1);
												Release(TotalTablesUpdateLock);
												break;
											}
											else
											{
												Release(TableAllotedCustLock);
											}
										}
										
									}
									else
									{
										Release(GrabedFoodLock);
									}
								}
								if(NoOfCustServed>0) 
								{
										NoOfCustServed=0;
										break;
								}
						}
							
						
					

						/* There are no customers waiting, Waiters go on break */
						Acquire(WaitersQLock);
						Print1("WAITER[%d] GOING ON BREAK\n", MyWaiterID);
						Wait(WaitersCV,WaitersQLock);
						Print1("WAITER[%d] IS OUT OF BREAK\n", MyWaiterID);
						Release(WaitersQLock);
					
				}/*end of while()*/
} /*end of waiter() */