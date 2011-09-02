//customer.cc
//
// Combined simulation of the customer interaction in the
// Carl's Jr restaurant
//
//


#include "app_init.h"
#include <stdio.h>

int custLineLength = 0; //Monitor Variable to track the Customer line length
// CVs' to schedule interaction between Customer and OT
Condition *custWaitingCV;
//Condition *orderTakerCV[MAX_OT];

Lock *custLineLock;
//WorkerStatus OTStatus[MAX_OT];
int myOT=-1;


void Customer(int CustID)
{

  // Just entered the restaurant, grab the lock and check the line length
  custLineLock->Acquire();
  printf("Customer[%d] acquired the lock:%s\nand checking for his OT", CustID, custLineLock->getName());
  //Decide who is my OT
  for(int i=0;i<MAX_OT;i++)
    {
      if(FREE == OT[i].OTStatus) //checking for a free order taker
      {
          OT[i].OTStatus = BUSY;;//make ith (my) OT busy
          myOT=i;
          printf("Customer[%d] has OrderTaker[%d]", CustID, myOT);
          break;
      }
    }
  //Here either I know who is my OT or there are no OT for me
  if(-1==myOT) //get in line
    {
        custLineLength++;
        printf("Customer[%d] has NO OrderTakers, Snoozing!", CustID);
        custWaitingCV->Wait(custLineLock);
        printf("Snoozing Customer[%d] has woken up by OrderTaker[%d]", CustID, myOT);
    }
  //Get a waiting OT
  for(int j=0;j<MAX_OT;j++)
  {
      if (WAITING == OT[j].OTStatus)
      {
        myOT=j;
        OT[j].OTStatus=BUSY;
        printf("Waiting Customer[%d] has OrderTaker[%d]", CustID,j);
        break;
      }
  }
  printf("Customer[%d] released the lock:%s\nand waiting for his OT", CustID, custLineLock->getName());
// By here, Customer is aware of his OT and ready to order his/her food
  custLineLock->Release(); //Allow OT to manipluate CustLineLength
  // It is OT turn now!
  // Take the transaction lock and signal the waiting OT that Cust is Done.
  printf("Ordertaker[%d] acquired the lock:%s", myOT, OT[myOT].OrderTakerLock->getName());
  OT[myOT].OrderTakerLock->Acquire();
  // Wake-up the WAITING OT who is waiting for my order.
  OT[myOT].OrderTakerCV->Signal(OT[myOT].OrderTakerLock);
  printf("Customer[%d] signaled WAITING OrderTaker[%d] to respond", CustID,myOT );
  //Call a function that randomly assign the eating style
  // and different types of food
  // I'm done with my job. Release the interaction lock.
  //**********************************************************************
  //PENDING logic here  goes here
  //**********************************************************************
  OT[myOT].OrderTakerLock->Wait(OT[myOT].OrderTakerLock);// Waiting for my order to be processed
  printf("Customer[%d] order is processed and he is leaving!!Die Customer, Die!!",CustID )
  // Probable upcoming code -
  // 1. Waiter interaction

}


void OrderTaker(int myOTId)
{
  while(true)
   {
    custLineLock->Acquire();
    //Are there any waiting customers?
    if(custLineLength > 0)
      {
        custWaitingCV->Signal(custLineLock);
        custLineLength--;
        OT[myOTId].OTStatus=WAITING;
      }
    //Is there any food to bag?
    else if
    {
      //Food to bag Logic
      //Bag one order at a time => Inventory --
      // Alert the waiting customer
      custLineLock->Release();
      continue;
    }
    else
      {
        //Nothing to do
        //Set OT status to FREE
        OT[myOTId].OTStatus=FREE;
      }

  //Now, OT is ready to take order
  // Take interaction lock
  OT[myOTId].OrderTakerLock->Acquire();
  //release hold on CustLineLen
  custLineLock->Release();
  //OT should wait untill customer signals him with orders
  OT[myOTId].OrderTakerCV->Wait(OT[myOTId].OrderTakerLock);
  //When I'm here, Customer has signalled that his order is ready
  printf("Customer[%d] is giving below order to OrderTaker[%d]\n",,myOTId );
  printf("My order - blah\nblah\blah\n")
  //Order processed Signal the customer to get out of sleep!
  OT[myOTId].OrderTakerCV->Signal(OT[myOTId].OrderTakerLock);


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