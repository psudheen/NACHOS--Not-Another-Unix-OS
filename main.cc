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

#ifdef THREADS
    ThreadTest();
    CarlJrSimulation();
#endif

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
    argCount = 1;
        if (!strcmp(*argv, "-z"))               // print copyright
            printf (copyright);
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
 int D6Bgr;
 int D3Bgr;
 int D4VBgr;
 int D2Ffries;
 int D1Soda;

};

void Customer(int CustID);
void OrderTaker(int OTID);
void Init_LockCV(int size);
void Init_CustomerData(CustomerData_t *CustData, int TypeOfFood, int CustID);


#define MAX_OT 100


typedef struct InventoryData_t
{
 int NoOfD6Bgr;
 int NoOfD3Bgr;
 int NoOfD4VBgr;
 int NoOf2DFfries;
 int NoOfD1Soda;
};
InventoryData_t  InvtryData;




CustomerData_t *CustData;

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

void Init_LockCV(int SIZE)
{
 int i;
 Lock *LockPointer;
 Condition *ConditionPointer;
 //OrderTakerStatus = new OrderTakerStatus [SIZE];
 for ( i =0; i< SIZE; i++)
  {
    LockPointer = new Lock("OrderTakerLock");
    ConditionPointer = new Condition("OTWaitingCV");
    OrderTakerLock[i] = LockPointer;
    OTWaitingCV[i] = ConditionPointer;
  }
}

//****************************************************************************************
void CarlJrSimulation (void)
{
    Thread *t;
    char *name;
    int i;

printf("Please enter the number of Customers: ");
scanf("%d", &(PersonCount.NoOfCust));
printf("Please enter the number of Order Takers: ");
scanf("%d", &(PersonCount.NoOfOT));
printf("Please enter the number of Cooks: ");
scanf("%d", &(PersonCount.NoOfCooks));
printf("Please enter the number of Waiters: ");
scanf("%d", &(PersonCount.NoOfWaiters));

Init_LockCV(PersonCount.NoOfOT);
printf("%d\n",PersonCount.NoOfOT);

//printf("Simulation starts here \n\n\n");
printf("Simulation starts here \n\n\n");
printf("%d\n",PersonCount.NoOfOT);
printf("Simulation starts here ");
printf("%d",PersonCount.NoOfOT);

//Create order taker threads
for (int i=0; i<PersonCount.NoOfOT; i++)
    {
        name = new char [20];
        sprintf(name,"OrderTaker[%d]",i);
        printf("Creating OT[%d] thread", i);
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
        printf("Creating Cust[%d] thread", i);
        t->Fork((VoidFunctionPtr)Customer,i);
    }

}

void Init_CustomerData(CustomerData_t *CustData, int TypeOfFood, int CustID)
{
 CustData->CustID = CustID;
 CustData->TypeOfFood = TypeOfFood; //Eat -in
  //Food pattern
  //Assuming eat-in customers will have only below food pattern
  switch ( CustID%5)
    {
     case 0: CustData->D6Bgr = 1;
             CustData->D3Bgr = 1;
             CustData->D4VBgr = 1;
             CustData->D2Ffries = 1;
             CustData->D1Soda = 1;
             break;
     case 1: CustData->D6Bgr = 1;
             CustData->D3Bgr = 0;
             CustData->D4VBgr = 1;
             CustData->D2Ffries = 0;
             CustData->D1Soda = 1;
             break;
     case 2: CustData->D6Bgr = 0;
             CustData->D3Bgr = 1;
             CustData->D4VBgr = 0;
             CustData->D2Ffries = 0;
             CustData->D1Soda = 1;
             break;
     case 3: CustData->D6Bgr = 1;
             CustData->D3Bgr = 0;
             CustData->D4VBgr = 1;
             CustData->D2Ffries = 0;
             CustData->D1Soda = 1;
             break;
     case 4: CustData->D6Bgr = 0;
             CustData->D3Bgr = 0;
             CustData->D4VBgr = 0;
             CustData->D2Ffries = 0;
             CustData->D1Soda = 1;
             break;
     default: CustData->D6Bgr = 0;
             CustData->D3Bgr = 0;
             CustData->D4VBgr = 0;
             CustData->D2Ffries = 0;
             CustData->D1Soda = 1;
             break;
    }
}



void Customer(int CustData)
{
  CustomerData_t *Cdata =(CustomerData_t *)CustData;
  int CustID = Cdata->CustID;
  int myOT=-1;
  // Just entered the restaurant, grab the lock and check the line length
  custLineLock->Acquire();
  //printf("Customer[%d] acquired the lock:%s\nand checking for his OT\n", CustID, custLineLock->getName());
  //Decide who is my OT
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
  //Here either I know who is my OT or there are no OT for me
  if(-1==myOT) //get in line
    {
        custLineLength++;
        printf("Customer[%d] has NO OrderTakers, Snoozing!\n", CustID);
        custWaitingCV->Wait(custLineLock);
    }
  //Get a waiting OT
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
 //

  //printf("Customer[%d] released the lock:%s\nand waiting for his OT\n", CustID, custLineLock->getName());
// By here, Customer is aware of his OT and ready to order his/her food
  custLineLock->Release(); //Allow OT to manipluate CustLineLength
  // It is OT turn now!
  // Take the transaction lock and signal the waiting OT that Cust is Done.
  //printf("Ordertaker[%d] acquired the lock:%s\n", myOT, OrderTakerLock[myOT]->getName());
  SigCustList->Append((void*) CustID);
  OrderTakerLock[myOT]->Acquire();
  printf("Customer[%d] is giving below order to OrderTaker[%d]\n",CustID,myOT);
  if(Cdata->D6Bgr == 0)
  printf("Customer[%d] is not ordering 6-dollar burger", CustID);
  else
  printf("Customer[%d] is ordering 6-dollar burger", CustID);
  if(Cdata->D3Bgr == 0)
  printf("Customer[%d] is not ordering 3-dollar burger", CustID);
  else
  printf("Customer[%d] is ordering 3-dollar burger", CustID);
  if(Cdata->D4VBgr == 0)
  printf("Customer[%d] is not ordering veggie burger", CustID);
  else
  printf("Customer[%d] is ordering veggie burger", CustID);
  if(Cdata->D2Ffries == 0)
  printf("Customer[%d] is not ordering French fries", CustID);
  else
  printf("Customer[%d] is ordering French fries", CustID);
  if(Cdata->D1Soda == 0)
  printf("Customer[%d] is not ordering Soda", CustID);
  else
  printf("Customer[%d] is ordering Soda", CustID);

  //CustComputedMoney[CustID] = (6 * Cdata->D6Bgr + 3 * Cdata->D3Bgr + 4 * Cdata->D4VBgr \
  //                              2 * Cdata->D2Ffries + Cdata->D1Soda);
  // Wake-up the WAITING OT who is waiting for my order.
  OTWaitingCV[myOT]->Signal(OrderTakerLock[myOT]);
  //printf("Customer[%d] signaled WAITING OrderTaker[%d] to respond\n", CustID,myOT );
  //Call a function that randomly assign the eating style
  // and different types of food
  // I'm done with my job. Release the interaction lock.
  //**********************************************************************
  //PENDING logic goes here
  //********************
  //**************************************************
  OTWaitingCV[myOT]->Wait(OrderTakerLock[myOT]);// Waiting for my order to be processed
  printf("Customer[%d] order is processed and he is leaving!!Die Customer, Die!!\n",CustID );
  OrderTakerLock[myOT]->Release();
  currentThread->Finish();
  // Probable upcoming code -
  // 1. Waiter interaction

}


void OrderTaker(int myOTId)
{
  int MyCustID = -1;
  while(true)
   {
    custLineLock->Acquire();
    //Are there any waiting customers?
    if(custLineLength > 0)
      {
        custWaitingCV->Signal(custLineLock);
        custLineLength--;
        OrderTakerStatus[myOTId]=WAITING;
      }
    //Is there any food to bag?
    //else if (true)
    //{
      //Food to bag Logic
      //Bag one order at a time => Inventory --
      // Alert the waiting customer
     // custLineLock->Release();
     // continue;
    //}
    else
      {
        //Nothing to do
        //Set OT status to FREE
        OrderTakerStatus[myOTId]=FREE;
      }

  //Now, OT is ready to take order
  // Take interaction lock
  OrderTakerLock[myOTId]->Acquire();
  //release hold on CustLineLen
  custLineLock->Release();
  //OT should wait until customer signals him with orders
  OTWaitingCV[myOTId]->Wait(OrderTakerLock[myOTId]);
  //When I'm here, Customer has signaled that his order is ready
  //Compute the money and handle the token number.

  //check which customer signalled me?
  if( SigCustList->IsEmpty())
    {
      printf("OrderTaker[%d]: ERROR!!Siganlling Customer list is empty!\n", myOTId);
      continue;
    }
  MyCustID = (int)SigCustList->Remove();

  MonitorAmtOfMoneyLock->Acquire();
  MoneyInRstrnt = 6*CustData[MyCustID].D6Bgr + 3*CustData[MyCustID].D3Bgr + 4*CustData[MyCustID].D4VBgr \
                  + 2*CustData[MyCustID].D2Ffries + CustData[MyCustID].D1Soda;
  MonitorAmtOfMoneyLock->Release();

  printf("OrderTaker[%d] processed the Customer [%d] order and acknowledged the payment\n", myOTId, MyCustID);

  //Order processed Signal the customer to get out of sleep!
  OTWaitingCV[myOTId]->Signal(OrderTakerLock[myOTId]);

  OrderTakerLock[myOTId]->Release();

  // Check the customer style of eating - eat in or to-go?
  // if eat-in --
    // If
       // food is only Soda, No token is given
       // and customer is siganlled that food is ready and
       //customer leaves the line to grab a table (Manager interaction)
    // else
      //Handle customer the token
      //Handle same token to waiter
      //if
        // food is available in inventory, signal the waiter
     // else,
       //set OT status to free and signal next waiting customer
   }//end of while()
}//end of OrderTaker()
