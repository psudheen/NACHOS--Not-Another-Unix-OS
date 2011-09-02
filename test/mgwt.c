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
void Customer();
void Init_LockCV(int size);
void Init_TableSizeData(int size);
void Init_CustomerData( int TypeOfFood, int CustID);
int IsCustomerFoodReady (int CustID);
void CarlJrSimulation (void);
void Manager();
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

int customerCountMV;


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
int restoIncomeMV;
int incomeLock;

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

    Print("here ---- 181 \n");
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
	
	
	#ifdef LOCK_BLOCK
	TblNeededCustList[0][0]=CreateMV("TL00",4);
	TblNeededCustList[0][1]=CreateMV("TL01",4);
	TblNeededCustList[0][2]=CreateMV("TL02",4);
	TblNeededCustList[0][3]=CreateMV("TL03",4);
	TblNeededCustList[0][4]=CreateMV("TL04",4);
	TblNeededCustList[0][5]=CreateMV("TL05",4);
	TblNeededCustList[0][6]=CreateMV("TL06",4);
	TblNeededCustList[0][7]=CreateMV("TL07",4);
	TblNeededCustList[0][8]=CreateMV("TL08",4);
	TblNeededCustList[0][9]=CreateMV("TL09",4);


	TblNeededCustList[1][0]=CreateMV("TL10",4);
	TblNeededCustList[1][1]=CreateMV("TL11",4);
	TblNeededCustList[1][2]=CreateMV("TL12",4);
	TblNeededCustList[1][3]=CreateMV("TL13",4);
	TblNeededCustList[1][4]=CreateMV("TL14",4);
	TblNeededCustList[1][5]=CreateMV("TL15",4);
	TblNeededCustList[1][6]=CreateMV("TL16",4);
	TblNeededCustList[1][7]=CreateMV("TL17",4);
	TblNeededCustList[1][8]=CreateMV("TL18",4);
	TblNeededCustList[1][9]=CreateMV("TL19",4);

	
	TblNeededCustList[2][0]=CreateMV("TL30",4);
	TblNeededCustList[2][1]=CreateMV("TL31",4);
	TblNeededCustList[2][2]=CreateMV("TL32",4);
	TblNeededCustList[2][3]=CreateMV("TL33",4);
	TblNeededCustList[2][4]=CreateMV("TL34",4);
	TblNeededCustList[2][5]=CreateMV("TL35",4);
	TblNeededCustList[2][6]=CreateMV("TL36",4);
	TblNeededCustList[2][7]=CreateMV("TL37",4);
	TblNeededCustList[2][8]=CreateMV("TL38",4);
	TblNeededCustList[2][9]=CreateMV("TL39",4);

	
	

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
    
    TblNeededCustLock=CreateLock("TA", 2); 
   
	WaitersQLock=CreateLock("WQ", 2); 
    
	
	
	NeedTableLock[0]=CreateLock("NL0",3);
	NeedTableLock[1]=CreateLock("NL1",3);
	NeedTableLock[2]=CreateLock("NL2",3);
	NeedTableLock[3]=CreateLock("NL3",3);
	NeedTableLock[4]=CreateLock("NL4",3);
	NeedTableLock[5]=CreateLock("NL5",3);
	NeedTableLock[6]=CreateLock("NL6",3);
	NeedTableLock[7]=CreateLock("NL7",3);
	NeedTableLock[8]=CreateLock("NL8",3);
	NeedTableLock[9]=CreateLock("NL9",3);

	
	
    /***************************************************************************************
    Intialization of all Condition Variables
    ***************************************************************************************/

   
    WaitersCV=CreateCondition("WJ",2);

	
	
	NeedTableCV[0]=CreateCondition("NC0",3);
	NeedTableCV[1]=CreateCondition("NC1",3);
	NeedTableCV[2]=CreateCondition("NC2",3);
	NeedTableCV[3]=CreateCondition("NC3",3);
	NeedTableCV[4]=CreateCondition("NC4",3);
	NeedTableCV[5]=CreateCondition("NC5",3);
	NeedTableCV[6]=CreateCondition("NC6",3);
	NeedTableCV[7]=CreateCondition("NC7",3);
	NeedTableCV[8]=CreateCondition("NC8",3);
	NeedTableCV[9]=CreateCondition("NC9",3);
	/**/
	
	
	
	
	Print("here---- 782\n");
    PersonCount.NoOfCust=NO_OF_CUST;
    PersonCount.NoOfOT=NO_OF_OT;
    PersonCount.NoOfWaiters=NO_OF_WAITERS;
    PersonCount.NoOfCooks=NO_OF_COOKS;	
	
	rawMaterialMV[0]=CreateMV("raMA1",5);
	rawMaterialMV[1]=CreateMV("raMA2",5);
	rawMaterialMV[2]=CreateMV("raMA3",5);
	rawMaterialMV[3]=CreateMV("raMA4",5);
	
	
	cookInfoMV[0].cookStatus=CreateMV("ckst1",5);
	cookInfoMV[1].cookStatus=CreateMV("ckst2",5);
	cookInfoMV[2].cookStatus=CreateMV("ckst3",5);
	cookInfoMV[3].cookStatus=CreateMV("ckst4",5);
	
	cookInfoMV[0].foodType=CreateMV("ckft1",5);
	cookInfoMV[1].foodType=CreateMV("ckft2",5);
	cookInfoMV[2].foodType=CreateMV("ckft3",5);
	cookInfoMV[3].foodType=CreateMV("ckft4",5);
	
	cookInfoLock[0]=CreateLock("ckin1",6);
	cookInfoLock[1]=CreateLock("ckin2",6);
	cookInfoLock[2]=CreateLock("ckin3",6);
	cookInfoLock[3]=CreateLock("ckin4",6);
	
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
	SetMV(TableAllotedCustList[0],99);
	SetMV(TableAllotedCustList[1],99);
	SetMV(TableAllotedCustList[2],99);
	SetMV(TableAllotedCustList[3],99);
	SetMV(TableAllotedCustList[4],99);
	SetMV(TableAllotedCustList[5],99);
	SetMV(TableAllotedCustList[6],99);
	SetMV(TableAllotedCustList[7],99);
	SetMV(TableAllotedCustList[8],99);
	SetMV(TableAllotedCustList[9],99);
	
	
	InitCookLock=CreateLock("InCkLk",6);
	
	waitingCookLock=CreateLock("wtgCkL",6);
	
	
	
	InitCookCV=CreateCondition("InitCk",6);
	waitingCookCV=CreateCondition("wtgCk",6);
	incomeLock=CreateLock("incLck",6);
	restoIncomeMV=CreateMV("rstInc",6);
	
	minCookedLevelMV=CreateMV("mnCkdL",6);
	rawmaterialLock=CreateLock("rwlk",4);
	CookedFoodLock=CreateLock("ckfl",5);
	
	CookedFood[0]=CreateMV("CF0",3);
	CookedFood[1]=CreateMV("CF1",3);
	CookedFood[2]=CreateMV("CF2",3);
	CookedFood[3]=CreateMV("CF3",3);
	CookedFood[4]=CreateMV("CF4",3);
	
	SetMV(minCookedLevelMV,15);


	SetMV(cookInfoMV[0].cookStatus,1);
	SetMV(cookInfoMV[0].foodType,9);
	
		SetMV(cookInfoMV[1].cookStatus,1);
	SetMV(cookInfoMV[1].foodType,9);
		SetMV(cookInfoMV[2].cookStatus,1);
	SetMV(cookInfoMV[2].foodType,9);
		SetMV(cookInfoMV[3].cookStatus,1);
	SetMV(cookInfoMV[3].foodType,9);

    TotalTables=CreateMV("TT",2);
	SetMV(TotalTables,10);
	
	AliveCustCountLock=CreateLock("ALC",3);
	customerCountMV=CreateMV("CCMV",4);
	SetMV(customerCountMV,10);
	
/* 	SetMV(TableAllotedCustList[0],1);
	SetMV(TableAllotedCustList[1],3);
	SetMV(TableAllotedCustList[2],4);
	SetMV(TableAllotedCustList[3],0); */
	
	  Manager();
} 




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
	int cookPresent=0;
    int j,k;
	int raw;
	int Cstatus;
	int min;
	int inv;
	int Ftype;
	int table;
	int totab;
	int taball;
	int tallcust;
	int eatin;
/* 	SetMV(GrabedFood[0][0],1);
	SetMV(GrabedFood[0][1],1);
	SetMV(GrabedFood[0][3],1); */

	Print("manager is starting");
  while (1)
  {
  
  

		Acquire(TblNeededCustLock);
		for(a=0;a<3;a++)	   /*a-OTID b-CUSTID*/
		{
				for(b=0;b<10;b++)
				{
				/*Print2("a=[%d]b=[%d] in for loop\n",a,b);
					Print1("TblNeededCustList[a][b]=%d outside for loop\n",TblNeededCustList[a][b]);*/
					if (GetMV(TblNeededCustList[a][b])==1)
					{
					/*There are atleast some customer either waiting for food to be prepared or with food prepared and waiting for table*/
							Acquire(TotalTablesUpdateLock);
							MyCustID=b;
							/* There is a table for Customers
							Alert the customer about this*/
							if (GetMV(TotalTables)>0 )
							{
								 SetMV(TotalTables,GetMV(TotalTables)-1);
								 Release(TotalTablesUpdateLock);
								 /*Update this table so that waiter is aware of seated customers*/
								 Acquire(TableAllotedCustLock);
								 for(c=0;c<10;c++)
								 {
										if(GetMV(TableAllotedCustList[c])==99)
										{
												SetMV(TableAllotedCustList[c],MyCustID);
												SetMV(TblNeededCustList[a][b],0);
												/* There is a table for Customers
												Alert the customer about this*/
												Acquire(NeedTableLock[MyCustID]);
												Print2("CUSTOMER [%d] RECEIVES TOKEN NUMBER [%d] FROM THE MANAGER\n",MyCustID,MyCustID);
												Print1("CUSTOMER[%d] IS INFORMED BY THE MANAGER - THE RESTAURANT IS NOT FULL\n",MyCustID);
												Print1("CUSTOMER [%d] IS SEATED AT TABLE\n",MyCustID);
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
									Print1("CUSTOMER[%d] IS INFORMED BY THE MANAGER-THE RESTAURANT IS  FULL\n",MyCustID);
									Print1("CUSTOMER [%d] IS WAITING TO SIT ON THE TABLE\n", MyCustID);
							}
					}
				}
		}
		Release(TblNeededCustLock);


/* Manager - Waiter Interaction */
/* Print(" Manager here\n"); */
 	for(d=0;d<10;d++)
	{ 
		/* Print(" Manager here2\n"); */
		Acquire(TableAllotedCustLock);
		 Print2("GetMV(TableAllotedCustList[%d])=%d \n",d,GetMV(TableAllotedCustList[d]));  
		if(99!=GetMV(TableAllotedCustList[d]))	  /*List is not empty*/
		{
			/* Print(" Manager here1\n"); */
			Release(TableAllotedCustLock);
			Acquire(WaitersQLock);	
			Signal(WaitersCV,WaitersQLock);
			/* SetMV(TableAllotedCustList[d],99); */			
			Print("MANAGER CALLS BACK ALL WAITERS FROM BREAK\n");
			/*Print(" Manager broadcasted\n");*/
			Release(WaitersQLock);
			break;
		}
		else
		{
			Release(TableAllotedCustLock);
		}
	}
	d=0;
    
	i=0;
	for (i=0;i<200;i++)
	{
	/*Print1("Manager yeilding%d\n",i);*/
	/* Yield(); */
	}
	i=0;
	/* Manager - Waiter Interaction End*/
	
	
	
	
	
	
	 for(i=0;i<4;i++)
	{
		Print1("rawmaterialLock----- manager %d\n",rawmaterialLock);
			
			Acquire(rawmaterialLock);
			Print1("GetMV(rawMaterialMV[i])=%d \n",GetMV(rawMaterialMV[i]));
			if((raw=GetMV(rawMaterialMV[i]))>0)
			{
				Print("rawmaterialLock RE---- manger \n");
				Print2("the raw material of food %d is %d \n",i,raw);
					Release(rawmaterialLock);
					
					Print("cookedfoodLock ------ manager  \n");
					Acquire(CookedFoodLock);
					Print2("the value in managers side of inventory FOR FOOD %d is %d \n",i,GetMV(CookedFood[i]));
					Print2("the value in managers side of raw material for Food %d  is %d \n",i,raw);
					if(GetMV(CookedFood[i])<=GetMV(minCookedLevelMV))
					{
					Print("cookedfoodLock RE  ---- manager \n");
						/*now this particular food type is less than the minimum thrshold */
						Release(CookedFoodLock);
						
						
							for(k=0;k<4;k++)
							{
										Acquire(cookInfoLock[k]);
										Print("cookInfoLock----- manager \n");
										if((Ftype=GetMV(cookInfoMV[k].foodType))==i)
										{
											Print1("cookInfoMV[].foodType=%d \n\n",GetMV(cookInfoMV[i].foodType));
											Print("but cook with that specialization already exists \n");
											cookPresent=1;
											Print("cookInfoLock RE----- manager\n");
											Release(cookInfoLock[k]);
											break;
										}
										Print("cookInfoLock RE----- manager\n");
										Release(cookInfoLock[k]);
							}
						
						

						for(j=0;j<4;j++)           /*hardcoded to number of cooks */
						{
							Print("cookedinfoLock \n");
							Acquire(cookInfoLock[j]);
							Print("cookInfoLock ----- manager\n");
							if((Cstatus=GetMV(cookInfoMV[j].cookStatus))==1)
							{
									Print("cook is free \n");
								
									Print1("cookpresent value is = %d \n\n",cookPresent);
									
									if(cookPresent!=1)          
									{
											cookPresent=0;
   
											SetMV(cookInfoMV[j].cookStatus,0);   
											SetMV(cookInfoMV[j].foodType,i);
											Print("cookedinfoLock RE------ manager \n");
											Release(cookInfoLock[j]);
											Print("InitCookLock ---------- manager \n");
											Acquire(InitCookLock);
											
											Signal(InitCookCV,InitCookLock);
											Print("signalled the cook manger\n");
											Print("InitCookLock RE------ manger \n");
											Release(InitCookLock);
											break;
									}
									else
									{
										Print("cookedinfoLock ----- manager \n");
										Release(cookInfoLock[j]);
									}
							}
							else
							{
								Print("cookedinfoLock RE----- manager \n");
								Release(cookInfoLock[j]);
							}
						}
					/* 	Print("waitingCookLock ---- manger\n");
						Acquire(waitingCookLock);
						Wait(waitingCookCV,waitingCookLock);
						Print("waitingCookLock RE------ manager\n");
						Release(waitingCookLock); */
					}
					else
					{	Print("cookedfoodlock Re----- manager\n");
					
						Release(CookedFoodLock);
					}
				} 
				else 
				{
						
						Print("rawmaterialLock----- manager\n");
						SetMV(rawMaterialMV[i],GetMV(rawMaterialMV[i])+20);
						Print("rawmaterialLock RE---- manager\n");
						Release(rawmaterialLock);
						Print((int)"MANAGER GOES TO BANK TO WITHDRAW THE CASH\n");
						Print((int)"MANAGER REFILLS THE INVENTORY\n");
						Print((int)"INVENTORY IS LOADED IN THE RESTAURANT\n");
						/*PrintSyscall("themanager withdraws money \n");*/
						/*PrintSyscall("manager goes to buy the raw matrials ");*/
						Acquire(incomeLock);
						Print("incomeLock ---- manager\n");
						SetMV(restoIncomeMV,GetMV(restoIncomeMV)+100);
						Print("incomeLock RE---- manager\n");
						Release(incomeLock);
				}
			}
	
	
	
Acquire(AliveCustCountLock);

if(GetMV(customerCountMV)==0)
{
	Release(AliveCustCountLock);
	Print("END OF THE SIMULATION \n\n\n");
	Halt();
}
Release(AliveCustCountLock);


	
}
Exit(0);
}


