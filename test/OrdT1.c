#include "syscall.h"
#ifndef NULL
#define NULL    0
#endif
#define LOCK_BLOCK 1
#define NO_OF_OT 				2
#define NO_OF_CUST 			4
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
int incomeLock;
int	restoIncomeMV;

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
		SigCustList[0]=CreateMV("S1",2);
		SigCustList[1]=CreateMV("S2",2);
		
		CustData[0].TypeOfFood=CreateMV("CD0",3);
		CustData[1].TypeOfFood=CreateMV("CD1",3);
		CustData[2].TypeOfFood=CreateMV("CD2",3);
		CustData[3].TypeOfFood=CreateMV("CD3",3);

		
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
		
		CustData[2].Food[0]=CreateMV("CDB0",4);
		CustData[2].Food[1]=CreateMV("CDB1",4);
		CustData[2].Food[2]=CreateMV("CDB2",4);
		CustData[2].Food[3]=CreateMV("CDB3",4);
		CustData[2].Food[4]=CreateMV("CDB4",4);
		
		CustData[3].Food[0]=CreateMV("CDC0",4);
		CustData[3].Food[1]=CreateMV("CDC1",4);
		CustData[3].Food[2]=CreateMV("CDC2",4);
		CustData[3].Food[3]=CreateMV("CDC3",4);
		CustData[3].Food[4]=CreateMV("CDC4",4);
		

		
		
				
		EatInCustWaitingForFood[0]=CreateMV("EC0",3);
		EatInCustWaitingForFood[1]=CreateMV("EC1",3);
		EatInCustWaitingForFood[2]=CreateMV("EC2",3);
		EatInCustWaitingForFood[3]=CreateMV("EC3",3);
		
		
		ToGoCustWaitingForFood[0]=CreateMV("TC0",3);
		ToGoCustWaitingForFood[1]=CreateMV("TC1",3);
		ToGoCustWaitingForFood[2]=CreateMV("TC2",3);
		ToGoCustWaitingForFood[3]=CreateMV("TC3",3);
		
		
		ToGoGrabedFood[0]=CreateMV("TG0",3);
		ToGoGrabedFood[1]=CreateMV("TG1",3);
		ToGoGrabedFood[2]=CreateMV("TG2",3);
		ToGoGrabedFood[3]=CreateMV("TG3",3);
		
		
		OrderTakerStatus[0]=CreateMV("OTS0",4);
		OrderTakerStatus[1]=CreateMV("OTS1",4);
		
		CookedFood[0]=CreateMV("CF0",3);
		CookedFood[1]=CreateMV("CF1",3);
		CookedFood[2]=CreateMV("CF2",3);
		CookedFood[3]=CreateMV("CF3",3);
		CookedFood[4]=CreateMV("CF4",3);
		

		/**/
		#ifdef LOCK_BLOCK
		GrabedFood[0][0]=CreateMV("GF00",4);
		GrabedFood[0][1]=CreateMV("GF01",4);
	GrabedFood[0][2]=CreateMV("GF02",4);
		GrabedFood[0][3]=CreateMV("GF03",4);
		GrabedFood[1][0]=CreateMV("GF10",4);
		GrabedFood[1][1]=CreateMV("GF11",4);
		GrabedFood[1][2]=CreateMV("GF12",4);
		GrabedFood[1][3]=CreateMV("GF13",4);

		#endif
		
		
		Print("End of createMV in OT\n");
    /***************************************************************************************
    Intialization of Global Arrays
    ***************************************************************************************/
			Print("For loop2 of createMV in OT\n");
    iInitCntr=0;
    jInitCntr=0;
    /*CookStyle[4][40]={"$6 Burger","$3 Burger","Veggie Burger","French Fries"};*/
		SetMV(CookedFood[0],100);
		SetMV(CookedFood[1],100);
		SetMV(CookedFood[2],100);
		SetMV(CookedFood[3],100);
		SetMV(CookedFood[4],100000);
		
		
		
		SetMV(OrderTakerStatus[0],BUSY);
		SetMV(OrderTakerStatus[1],BUSY);
		Print("For loop3 of createMV in OT\n");
    iInitCntr=0;

    Print("Before locks\n");
    /***************************************************************************************
    Intialization of locks
    ***************************************************************************************/
    CustDataLock=CreateLock("C1", 2);
    SigCustListLock=CreateLock("S1", 2);
    GrabedFoodLock=CreateLock("G1", 2);
    EatInCustWaitingForFoodLock=CreateLock("E1", 2);
    ToGoCustWaitingForFoodLock=CreateLock("TQ", 2);
    ToGoGrabedFoodLock=CreateLock("TW", 2);
    
    OTStatusUpdateLock=CreateLock("OX", 2);
    CookedFoodLock=CreateLock("CX", 2); 
    CustCountLock=CreateLock("CJ", 2);
    OTCountLock=CreateLock("OZ", 2);

    custLineLock=CreateLock("CD",2);

		

		/* //OrderTakerLock[iInitCntr]=CreateLock("OrderTakerLock",14); */
		OrderTakerLock[0]=CreateLock("OL0",3);
			OrderTakerLock[1]=CreateLock("OL1",3);
		

		Print("End of createLocks in OT\n");
    /***************************************************************************************
    Intialization of all Condition Variables
    ***************************************************************************************/

    CookQCV=CreateCondition("CQ",2);  
    custWaitingCV=CreateCondition("CB",2);
    WaitersCV=CreateCondition("WJ",2);
		
		
		OTWaitingCV[0]=CreateCondition("OC0",3);
		OTWaitingCV[1]=CreateCondition("OC1",3);
		
		
		
		 	incomeLock=CreateLock("incLck",6);
		restoIncomeMV=CreateMV("rstInc",6);
		
 
		/**/
		Print("End of createCC in OT\n");    
    PersonCount.NoOfCust=NO_OF_CUST;
    PersonCount.NoOfOT=NO_OF_OT;
    PersonCount.NoOfWaiters=NO_OF_WAITERS;
    PersonCount.NoOfCooks=NO_OF_COOKS;	
		OrderTaker();
} 


void OrderTaker()
{
	int MyCustID = -1;
  int custFoodIsReady =-1;
  int Poped_CID = -1;
  int Poped_OT = -1;
	int i=0;
	int myOTId=0;
	int Money=0;
	Acquire(OTCountLock);
	myOTId=GetMV(OTcount);
	SetMV(OTcount,myOTId+1);
	Release(OTCountLock);
	Print1("Created %d OT\n", myOTId);
	
	while(1)
	{
		Acquire(custLineLock);
	 Print1(" 111 customer line length = %d \n",GetMV(custLineLength)); 
		/*Are there any waiting customers?*/
		if((GetMV(custLineLength)) > 0)
		{
			Signal(custWaitingCV,custLineLock);
			SetMV(custLineLength,(GetMV(custLineLength)-1));
			Print1("  customer line length = %d \n",GetMV(custLineLength)); 
			/* Acquire(OTStatusUpdateLock); */
			SetMV(OrderTakerStatus[myOTId],WAITING);         /*waiting */
		/* 	Release(OTStatusUpdateLock); */
		}
		/*Is there any food to bag?*/
		else 
		{
		
			Acquire(EatInCustWaitingForFoodLock);
			for(i=0;i<NO_OF_CUST;i++)
			{
				if((GetMV(EatInCustWaitingForFood[i]))==1)
				{
					SetMV(EatInCustWaitingForFood[i],0);
					MyCustID=i;
					
					break;
				}
			}
			Release(EatInCustWaitingForFoodLock);
	/*For (each) this customer, check if his order is ready for each type of food*/
			if(MyCustID!=-1)
			{
				custFoodIsReady = IsCustomerFoodReady (MyCustID);
				/* if CustFoodIsReady = 1, Food is ready and can be grabed*/
				if (custFoodIsReady==1)
				{
					Acquire(GrabedFoodLock);
					SetMV(GrabedFood[myOTId][MyCustID],1);/* 1 means food is grabed and 0 means otherwise*/
					Release(GrabedFoodLock);
					Print2("ORDERTAKER[%d] PACKED THE FOOD FOR CUSTOMER[%d]\n",myOTId, MyCustID);
					MyCustID=-1;
				}

				else
				{
					/*list of customers to whom token should be handled but shouldnt grab the food*/
					/*this Q is as good as saying who gave token number to whom when they wanna wait*/
					Acquire(EatInCustWaitingForFoodLock);
					SetMV(EatInCustWaitingForFood[MyCustID],1);
					Release(EatInCustWaitingForFoodLock);
					Print2("1ORDERTAKER[%d] GIVES TOKEN NUMBER TO CUSTOMER[%d]\n",myOTId,MyCustID);
					MyCustID=-1;
				}
			}
			else
			{
				Acquire(ToGoCustWaitingForFoodLock);
				for(i=0;i<NO_OF_CUST;i++)
				{
					if((GetMV(ToGoCustWaitingForFood[i]))==1)
					{
						SetMV(ToGoCustWaitingForFood[i],0); 
						MyCustID=i;
						break;
					}
				}
				Release(ToGoCustWaitingForFoodLock);  
				if(MyCustID!=-1)
				{ 
					custFoodIsReady = IsCustomerFoodReady (MyCustID);
					/* if CustFoodIsReady = 1, Food is ready and can be grabed*/
					if (custFoodIsReady==1)
					{
						Acquire(GrabedFoodLock);
						SetMV(GrabedFood[myOTId][MyCustID],1);/* 1 means food is grabed and 0 means otherwise*/
						Release(GrabedFoodLock);
						Print2("ORDERTAKER[%d] PACKED THE FOOD FOR CUSTOMER[%d]\n",myOTId, MyCustID);
						MyCustID=-1;
					}
					else
					{
						/*list of customers to whom token should be handled but shouldnt grab the food
						/*this Q is as good as saying who gave token number to whom when they wanna wait*/
						Acquire(ToGoCustWaitingForFoodLock);  
						SetMV(ToGoCustWaitingForFood[MyCustID],1);
						Print2("2ORDERTAKER[%d] GIVES TOKEN NUMBER TO CUSTOMER[%d]\n",myOTId, MyCustID);
						MyCustID=-1;
						Release(ToGoCustWaitingForFoodLock); 
					}
				}
				else
				{
					/*Nothing to do
					/*Set OT status to FREE*/
					Print("OT is free\n");
					/* Acquire(OTStatusUpdateLock); */
					Print("OT1 is free\n");
					Print1("OT free == %d\n",GetMV(OrderTakerStatus[myOTId]));
					SetMV(OrderTakerStatus[myOTId],FREE);
					/* Release(OTStatusUpdateLock); */
				}
			}
		}

		Acquire(OrderTakerLock[myOTId]);
		Release(custLineLock);
		/*release hold on CustLineLen
		/*OT should wait until customer signals him with orders*/
		Print3(" %d OT waiting for customers to signal on CVID %d  LockID %d\n", myOTId,OTWaitingCV[myOTId],OrderTakerLock[myOTId]);
		Wait(OTWaitingCV[myOTId],OrderTakerLock[myOTId]);                

		MyCustID=GetMV(SigCustList[myOTId]);  
		
		Acquire(incomeLock);
		Money = (6 * GetMV(CustData[MyCustID].Food[0]) + 3 * GetMV(CustData[MyCustID].Food[1]) + 4 * GetMV(CustData[MyCustID].Food[2]) + 2 * GetMV(CustData[MyCustID].Food[3])+ GetMV(CustData[MyCustID].Food[4]));
		SetMV(restoIncomeMV,Money);
		
		Release(incomeLock);
	  Print2("ORDERTAKER[%d] IS TAKING ORDER OF THE CUSTOMER [%d]\n", myOTId, MyCustID);

		if((GetMV(CustData[MyCustID].TypeOfFood)) == 0)
		{
			Release(OrderTakerLock[myOTId]);
			/*Eat - in customers
			/*If type of food is only Soda, token is handled
			/*and add to GrabedFood*/
			if( MyCustID%5==4)
			{
				/*Update the list which waiter will look to serve customers*/
				Acquire(GrabedFoodLock);
				SetMV(GrabedFood[myOTId][MyCustID],1);
				Release(GrabedFoodLock);
			}
			/* Customers with multiple orders*/
			else
			{
				/*For (each) this customer, check if his order is ready for each type of food*/
				custFoodIsReady = IsCustomerFoodReady (MyCustID);
				/* if CustFoodIsReady = 1, Food is ready and can be grabed*/
				if (custFoodIsReady=1)
				{
					Acquire(GrabedFoodLock);
					SetMV(GrabedFood[myOTId][MyCustID],1);
					Release(GrabedFoodLock);
					Print2("3ORDERTAKER[%d] GIVES TOKEN NUMBER TO CUSTOMER[%d]\n",myOTId, MyCustID);
				}
				else
				{
					/*list of customers to whom token should be handled but shouldnt grab the food
					/*this Q is as good as saying who gave token number to whom when they wanna wait*/
					Acquire(EatInCustWaitingForFoodLock);
					SetMV(EatInCustWaitingForFood[MyCustID],1);
					Release(EatInCustWaitingForFoodLock);
					Print2("4ORDERTAKER[%d] GIVES TOKEN NUMBER TO CUSTOMER[%d]\n",myOTId, MyCustID);
					/* Print1(" 222 customer line length = %d \n",GetMV(custLineLength)); */
					
				}
			}

		}
		/* To go customers */
		else
		{
			Release(OrderTakerLock[myOTId]);
			if( MyCustID%5 == 4)/* If customer needs only Soda*/
			{
				/*Signal this waiting customer and "wait" for him to acknowledge*/
				/*If acknowledged, signal the waiting customer and Customer threads finishes*/
			}
			/* Customers with multiple orders*/
			else
			{
				/*For (each) this customer, check if his order is ready for each type of food*/
				custFoodIsReady = IsCustomerFoodReady (MyCustID);
				/* if CustFoodIsReady = 1, Food is ready and can be grabed*/
				if (custFoodIsReady=1)
				{
					Acquire(ToGoGrabedFoodLock);
					SetMV(ToGoGrabedFood[MyCustID],1);
					Release(ToGoGrabedFoodLock);
					Print2("ORDERTAKER[%d] PACKED THE FOOD FOR CUSTOMER[%d]\n",myOTId, MyCustID);
				}
				else
				{
					/*list of customers to whom token should be handled but shouldnt grab the food*/
					/*this Q is as good as saying who gave token number to whom when they wanna wait*/
					Acquire(ToGoCustWaitingForFoodLock);
					SetMV(ToGoCustWaitingForFood[MyCustID],1);
					Release(ToGoCustWaitingForFoodLock);
					Print2("5ORDERTAKER[%d] GIVES TOKEN NUMBER TO CUSTOMER[%d]\n",myOTId, MyCustID);
				}
			}

		}
		/*Order processed Signal the customer to get out of sleep!*/
		Acquire(OrderTakerLock[myOTId]);
		Signal(OTWaitingCV[myOTId],OrderTakerLock[myOTId]);
		Release(OrderTakerLock[myOTId]);
		Print1(" 333 customer line length = %d \n",GetMV(custLineLength));
	}/*end of while()*/
}

int IsCustomerFoodReady (int CID)
{
	int CustFoodIsReady = 0;
	int CustID = CID;

	Acquire(CookedFoodLock);
	switch(CustID%5)
	{
	case 0: if ( GetMV(CustData[CustID].Food[0])< GetMV(CookedFood[0] )&&
	GetMV(CustData[CustID].Food[1])< GetMV(CookedFood[1]) &&
	GetMV(CustData[CustID].Food[2]) < GetMV(CookedFood[2]) && 
	GetMV(CustData[CustID].Food[3])< GetMV(CookedFood[3]) && 
	GetMV(CustData[CustID].Food[4]) < GetMV(CookedFood[4]))
	{
		CustFoodIsReady = 1;
		SetMV(CookedFood[0],(GetMV(CookedFood[0])-1));
		SetMV(CookedFood[1],(GetMV(CookedFood[1])-1));
		SetMV(CookedFood[2],(GetMV(CookedFood[2])-1));
		SetMV(CookedFood[3],(GetMV(CookedFood[3])-1));
		SetMV(CookedFood[4],(GetMV(CookedFood[4])-1));
	}
	else
	{
		SetMV(FoodToBeCooked[0],(GetMV(FoodToBeCooked[0])+1));
		SetMV(FoodToBeCooked[1],(GetMV(FoodToBeCooked[1])+1));
		SetMV(FoodToBeCooked[2],(GetMV(FoodToBeCooked[2])+1));
		SetMV(FoodToBeCooked[3],(GetMV(FoodToBeCooked[3])+1));
	
	}
	break;
	case 1: if ( GetMV(CustData[CustID].Food[0]) < GetMV(CookedFood[0])&&
	GetMV(CustData[CustID].Food[2]) < GetMV(CookedFood[2])&&
	GetMV(CustData[CustID].Food[4]) < GetMV(CookedFood[4]))
	{
		 CustFoodIsReady = 1;
		 SetMV(CookedFood[0],(GetMV(CookedFood[0])-1));
		 SetMV(CookedFood[2],(GetMV(CookedFood[2])-1));
		 SetMV(CookedFood[4],(GetMV(CookedFood[4])-1));
	
	}
	 else
	{
		SetMV(FoodToBeCooked[0],(GetMV(FoodToBeCooked[0])+1));
		SetMV(FoodToBeCooked[2],(GetMV(FoodToBeCooked[2])+1));
	}
	break;
	case 2: if( GetMV(CustData[CustID].Food[1])<GetMV(CookedFood[1])&&
	GetMV(CustData[CustID].Food[4])< GetMV(CookedFood[4]))
	{
	 CustFoodIsReady = 1;
	 SetMV(CookedFood[1],(GetMV(CookedFood[1])-1));
	 SetMV(CookedFood[4],(GetMV(CookedFood[4])-1));
	}
	else
	{
     SetMV(FoodToBeCooked[1],(GetMV(FoodToBeCooked[1])+1));
	}
	break;
	case 3: if( GetMV(CustData[CustID].Food[0]) <GetMV(CookedFood[0])&&
	GetMV(CustData[CustID].Food[2]) < GetMV(CookedFood[2])&&
	GetMV(CustData[CustID].Food[4]) < GetMV(CookedFood[4]))
	{
		 CustFoodIsReady = 1;
		SetMV(CookedFood[0],(GetMV(CookedFood[0])-1));
		SetMV(CookedFood[2],(GetMV(CookedFood[2])-1));
		SetMV(CookedFood[4],(GetMV(CookedFood[4])-1));
	}
	else
	{
		 SetMV(FoodToBeCooked[0],(GetMV(FoodToBeCooked[0])+1));
		 SetMV(FoodToBeCooked[2],(GetMV(FoodToBeCooked[2])+1));
	}
	break;
	default: CustFoodIsReady = -1;
	 break;

	}/*end of switch*/
	Release(CookedFoodLock);
	return(CustFoodIsReady);

}

