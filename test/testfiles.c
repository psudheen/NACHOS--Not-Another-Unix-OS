/* testfiles.c
 *	Simple program to test the file handling system calls
 */
#include "syscall.h"
#ifndef NULL
#define NULL    0
#endif
void TestCreateLock();
int Lock1;
int Lock2;
int Lock3;
int CV1;
int CV2;
int CV3;
int CV4;
int i,j;
void t9();

int main()
{
	int Choice = -1;
 Write("*********************************************\n",46,ConsoleOutput);
 Write("Test Cases for System Calls\n",28,ConsoleOutput);
 Write("*********************************************\n",46,ConsoleOutput);
 Write("Select one of the below test cases\n\n",36,ConsoleOutput);
 Print("Choice#1:\n");
 Print("TestCase#1:Tests the CreateLock,AcquireLock,ReleaseLock,DestroyLock Syscalls \n\n");

 Print("Choice#2:\n");
 Print("TestCase#2:Other Tests on Locks\n\n");

 Print("Choice#3:\n");
 Print("TestCase#3:Tests the Create Condition and Destroy Condition Syscalls\n\n");

 Print("Choice#4:\n");
 Print("TestCase#4:Tests for Wait, Signal and Broadcast Syscalls\n\n");

 Print("Choice#5:\n");
 Print("TestCase#5:Tests Fork Syscall and Yield Syscall \n\n");
 
 Print("Choice#6:\n");
 Print("TestCase#6:Single instance of CarlJr Simulation\n\n");
 
 Choice=Scan("%d");

  switch (Choice)
  {
      case 1: TestsForCreateLock();
							TestsForAcquireLock();
							TestsForReleaseLock();
							TestsForDestroyLock();
              break;
     
      case 2: TestsOnLocks();
              break;
     
      case 3: TestsForCreateConditionDestroyCondition();
              break;
     
      case 4: TestsForWaitSignalBroadcast();
              break;

			case 5: TestsForForkandYield();
              break;
			case 6: Print("Number of OrderTakers=4\n");
							Print("Number of Waiters =3\n");
							Print("Number of Cooks=4\n");
							Print("Number of Customers=20\n");
							Fork(t9);
							break;
			default: Print("Invalid Choice\n");
								break;
  }
	Exit(1);
 } 
/* end of main()*/

 
 

 

/*--------------------------------------------------TEST CASES--------------------------------------*/
void TestsForCreateLock()
{
	Print("\nValid CreateLock System calls:\n");
	Lock1=CreateLock("USERLOCK1",9);/*Valid*/
	Print1("Lock1=%d\n",Lock1);
    Lock2=CreateLock("USERLOCK2",9);/*Valid*/
	Print1("Lock2=%d\n",Lock2);
	Print("\nPassing invalid values for address length:\n");
	Lock3=CreateLock("USERLOCK",-6);/*Invalid Addr Length*/
	Print1("Lock3=%d\n",Lock3);
	Lock3= CreateLock("USERLOCK",65000);/*Address Length exceeded address size*/
	Print1("Lock4=%d\n",Lock3);
    Lock3=CreateLock("USERLOCK",0);/*Invalid Address Length*/
	Print1("Lock5=%d\n",Lock3);	   
} 

 void TestsForAcquireLock()
{
	 Print("\nTry to Acquire a Valid Lock:\n");
     Acquire(Lock2);/*Valid*/
	 Print("\nPassing invalid values as Lock Index:\n");
     Acquire(-1);/*InValid Lock Index*/
     Acquire(3000);/*InValid Lock Index*/
	 Acquire(Lock3);/*Invalid Lock*/
	 Print("\nTry to Acquire a lock that was not created\n");
	 Acquire(10);
}

void TestsForReleaseLock()
{
	  Print("\nTry to Release a Valid Lock:\n");
      Release(Lock2);/*Valid*/
	  Print("\nPassing invalid values as Lock Index:\n");
      Release(-1);/*InValid Lock Index*/
      Release(3000);/*InValid Lock Index*/
	  Print("\nTry to Release a lock that was not Acquired\n");
	  Release(Lock1); /*Invalid Lock*/
	  Print("\nTry to Release a lock that was not Created\n");
	  Release(Lock3);/*Invalid Lock*/
}

void TestsForDestroyLock()
{
	  Print("\nTry to Destroy a Valid Lock:\n");
      DestroyLock(Lock2);/*Valid*/
	  Print("\nPassing invalid values as Lock Index:\n");
      DestroyLock(-1);/*InValid Lock Index*/
      DestroyLock(3000);/*InValid Lock Index*/
	  Print("\nTry to Destroy a lock that was not Created\n");
	  DestroyLock(Lock3);/*Invalid Lock*/
}


void TestsOnLocks()
{
     Lock1=CreateLock("USERLOCK",8);/*Valid*/
      Acquire(Lock1);/*Valid Lock Id*/
	  Print("\nTry to destroy a lock that has not been released\n");
      DestroyLock(Lock1);/*Invalid since there is a lock that has acquired the lock*/
	  Release(Lock1);
	  DestroyLock(Lock1);
	  Print("\nTry to acquire a lock that has been destroyed\n");
      Acquire(Lock1);/*Invalid since lock has been destroyed"*/
 }


void TestsForCreateConditionDestroyCondition()
{
      Print("\nCreate a CV\n");
      CV1=CreateCondition("USERCV",8);/*Valid*/
	  Print("\nPassing invalid values for address length:\n");
      CV2=CreateCondition("USERCV",-2);/*Invalid Addr Length*/
      CV3=CreateCondition("USERCV",400000);/*Address Length exceeded address size*/
      CV4=CreateCondition("USERCV",0);/*Invalid Address Length*/
	  Print("\nDestroy a Valid CV\n");
      DestroyCondition(CV1);/*Valid*/
	  Print("\nPassing invalid values as CV Index:\n");
	  DestroyCondition(-2);
	  DestroyCondition(4000);
	  Print("\nTry to Destroy a CV that was not Created\n");
      DestroyCondition(CV2);/*Invalid*/
	  
}

void t1()
{
      Acquire(Lock1);
	  Print("\nPass invalid parameters to wait:\n");
	  Wait(-45,Lock1);
	  Wait(CV1,5000);
	  Wait(CV2,Lock1);
	  Print("\nTry to wait on the wrong Lock:\n");
	  Wait(CV1,Lock2);
	  Print("\nA Signal with no thread waiting should be ignored:\n");
	  Signal(CV1,Lock1);
	  Print("\nt1 is going on Wait \n");
      Wait(CV1,Lock1);
      Print("\n t1 was signalled \n");
	  Release(Lock1);
      Exit(0);
}
void t2()
{
      Acquire(Lock1);
	  Print("\nPass invalid parameters to signal:\n");
	  Signal(-45,Lock1);
	  Signal(CV1,5000);
	  Signal(CV2,Lock1);
	  Print("\nTry to signal on the wrong Lock:\n");
	  Signal(CV1,Lock2);
	  Print("\nt2 is going to Signal t1 \n");
      Signal(CV1,Lock1);
	  Release(Lock1);
      Exit(0);
}

void t3()
{
      Acquire(Lock1);
      Print("\nt3 is going on Wait \n");
      Wait(CV1,Lock1);
      Print("\n t3 woke up \n");
	  Release(Lock1);
      Exit(0);
}

void t4()
{
      Acquire(Lock1);
      Print("\nt4 is going on Wait \n");
      Wait(CV1,Lock1);
      Print("\n t4 woke up \n");
	  Release(Lock1);
      Exit(0);
}
void t5()
{
      Acquire(Lock1);
	  Print("\nPass invalid parameters to broadcast:\n");
	  Broadcast(-45,Lock1);
	  Broadcast(CV1,5000);
	  Broadcast(CV2,Lock1);
      Print("\nt5 is going to Broadcast\n");
      Broadcast(CV1,Lock1);
	  Release(Lock1);      
      Exit(0);
}

void TestsForWaitSignalBroadcast()
{
	Lock1=CreateLock("USERLOCK",8);/*Valid*/
    CV1=CreateCondition("USERCV",8);/*Valid*/
	Lock2=CreateLock("USERLOCK",8);/*Valid*/
	CV2=CreateCondition("USERCV",-2);/*Invalid Addr Length*/
	CV3=CreateCondition("USERCV",6);/*Invalid Addr Length*/


	Fork(t1);
	Fork(t2);
	for(j=0;j<3;j++)
	{
		Yield();
	}

	Fork(t3);
	Fork(t4);
	Fork(t5); 
}		  

void FunctionToFork()
{
	Print("Function forked successfully\n");
}	


void TestsForForkandYield()
{
   /* Print("\nForking a valid function:\n");
	Fork(FunctionToFork);  */
	Print("\nForking multiple times:\n");
	for(i=0;i<30;i++)
	{
		Fork(FunctionToFork);
		for(j=0;j<10;j++)
		{
			Yield();
		}
	}

}

void t9()
{
		Exec("../test/carljrsim",17);
}