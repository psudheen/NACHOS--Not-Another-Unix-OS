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


// External functions used by this file

extern void ThreadTest(void), Copy(char *unixFile, char *nachosFile);
extern void Print(char *file), PerformanceTest(void);
extern void StartProcess(char *file), ConsoleTest(char *in, char *out);
extern void MailTest(int networkID);
extern void TestSuite(void);
void CarlJrSimulation (void);
void SimiluationMenu();

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
		if (!strcmp(*argv, "-T"))               // Test Suite: link for this code is at the bottom of part 1 description
		      TestSuite();
		if (!strcmp(*argv, "-P2"))               // Problem 2: for part 2
		      SimiluationMenu();
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
            Delay(2);               // delay for 2 seconds
                        // to give the user time to
                        // start up another nachos
            MailTest(atoi(*(argv + 1)));
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

//****************************************************************************************
// all the contents here is an equivalent for .h file
typedef struct CustomerData_t
{
 int CustID;
 int TypeOfFood;
 int Food[5];
 //int D6Bgr;
 //int D3Bgr;
 //int D4VBgr;
 //int D2Ffries;
 //int D1Soda;

};

void Customer(int CustID);
void OrderTaker(int OTID);
void Waiter(int MyWaiterID);
void Manager(int MyMgr);
void Cook(int CookID);
void Init_LockCV(int size);
void Init_TableSizeData(int size);
void Init_CustomerData(CustomerData_t *CustData, int TypeOfFood, int CustID);
int IsCustomerFoodReady (CustomerData_t *CustData);


Lock *InventoryLock = new Lock("InventoryLock");
Lock *MgrCookStatUpdateLock = new Lock("MgrCookStatUpdateLock");
Lock *NeedInventoryLock[200];

Condition *NeedInventoryCV[200];


char CookStyle[4][40]={"$6 Burger","$3 Burger","Veggie Burger","French Fries"};
int Inventory[4]={250,250,250,250};
int ManagerInstrToStop[4];

#define MIN_REQUIRED_INVENTORY 100// = 20 customers * 5 types of food
#define COOKEDFOOD_MAX ((COOKEDFOOD_MIN*3)+1)


//Cook needed stuff
Lock *CookQLock = new Lock("CookQLock");
Condition *CookQCV = new Condition("CookQCV");

#define MAX_OT 200


typedef struct InventoryData_t
{
 int NoOfD6Bgr;
 int NoOfD3Bgr;
 int NoOfD4VBgr;
 int NoOf2DFfries;
 int NoOfD1Soda;
};
InventoryData_t  InvtryData;




CustomerData_t CustData[40];

typedef struct PersonCount_t
{
 int NoOfOT;
 int NoOfCust;
 int NoOfCooks;
 int NoOfWaiters;
 int NoOfMgr;
};

PersonCount_t PersonCount;

enum WorkerStatus { FREE =0, BUSY, WAITING};

// Lists
//List of Customer who wakes the OT to process thier orders
List *SigCustList = new List();
List *GrabedFood = new List();
List *EatInCustWaitingForFood = new List();
List *ToGoCustWaitingForFood = new List();
List *ToGoGrabedFood = new List();
List *MgrCookStyleLst = new List();

Lock *SigCustListLock = new Lock("SigCustListLock");
Lock *GrabedFoodLock = new Lock("GrabedFoodLock");
Lock *EatInCustWaitingForFoodLock = new Lock("EatInCustWaitingForFoodLock");
Lock *ToGoCustWaitingForFoodLock = new Lock("ToGoCustWaitingForFoodLock");
Lock *ToGoGrabedFoodLock = new Lock("ToGoGrabedFoodLock");
Lock *MgrCookStyleLstLock = new Lock("MgrCookStyleLstLock");
//Monitor variables
int MoneyInRstrnt = 0;

//All the Locks are listed here
Lock *MonitorAmtOfMoneyLock = new Lock("MonitorAmtOfMoneyLock");

int custLineLength = 0; //Monitor Variable to track the Customer line length
// CVs' to schedule interaction between Customer and OT
Condition *custWaitingCV = new Condition("custWaitingCV");
//Condition *orderTakerCV[MAX_OT];

Lock *custLineLock = new Lock("custLineLock");
//WorkerStatus OTStatus[MAX_OT];

//OrderTaker Data

Lock *OrderTakerLock[MAX_OT];
Condition *OTWaitingCV[MAX_OT];

int OrderTakerStatus[MAX_OT];
int CookedFood[5]={100,100,100,100,100000000};
Lock *OTStatusUpdateLock = new Lock("OTStatusUpdateLock");

Lock *CookedFoodLock = new Lock("CookedFoodLock");

//Waiter stuff
List *TableAllotedCustList = new List();
Lock *TableAllotedCustLock = new Lock("TableAllotedCustLock");

Lock *WaitersQLock = new Lock("WaitersQLock");
Condition *WaitersCV= new Condition("WaitersCV");

//Cust- Mgr data
int TotalTables = 25; // Mgr decrements, Waiter Increments
Lock *TotalTablesUpdateLock = new Lock ("TotalTablesUpdateLock");

Lock *NeedTableLock[200];// = new Lock ("NeedTableLock");
Condition *NeedTableCV[200];// = new Condition("NeedTableCV");

List *TblNeededCustList = new List();
Lock *TblNeededCustLock = new Lock("TblNeededCustLock");

//Cust- Waiter data
Lock *NeedWaitersLock[200];// = new Lock ("NeedTableLock");
Condition *NeedWaitersCV[200] ;//= new Condition("NeedWaitersCV");

// OT maintaining the pending orders for food
//This is the array of food that needs to cooked from the cooks
//Assumimg Soda is always available
// This array will be incrmented when there is food to be grabed - Eat In or To Go
// FoodToBeCooked[0] is 6$ burger, [1] is 3$ burger, [2] is Veggie and [3] is French fries
int FoodToBeCooked[4];
Lock *FoodToBeCookedUpdateLock = new Lock("FoodToBeCookedUpdateLock");
int FoodLevelCheckIteration[4];
int AliveCustCount=0;
Lock *AliveCustCountLock=new Lock("AliveCustCountLock");

void Init_LockCV(int size)
{
 int i;
 Lock *LockPointer;
 Condition *ConditionPointer;
 for ( i =0; i< size; i++)
  {
    LockPointer = new Lock("OrderTakerLock");
    ConditionPointer = new Condition("OTWaitingCV");
    OrderTakerLock[i] = LockPointer;
    OTWaitingCV[i] = ConditionPointer;
  }
}

void Init_TableSizeData(int size)
{
 Lock *LockPointer;
 Lock *LockPointer_1;
 Lock *LockPointer_2;
 Condition *ConditionPointer;
 Condition *ConditionPointer_1;
 Condition *ConditionPointer_2;
 for ( int i =0; i< size; i++)
  {
    LockPointer = new Lock("NeedTableLock");
    NeedTableLock[i] = LockPointer;
    ConditionPointer = new Condition("NeedTableCV");
    NeedTableCV[i] = ConditionPointer;
  }
  for (int j =0; j< size; j++)
  {
    LockPointer_1 = new Lock("NeedWaitersLock");
    NeedWaitersLock[j] = LockPointer_1;
    ConditionPointer_1 = new Condition("NeedWaitersCV");
    NeedWaitersCV[j] = ConditionPointer_1;
  }
  for (int k =0; k< size; k++)
  {
    LockPointer_2 = new Lock("NeedInventoryLock");
    NeedInventoryLock[k] = LockPointer_2;
    ConditionPointer_2 = new Condition("NeedInventoryCV");
    NeedInventoryCV[k] = ConditionPointer_2;
  }

}

//****************************************************************************************
int test_case = 0;
void CarlJrSimulation (void)
{
    Thread *t;
    char *name;
    if (test_case==0)
        {
            printf("Please enter the number of Customers: ");
            scanf("%d", &(PersonCount.NoOfCust));
            printf("Please enter the number of Order Takers: ");
            scanf("%d", &(PersonCount.NoOfOT));
            printf("Please enter the number of Cooks: ");
            scanf("%d", &(PersonCount.NoOfCooks));
            printf("Please enter the number of Waiters: ");
            scanf("%d", &(PersonCount.NoOfWaiters));
        }

    Init_LockCV(175);
    Init_TableSizeData(175);
	AliveCustCount=PersonCount.NoOfCust;
    printf("Simulation starts here \n\n\n");


    //Create order taker threads
    for (int i=0; i<PersonCount.NoOfOT; i++)
        {
            name = new char [20];
            sprintf(name,"OrderTaker[%d]",i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)OrderTaker,i);
        }
    //Create Customer threads
    for (int i=0; i<PersonCount.NoOfCust; i++)
        {
            //Logic to determine eat-in or To-go type
            int NoOfEatIn = (2*(PersonCount.NoOfCust))/3;
            int NoOfToGo = (PersonCount.NoOfCust) - NoOfEatIn;
            if( i <= NoOfEatIn )
                {
                  Init_CustomerData(&CustData[i], 0, i);
                }
            else
                {
                  Init_CustomerData(&CustData[i], 1, i);
                }
            name = new char [20];
            sprintf(name,"Customer[%d]",i);

            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Customer,int (&CustData[i]));
        }

    //Create Waiters
     for (int i=0; i<PersonCount.NoOfWaiters; i++)
        {
            name = new char [20];
            sprintf(name,"Waiters[%d]",i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Waiter,i);
        }

    //Create Cooks
     for (int i=0; i<PersonCount.NoOfCooks; i++)
        {
            name = new char [20];
            sprintf(name,"Cook[%d]",i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Cook,i);
        }

    //Create manager
    name = new char [20];
    sprintf(name,"Manager");
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)Manager,1);

}

void Init_CustomerData(CustomerData_t *MyCustData, int TypeOfFood, int CustID)
{
 if(MyCustData == NULL)
    {

     printf("CustData is null");
     return;
    }
 MyCustData->CustID = CustID;
 MyCustData->TypeOfFood = TypeOfFood; //Eat -in
  //Food pattern
  //Assuming all customers will have any of below food pattern
  switch ( CustID%5)
    {
     case 0: MyCustData->Food[0] = 1;
             MyCustData->Food[1] = 1;
             MyCustData->Food[2] = 1;
             MyCustData->Food[3] = 1;
             MyCustData->Food[4] = 1;
             break;
     case 1: MyCustData->Food[0] = 1;
             MyCustData->Food[1] = 0;
             MyCustData->Food[2] = 1;
             MyCustData->Food[3] = 0;
             MyCustData->Food[4] = 1;
             break;
     case 2: MyCustData->Food[0] = 0;
             MyCustData->Food[1] = 1;
             MyCustData->Food[2] = 0;
             MyCustData->Food[3] = 0;
             MyCustData->Food[4] = 1;
             break;
     case 3: MyCustData->Food[0] = 1;
             MyCustData->Food[1] = 0;
             MyCustData->Food[2] = 1;
             MyCustData->Food[3] = 0;
             MyCustData->Food[4] = 1;
             break;
     case 4: MyCustData->Food[0] = 0;
             MyCustData->Food[1] = 0;
             MyCustData->Food[2] = 0;
             MyCustData->Food[3] = 0;
             MyCustData->Food[4] = 1;
             break;
     default: MyCustData->Food[0] = 0;
             MyCustData->Food[1] = 0;
             MyCustData->Food[2] = 0;
             MyCustData->Food[3] = 0;
             MyCustData->Food[4] = 1;
             break;
    }
}



void Customer(int CustomerData)
{
  CustomerData_t *Cdata =(CustomerData_t *)CustomerData;
  int CustID = Cdata->CustID;
  int myOT=-1;
  // Just entered the restaurant, grab the lock and check the line length
  custLineLock->Acquire();
  //printf("Customer[%d] acquired the lock:%s\nand checking for his OT\n", CustID, custLineLock->getName());
  //Decide who is my OT
  OTStatusUpdateLock->Acquire();
  for(int i=0;i<PersonCount.NoOfOT;i++)
    {
      if(FREE == OrderTakerStatus[i]) //checking for a free order taker
      {
          OrderTakerStatus[i] = BUSY;;//make ith (my) OT busy
          myOT=i;
          printf("Customer[%d] has OrderTaker[%d]\n", CustID, myOT);
          break;
      }
    }
    OTStatusUpdateLock->Release();

  //Here either I know who is my OT or there are no OT for me
  if(-1==myOT) //get in line
    {
        custLineLength++;
        printf("Customer[%d] has NO OrderTakers, Snoozing!\n", CustID);
        custWaitingCV->Wait(custLineLock);
    }
  //Get a waiting OT
  OTStatusUpdateLock->Acquire();
  for(int j=0;j<PersonCount.NoOfOT;j++)
  {
      if (WAITING == OrderTakerStatus[j])
      {
        myOT=j;
        OrderTakerStatus[j]=BUSY;
        printf("Waiting Customer[%d] has OrderTaker[%d]\n", CustID,j);
        break;
      }
  }
  OTStatusUpdateLock->Release();

 //

  //printf("Customer[%d] released the lock:%s\nand waiting for his OT\n", CustID, custLineLock->getName());
// By here, Customer is aware of his OT and ready to order his/her food
  custLineLock->Release(); //Allow OT to manipluate CustLineLength
  // It is OT turn now!
  // Take the transaction lock and signal the waiting OT that Cust is Done.
  //printf("Ordertaker[%d] acquired the lock:%s\n", myOT, OrderTakerLock[myOT]->getName());
  SigCustListLock->Acquire();
  SigCustList->Append((void*) CustID);
  SigCustListLock->Release();

  printf("Customer[%d] is giving below order to OrderTaker[%d]\n",CustID,myOT);
  if(Cdata->Food[0] == 0)
  printf("Customer[%d] is NOT ordering 6-dollar burger\n", CustID);
  else
  printf("Customer[%d] is ordering 6-dollar burger\n", CustID);
  if(Cdata->Food[1] == 0)
  printf("Customer[%d] is NOT ordering 3-dollar burger\n", CustID);
  else
  printf("Customer[%d] is ordering 3-dollar burger\n", CustID);
  if(Cdata->Food[2] == 0)
  printf("Customer[%d] is NOT ordering veggie burger\n", CustID);
  else
  printf("Customer[%d] is ordering veggie burger\n", CustID);
  if(Cdata->Food[3] == 0)
  printf("Customer[%d] is NOT ordering French fries\n", CustID);
  else
  printf("Customer[%d] is ordering French fries\n", CustID);
  if(Cdata->Food[4] == 0)
  printf("Customer[%d] is NOT ordering Soda\n", CustID);
  else
  printf("Customer[%d] is ordering Soda\n", CustID);

  // Wake-up the WAITING OT who is waiting for my order.
  OrderTakerLock[myOT]->Acquire();
  OTWaitingCV[myOT]->Signal(OrderTakerLock[myOT]);
  OTWaitingCV[myOT]->Wait(OrderTakerLock[myOT]);// Waiting for my order to be processed
  printf("Customer [%d] receives token number [%d] from the OrderTaker [%d]\n",CustID,CustID,myOT);
  OrderTakerLock[myOT]->Release();

  //Customer-Manager interaction
    //If Customer is Eat-in type
    //Wait for Manager to check if there is table for me
      //Need lock and CV to go on wait
  if (Cdata->TypeOfFood == 0)
    {
      TblNeededCustLock->Acquire();
      TblNeededCustList->Append((void *)CustID);
      TblNeededCustLock->Release();

      NeedTableLock[CustID]->Acquire();
      printf("Customer %d is waiting for the table\n", CustID);
      NeedTableCV[CustID]->Wait(NeedTableLock[CustID]);
      NeedTableCV[CustID]->Signal(NeedTableLock[CustID]);// Send ack to Mgr
      printf("Eat IN Customer %d got the table\n", CustID);
      // Manager signals only when there are free tables
      //Here Customers are sure of allocated Table
      NeedTableLock[CustID]->Release();

      //Customer Waits for Waiter
        // Eat-in Customer whose food is ready, seated and waiting for waiters to serve them
      NeedWaitersLock[CustID]->Acquire();
      printf("Eat IN Customer %d is waiting for the waiter\n", CustID);
      NeedWaitersCV[CustID]->Wait(NeedWaitersLock[CustID]);
      // Waiters signals only when Customer Food is ready
        //Here Customers are sure of served food and they must leave!
      NeedWaitersLock[CustID]->Release();
      printf("\n\nEAT IN Customer[%d] order is served by the waiter and he is leaving!!Thank you!!\n\n\n",CustID );
    }
    else
      {
         printf("\n\nTO GO Customer[%d] order is served by Order Taker[%d] and he is leaving!!Thank you!!\n\n\n",CustID,myOT );
      }
   AliveCustCountLock->Acquire();
   AliveCustCount--;
   AliveCustCountLock->Release();
  currentThread->Finish();
}


void OrderTaker(int myOTId)
{
  int MyCustID = -1;
  int CustFoodIsReady =-1;
  int Poped_CID = -1;
  int Poped_OT = -1;
  //printf("Created %d OT\n", myOTId);
  while(true)
   {
    custLineLock->Acquire();
    //Are there any waiting customers?
    EatInCustWaitingForFoodLock->Acquire();
    ToGoCustWaitingForFoodLock->Acquire();

    if(custLineLength > 0)
      {
        custWaitingCV->Signal(custLineLock);
        custLineLength--;
        custLineLock->Release();
        OTStatusUpdateLock->Acquire();
        OrderTakerStatus[myOTId]=WAITING;
        OTStatusUpdateLock->Release();
        EatInCustWaitingForFoodLock->Release();
        ToGoCustWaitingForFoodLock->Release();

      }
    //Is there any food to bag?
    else if(!EatInCustWaitingForFood->IsEmpty())
        {
          custLineLock->Release();
          Poped_CID = (int)EatInCustWaitingForFood->Remove();
          Poped_OT = (int)EatInCustWaitingForFood->Remove();

          if( Poped_CID!=-1 && Poped_OT!=-1)
            {
             if (IsCustomerFoodReady (&CustData[Poped_CID]))
                {
                  // Add this customer to GrabedFood
                  GrabedFoodLock->Acquire();
                  GrabedFood->Append((void*)myOTId);
                  GrabedFood->Append((void*)Poped_CID);
                  GrabedFoodLock->Release();
                }
              else//Food is not ready, put him back on Q
                {
                  EatInCustWaitingForFood->Append((void*)Poped_CID);
                  EatInCustWaitingForFood->Append((void*)Poped_OT);
                }
               for (int i=0;i<2;i++)
               {
                 currentThread->Yield();
               }
               EatInCustWaitingForFoodLock->Release();
               ToGoCustWaitingForFoodLock->Release();
               continue;
            }
           EatInCustWaitingForFoodLock->Release();
           ToGoCustWaitingForFoodLock->Release();

        }


    else if(!ToGoCustWaitingForFood->IsEmpty())
            {
             //Now, I see only ToGo customers
             custLineLock->Release();
             Poped_CID = (int)ToGoCustWaitingForFood->Remove();
             Poped_OT = (int)ToGoCustWaitingForFood->Remove();

             if( Poped_CID!=-1 && Poped_OT!=-1)
               {
                if (IsCustomerFoodReady (&CustData[Poped_CID]))
                   {
                     // Add this customer to GrabedFood
                     OTWaitingCV[myOTId]->Signal(OrderTakerLock[myOTId]);
                     OrderTakerLock[myOTId]->Release();
                     //ToGoGrabedFoodLock->Acquire();
                     //ToGoGrabedFood->Append((void*)myOTId);
                     //ToGoGrabedFood->Append((void*)Poped_CID);
                     //ToGoGrabedFoodLock->Release();
                    }
                 else//Food is not ready, put him back on Q
                   {
                     ToGoCustWaitingForFood->Append((void*)Poped_CID);
                     ToGoCustWaitingForFood->Append((void*)Poped_OT);
                   }
                 EatInCustWaitingForFoodLock->Release();
                 ToGoCustWaitingForFoodLock->Release();
                 continue;
               }
              EatInCustWaitingForFoodLock->Release();
               ToGoCustWaitingForFoodLock->Release();

            }
                        // Alert the waiting customer
             // custLineLock->Release();
             // continue;
      else
      {
        //Nothing to do
        //Set OT status to FREE
        custLineLock->Release();
        EatInCustWaitingForFoodLock->Release();
        ToGoCustWaitingForFoodLock->Release();
        OTStatusUpdateLock->Acquire();
        OrderTakerStatus[myOTId]=FREE;
        OTStatusUpdateLock->Release();
      }
  //Now, OT is ready to take order
  // Take interaction lock
  OrderTakerLock[myOTId]->Acquire();
  //release hold on CustLineLen
  //OT should wait until customer signals him with orders
  OTWaitingCV[myOTId]->Wait(OrderTakerLock[myOTId]);
  //When I'm here, Customer has signaled that his order is ready
  //Compute the money and handle the token number.

  //check which customer signalled me?
  SigCustListLock->Acquire();
  if( SigCustList->IsEmpty())
    {
      printf("OrderTaker[%d]: ERROR!!Siganlling Customer list is empty!\n", myOTId);
      continue;
    }
  MyCustID = (int)SigCustList->Remove();
  SigCustListLock->Release();

  MonitorAmtOfMoneyLock->Acquire();
  MoneyInRstrnt = (6 * CustData[MyCustID].Food[0] + 3 * CustData[MyCustID].Food[1] + 4 * CustData[MyCustID].Food[2] \
                  + 2 * CustData[MyCustID].Food[3] + CustData[MyCustID].Food[4]);
  MonitorAmtOfMoneyLock->Release();

  printf("OrderTaker[%d] processed the Customer [%d] order and acknowledged the payment\n", myOTId, MyCustID);

  //Order processed Signal the customer to get out of sleep!
  //OTWaitingCV[myOTId]->Signal(OrderTakerLock[myOTId]);

  //OrderTakerLock[myOTId]->Release();

//Order Processing
  // Check the customer style of eating - eat in or to-go?
  if( CustData[MyCustID].TypeOfFood == 0)
    {
     //Eat - in customers
     //If type of food is only Soda, token is handled
     //and GrabedFood()->Append()
     if( MyCustID%5 == 4)
        {
          //Update the list which waiter will look to serve customers
          GrabedFoodLock->Acquire();
          GrabedFood->Append((void*)myOTId);
          GrabedFood->Append((void*)MyCustID);
          GrabedFoodLock->Release();
        }
      // Customers with multiple orders
      else
        {
         //For (each) this customer, check if his order is ready for each type of food
          CustFoodIsReady = IsCustomerFoodReady (&CustData[MyCustID]);
         // if CustFoodIsReady = 1, Food is ready and can be grabed
         if (CustFoodIsReady==1)
            {
             GrabedFoodLock->Acquire();
             GrabedFood->Append((void*)myOTId);
             GrabedFood->Append((void*)MyCustID);
             GrabedFoodLock->Release();
             printf("Eat-in Custmer %d food is ready\n", MyCustID);
            }
        else
            {
             //list of customers to whom token should be handled but shouldnt grab the food
             //this Q is as good as saying who gave token number to whom when they wanna wait
              EatInCustWaitingForFoodLock->Acquire();
              EatInCustWaitingForFood->Append((void*)MyCustID);
              EatInCustWaitingForFood->Append((void*)myOTId);
              EatInCustWaitingForFoodLock->Release();
              printf("Eat-in Custmer %d food NOT is ready\n", MyCustID);
            }


        }

    }
  // Customers of type Dine -in
  else
    {
     if( MyCustID%5 == 4)// If customer needs only Soda
       {
        //Signal this waiting customer and "wait" for him to acknowledge
        //If acknowledged, signal the waiting customer and Customer threads finishes
       }
     // Customers with multiple orders
     else
       {
        //For (each) this customer, check if his order is ready for each type of food
        CustFoodIsReady = IsCustomerFoodReady (&CustData[MyCustID]);
         // if CustFoodIsReady = 1, Food is ready and can be grabed
        if (CustFoodIsReady==1)
           {
            ToGoGrabedFoodLock->Acquire();
            ToGoGrabedFood->Append((void*)myOTId);
            ToGoGrabedFood->Append((void*)MyCustID);
            ToGoGrabedFoodLock->Release();
            printf("ToGo Custmer %d food is ready\n", MyCustID);
            OTWaitingCV[myOTId]->Signal(OrderTakerLock[myOTId]);
            OrderTakerLock[myOTId]->Release();
            continue;
           }
        else
           {
            //list of customers to whom token should be handled but shouldnt grab the food
            //this Q is as good as saying who gave token number to whom when they wanna wait
             ToGoCustWaitingForFoodLock->Acquire();
             ToGoCustWaitingForFood->Append((void*)MyCustID);
             ToGoCustWaitingForFood->Append((void*)myOTId);
             ToGoCustWaitingForFoodLock->Release();
             printf("ToGo Custmer %d food NOT is ready\n", MyCustID);
           }
       }

    }
    //Order processed Signal the customer to get out of sleep!
    OTWaitingCV[myOTId]->Signal(OrderTakerLock[myOTId]);

    OrderTakerLock[myOTId]->Release();

   }//end of while()
}//end of OrderTaker()

int IsCustomerFoodReady (CustomerData_t *CID)
    {
      int CustFoodIsReady = 0;
      CustomerData_t *MyCustData = CID;
      int MyCustID = CustData->CustID;
      CookedFoodLock->Acquire();
      FoodToBeCookedUpdateLock->Acquire();
      switch(MyCustID%5)
            {
            case 0: if ( MyCustData->Food[0] < CookedFood[0] && \
                    MyCustData->Food[1] < CookedFood[1] &&\
                    MyCustData->Food[2] < CookedFood[2] && \
                    MyCustData->Food[3] < CookedFood[3] && \
                    MyCustData->Food[4] < CookedFood[4])
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
            case 1: if ( MyCustData->Food[0] < CookedFood[0]&&\
                    MyCustData->Food[2] < CookedFood[2]&&\
                    MyCustData->Food[4] < CookedFood[4])
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
            case 2: if( MyCustData->Food[1]<CookedFood[1]&&\
                        MyCustData->Food[4] < CookedFood[4])
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
            case 3: if( MyCustData->Food[0] <CookedFood[0]&&\
                    MyCustData->Food[2] < CookedFood[2]&&\
                    MyCustData->Food[4] < CookedFood[4])
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

            }//end of switch
      CookedFoodLock->Release();
      FoodToBeCookedUpdateLock->Release();
      return(CustFoodIsReady);

    }
void Waiter(int MyWaiterID)
{
  printf("Created %d Waiter\n", MyWaiterID);
  while(true)
    {
      // Check if there are customers seated and thier food is ready
      //Looping myself
      int Poped_CID=-1;
      int Poped_OT=-1;
      int Poped_CID_Alloc =-1;
      int RmvCount =0;
      int RmvTable[200];
			int RmvCustCnt=0;
			int RmvCustCntAry[200];
      bool IsThisCustServed = false;
      GrabedFoodLock->Acquire();
      //See for which customers food is grabed
			while(!GrabedFood->IsEmpty())
        {
          Poped_OT = (int)GrabedFood->Remove();
          Poped_CID = (int)GrabedFood->Remove();

          if( Poped_CID!=-1 && Poped_OT!=-1)
            {
              //printf("Waiter [%d] got token number [%d] for Customer [%d] from OrderTaker [%d]\n",MyWaiterID,Poped_CID,Poped_CID,Poped_OT);
            }
          // See if the customers for whom food is grabed are seated or not?
          TableAllotedCustLock->Acquire();
           while(!TableAllotedCustList->IsEmpty())
            {
              Poped_CID_Alloc = (int)TableAllotedCustList->Remove();
              //Is the Customer given by OT is seated?
              if( Poped_CID_Alloc == Poped_CID)
                {
                  // logic to signal the customer who is been served here
                  NeedWaitersLock[Poped_CID_Alloc]->Acquire();
                  NeedWaitersCV[Poped_CID_Alloc]->Signal(NeedWaitersLock[Poped_CID_Alloc]);
                  NeedWaitersLock[Poped_CID_Alloc]->Release();

                  printf("Waiter [%d] validates the token number for Customer [%d]\n",MyWaiterID, Poped_CID_Alloc);
                  printf("Waiter [%d] serves food to Customer [%d]\n",MyWaiterID,Poped_CID_Alloc);
                  IsThisCustServed = true;
                  break;
                }
                // OT customer is not seated and I will not serve him!
              else
                {
                  //Poped_CID_Alloc = TableAllotedCustList->Append();
                  RmvTable[RmvCount] = Poped_CID_Alloc;
                  RmvCount++;
                }

             }
             for(int i=0;i<RmvCount;i++)
              {
                TableAllotedCustList->Append((void*)RmvTable[RmvCount]);
              }
            RmvCount =0;
            TableAllotedCustLock->Release();
            if ( IsThisCustServed!=true)
              {
							  RmvCustCntAry[RmvCustCnt]=Poped_OT;
								RmvCustCnt++;
								RmvCustCntAry[RmvCustCnt]=Poped_CID;
								RmvCustCnt++;
							}

						if(IsThisCustServed==true)
						{
							IsThisCustServed = false; //reset this flag!
							break;
						}
        }
       if(RmvCustCnt>0)
        {
					for(int i=0;i<RmvCustCnt;i++)
					{
						GrabedFood->Append((void*)RmvCustCntAry[i]);
						i=i+1;
            GrabedFood->Append((void*)RmvCustCntAry[i]);
					}
				}
        GrabedFoodLock->Release();
        // There are no customers waiting, Waiters go on break
        WaitersQLock->Acquire();
        printf(" %d Waiter went on wait\n", MyWaiterID);
        WaitersCV->Wait(WaitersQLock);
        printf(" %d Waiter signalled by manager\n", MyWaiterID);
        WaitersQLock->Release();
    
    }//end of while()
			  AliveCustCountLock->Acquire();
   	  if (AliveCustCount==0 )
		interrupt->Halt();
      AliveCustCountLock->Release();
} //end of waiter()


void Manager(int MyMgrID)
{
  printf("Created %d Manager\n", MyMgrID);
  int MyCustID;
  int MyCookSpec;
  int MyCookStat;
  int RmvedCookCnt =0;
  int RmvedCook[200];
  while (true)
    {
       //Manager should not take customers untill OT finished interaction with them
       TblNeededCustLock->Acquire();
       if ( !TblNeededCustList->IsEmpty())
        {
           // There are atleast some customer either waiting for food to be prepared
           // or with food prepared and waiting for table
           TotalTablesUpdateLock->Acquire();
           MyCustID =(int)TblNeededCustList->Remove();
           // There is a table for Customers
           //Alert the customer about this
           if ( TotalTables>0 )
            {
              TotalTables--;
              TotalTablesUpdateLock->Release();
              TblNeededCustLock->Release();
              printf("Manager found Cust%d needs table\n", MyCustID);
              // Update this table so that waiter is aware of seated customers
              TableAllotedCustLock->Acquire();
              TableAllotedCustList->Append((void *)MyCustID);
              TableAllotedCustLock->Release();
              // There is a table for Customers
              //Alert the customer about this
              NeedTableLock[MyCustID]->Acquire();
              printf("Customer [%d] receives token number [%d] from the Manager\n",MyCustID,MyCustID);
              printf("Customer[%d] is informed by the Manager - the restaurant is NOT full\n",MyCustID);
              printf("Customer [%d] is seated at table\n",MyCustID);
              NeedTableCV[MyCustID]->Signal(NeedTableLock[MyCustID]);
              NeedTableCV[MyCustID]->Wait(NeedTableLock[MyCustID]);
              NeedTableLock[MyCustID]->Release();
            }
            // if there are no tables, just give them the token number
            else
              {
                //pritnf("Customer [%d] receives token number [%d] from the Manager\n",MyCustID,MyCustID);
                TblNeededCustLock->Release();
                TotalTablesUpdateLock->Release();
                printf("Customer[%d] is informed by the Manager-the restaurant is  full\n",MyCustID);
                printf("Customer [%d] is waiting to sit on the table\n", MyCustID);
              }
        }
        else
         {
           TblNeededCustLock->Release();
         }

     // Manager - Waiter Interaction
      GrabedFoodLock->Acquire();

      if(!GrabedFood->IsEmpty())
        {
          WaitersQLock->Acquire();
          WaitersCV->Broadcast(WaitersQLock);
          //printf(" Manager broadcasted\n");
          WaitersQLock->Release();
          //printf("mgr is yielding\n");
        }
       GrabedFoodLock->Release();
       // Cook-MGR addition
        for(int i=0;i<3;i++)
        {
            InventoryLock->Acquire();
            if(Inventory[i]>MIN_REQUIRED_INVENTORY)
            {
                InventoryLock->Release();
                FoodToBeCookedUpdateLock->Acquire();
                if(FoodToBeCooked[i]>0)
                {
                    // Check for five times.
                    if(FoodLevelCheckIteration[i]<=5)
                    {
                        FoodToBeCookedUpdateLock->Release();
                        FoodLevelCheckIteration[i]++;
                        //Check if there are any free cooks?
                        MgrCookStatUpdateLock->Acquire();
                        while (!MgrCookStyleLst->IsEmpty())
                          {
                            MyCookSpec = (int)MgrCookStyleLst->Remove();
                            MyCookStat = (int)MgrCookStyleLst->Remove();
                            if ( MyCookStat == WAITING)
                              {
                                // Signal one cook and Manager specified
                                // cook will start cooking
                                MgrCookStyleLst->Append((void *)i); // Speciality
                                MgrCookStyleLst->Append((void *)BUSY); // status
                                CookQLock->Acquire();
                                FoodToBeCooked[i] = FoodToBeCooked[i]-2;
                                CookQCV->Signal(CookQLock);
                                CookQLock->Release();
                                break;
                              }
                            else
                              {
                               RmvedCookCnt++;
                               RmvedCook[RmvedCookCnt] = MyCookSpec;
                               RmvedCook[RmvedCookCnt+1] =MyCookStat;
                              }
                          }
                          if ( RmvedCookCnt >0 )
                            {
                              for (int k =0; k<RmvedCookCnt;k++)
                                {
                                  MgrCookStyleLst->Append((void *)RmvedCook[k]); // Speciality
                                  MgrCookStyleLst->Append((void *)RmvedCook[k+1]); // Speciality
                                  k++;
                                }
                              RmvedCookCnt =0;
                            }
                        MgrCookStatUpdateLock->Release();
                    }
                    //checked five time for waiting cooks to clear the backlog
                    // I will hire new cooks
                    else
                        {
                           //FoodLevelCheckIteration[i]>5
                          //Hire a new cook logic
                          FoodLevelCheckIteration[i] = 0;//reset the counter, I hired a new cook
                        }
                }
                else
                  {
                    FoodToBeCookedUpdateLock->Release();
                  }
            }
            else
              {
                //refill the inventory
                InventoryLock->Release();
                MonitorAmtOfMoneyLock->Acquire();
                if (MoneyInRstrnt > MIN_REQUIRED_INVENTORY*25)
                  {
                    MoneyInRstrnt = (MoneyInRstrnt - (MIN_REQUIRED_INVENTORY*25));
                    InventoryLock->Acquire();
                    Inventory[i] = Inventory[i]+100;
                    InventoryLock->Release();
                    for (int iCount=0;iCount<100;iCount++)
                    {
                      currentThread->Yield();
                    }
                    printf("\n\nInventory is loaded into the restaurent\n\n");
                    NeedInventoryLock[i]->Acquire();
                    NeedInventoryCV[i]->Broadcast(NeedInventoryLock[i]);
                    NeedInventoryLock[i]->Release();
                  }
                else
                  {
                    printf("\n\nManager goes to bank to withdraw cash\n\n");
                    for (int iCount=0;iCount<100;iCount++)
                      {
                        currentThread->Yield();
                      }
                      MoneyInRstrnt = (MIN_REQUIRED_INVENTORY*40);
                  }
                MonitorAmtOfMoneyLock->Release();
              }

        }
      // Manager has nothing to do, so he will give CPU to others
      for (int i=0;i<200;i++)
      {
        currentThread->Yield();
      }
	  AliveCustCountLock->Acquire();
   	  if (AliveCustCount==0 )
		interrupt->Halt();
      AliveCustCountLock->Release();


    }//End of while

}

void Cook(int MyCookID)
{
  // When Cook thread is created, he just gets default values for
  // his status and speciality.
  // So cook needs only Speciality and status as input.

  int MyCookStyle = -1;
  int MyCookStat = WAITING;

   //Get the Cook specialty and set status as WAITING
  MgrCookStatUpdateLock->Acquire();
  if ( MgrCookStyleLst->IsEmpty())
    {
      // Get the default Speciality
       MyCookStyle = -1;
       MyCookStat = WAITING;
       MgrCookStyleLst->Append((void *) MyCookStyle); //Cooking spec
       MgrCookStyleLst->Append((void *) MyCookStat); // Cook status
    }
  MgrCookStatUpdateLock->Release();


  printf("COOK[%d] created\n",MyCookID);
  CookQLock->Acquire();
  CookQCV->Wait(CookQLock);
  CookQLock->Release();

  while(true)
  {
    //Check this cook speciality and status as set by manager
       MgrCookStatUpdateLock->Acquire();
       if( !MgrCookStyleLst->IsEmpty())
         {
           MyCookStyle = (int)MgrCookStyleLst->Remove();
           MyCookStat = (int)MgrCookStyleLst->Remove();
         }
       else
         {
           return; //error, Manager illegally waking the cook
         }
       MgrCookStatUpdateLock->Release();
       //  is Manager asking me to cook?
       if(Inventory[MyCookStyle]>0 && MyCookStat==BUSY)
       {
          printf("Cook[%d] is going to cook [%s]\n",MyCookID,CookStyle[MyCookStyle]);

           //Delay the cooking process
          for(int i=0;i<10;i++)
          {
            currentThread->Yield();
          }
          CookedFoodLock->Acquire();
          CookedFood[MyCookStyle]++;
          printf("Cook[%d] Cooked %s UPDATED VALUE:%d\n",MyCookID,CookStyle[MyCookStyle],CookedFood[MyCookStyle]);
          CookedFoodLock->Release();

          InventoryLock->Acquire();
          Inventory[MyCookStyle]--;
            //printf("Cook[%d] CookedRawMaterial:%d REMAINING VALUE:%d\n",CookID,MyCookStyle,RawMaterials[MyCookStyle]);
          InventoryLock->Release();
       }
       // There is no inventory to cook
       else
        {
          // cook should wait until inventory is loaded
          NeedInventoryLock[MyCookStyle]->Acquire();
          NeedInventoryCV[MyCookStyle]->Wait(NeedInventoryLock[MyCookStyle]);
          NeedInventoryCV[MyCookStyle]->Signal(NeedInventoryLock[MyCookStyle]);
          NeedInventoryLock[MyCookStyle]->Release();
        }

    MgrCookStatUpdateLock->Acquire();
     // Did Manager asked to stop cooking?
     if(MyCookStat==WAITING)
      {
        // Manager wants me to go on break;
        // Add my self to free cooks list
        MgrCookStyleLst->Append((void *) -1); //Cooking spec
        MgrCookStyleLst->Append((void *) MyCookStat); // Cook status
        printf("Cook[%d] is going on break as MAX FOOD REACHED\n",MyCookID);
        CookQLock->Acquire();
        CookQCV->Wait(CookQLock);
        printf("Cook[%d] returned from break\n",MyCookID);
        CookQLock->Release();
      }
      else
      {
        //Continue cooking
      }
      MgrCookStatUpdateLock->Release();

  }//End of While()

}




void SimiluationMenu()
{

 int Choice = -1;
 printf("*********************************************\n");
 printf("Welcome to Carl's Junior restaurant Simulation\n");
 printf("*********************************************\n");
 printf("Below select one of the below test cases to simulate the Carl's Junior restaurant\n\n");
 printf("Choice#1:\n");
 printf("TestCase#1: Customers who wants to eat-in, must wait if the restaurant is full.\n\n");

 printf("Choice#2:\n");
 printf("TestCase#2: OrderTaker/Manager gives order number to the Customer when the food is not ready.\n\n");

 printf("Choice#3:\n");
 printf("TestCase#3: Customers who have chosent to eat-in, must leave after they have their food\n");
 printf("and Customers who have chosen to-go,\n");
 printf("must leave the restaurant after the OrderTaker/Manager has given the food.\n");


 printf("Choice#4:\n");
 printf("TestCase#4: Manager maintains the track of food inventory. Inventory is refilled when it goes below order level.\n\n");

 printf("Choice#5:\n");
 printf("TestCase#5: A Customer who orders only soda need not wait.\n\n");

 printf("Choice#6:\n");
 printf("TestCase#6: The OrderTaker and the Manager both somethimes bag the food.\n\n");

 printf("Choice#7:\n");
 printf("TestCase#7: Manager goes to the bank for cash when inventory is to be refilled and there is no cash in the restaurant.\n\n");

 printf("Choice#8:\n");
 printf("TestCase#8:Cooks goes on break when informed by manager.\n\n");

 printf("Choice#9:\n");
 printf("TestCase#8:COMPLETE SIMULATION.\n\n");


 scanf("%d",&Choice);

 if (Choice!=-1)
    {
       switch (Choice)
        {
            case 1:  test_case =1;
                     PersonCount.NoOfCust = 9;
                     PersonCount.NoOfOT = 3;
                     PersonCount.NoOfCooks = 4;
                     PersonCount.NoOfWaiters = 3;
                     TotalTablesUpdateLock->Acquire();
                     TotalTables = 0;
                     TotalTablesUpdateLock->Release();
                     CarlJrSimulation();
                     break;
            case 2:
             case 3:  test_case =1;
                      PersonCount.NoOfCust = 9;
                      PersonCount.NoOfOT = 3;
                      PersonCount.NoOfCooks = 4;
                      PersonCount.NoOfWaiters = 3;
                      CarlJrSimulation();
                      break;
              case 4:  test_case =1;
                       PersonCount.NoOfCust = 9;
                       PersonCount.NoOfOT = 3;
                       PersonCount.NoOfCooks = 4;
                       PersonCount.NoOfWaiters = 3;
                       InventoryLock->Acquire();
                       for(int i=0;i<3;i++)
                       {
                         Inventory[i] = 50;
                       }
                       MonitorAmtOfMoneyLock->Acquire();
                       MoneyInRstrnt = 50000;
                       MonitorAmtOfMoneyLock->Release();
                       InventoryLock->Release();
                       CarlJrSimulation();
                       break;
             case 5:  test_case =1;
                      PersonCount.NoOfCust = 9;
                      PersonCount.NoOfOT = 3;
                      PersonCount.NoOfCooks = 4;
                      PersonCount.NoOfWaiters = 3;
                       printf("*********************************************\n");
                      printf(" Please observe that Customers numbers 4, 8, 12 and so on always\n");
                      printf("order sodas\n");
                       printf("*********************************************\n");
                      CarlJrSimulation();
                      break;
            case 6:  test_case =1;
                      PersonCount.NoOfCust = 9;
                      PersonCount.NoOfOT = 3;
                      PersonCount.NoOfCooks = 4;
                      PersonCount.NoOfWaiters = 3;
                      CarlJrSimulation();
                      break;
              case 7:  test_case =1;
                       PersonCount.NoOfCust = 9;
                       PersonCount.NoOfOT = 3;
                       PersonCount.NoOfCooks = 4;
                       PersonCount.NoOfWaiters = 3;
                       InventoryLock->Acquire();
                       for(int i=0;i<3;i++)
                       {
                         Inventory[i] = 50;
                       }
                       InventoryLock->Release();
                       CarlJrSimulation();
                       break;
                case 8:  test_case =1;
                          PersonCount.NoOfCust = 9;
                          PersonCount.NoOfOT = 3;
                          PersonCount.NoOfCooks = 4;
                          PersonCount.NoOfWaiters = 3;
                          CarlJrSimulation();
                          break;
                 case 9: test_case = 0;
                         CarlJrSimulation();
                          break;
            default: test_case = 0;
                         CarlJrSimulation();
                          break;

        }
    }
  else
    {
       printf("Invalid Choice\n");
       Abort();
    }

}


#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");          // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock

    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
        t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();  // Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
        t1_l1.getName());
    for (int i = 0; i < 10; i++)
    ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock

    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
    printf("%s: Trying to release Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void
t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
    t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
    t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//   4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;

    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
    t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t3_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
    t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t4_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
    t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif
