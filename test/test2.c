#include "syscall.h"



/*client 2 function shows proper functioning of signal and wait calls*/


void client2()
{ 
 int Lock1,CV1;
 
 Lock1=CreateLock("USERLOCK",8);
 CV1=CreateCondition("USERCV",6);
 

 
  Acquire(Lock1);
  Signal(CV1,Lock1);
  Release(Lock1);
 
}


void client3()
{
  int Lock1,CV1;

  Lock1=CreateLock("USERLOCK",8);
  CV1=CreateCondition("USERCV",6);
  
  Acquire(Lock1);
  Wait(CV1,Lock1);
  
 }
 
 
 /*Subsequent clients cannot acquire the lock until the previous clients release them */

 void Lockacquiretest()
{
  int La;
  
  La=CreateLock("La",2);/*LOCK is created*/
  Acquire(La);
  Print("Enter a key to release the lock\n");
  Scan("%d");
  Release(La);
  
}
 
 /* To show that a lock or a CV cannot be destroyed by other client if this client is using it*/
 
void Lockdestroytest()
{
  int lockd,cvd;
  
  lockd=CreateLock("lockd",5);
  DestroyLock(lockd);/* you cannot destroy as client 1 is also using this lock*/
  cvd=CreateCondition("cvd",3);
  DestroyCondition(cvd);
}

void lockcvdestroytest()
{
   int lockd,cvd;
   
  lockd=CreateLock("lockd",5);
  cvd=CreateCondition("cvd",3);
  DestroyLock(lockd);
  DestroyCondition(cvd);
  
}


  

int main()
{
	int Choice = -1;
	
	  Print((int)"*************************************client 2****************************************\n");

        
		
		Print("Choice#1:\n");
		Print("TestCase#1: Shows that signal only wakes up one thread, this therad goes on wait\n\n");
		
		Print("Choice#2:\n");
		Print("TestCase#2:shows that broadcast wakes up all the threads,this therad goes on wait\n\n");
		
		Print("Choice#3:\n");
		Print("TestCase#3:Tests the proper functioning of signal and wait \n\n");
		
		Print("Choice#4:\n");
		Print("TestCase#4:Subsequent clients cannot acquire the lock until the previous clients release them \n\n");
		
		Print("Choice#5:\n");
		Print("TestCase#5: To show that a lock or a CV cannot be destroyed by other client if this client is using it\n\n");
		
		Print("Choice#6:\n");
		Print("TestCase#6: To show that a lock or a condition variable can only be destoyed if all clients using it call destroy\n\n");
		
		
		
		Choice=Scan("%d");
		 switch (Choice)
			{
                    case 1: 
							client3();
							break;
							
					case 2: 
							client3();
							break;
							
					case 3: Print("This test case checks the proper functioning of wait and signal RPC\n");
					        client2();
                            break;
							
					case 4: Print("Susequent clients can acquire the lock only when this client releases\n");
					        Lockacquiretest();
                            break;
							
					case 5: Print("To show that a lock or a CV cannot be destroyed by other client if this client is using it\n");
					        Lockdestroytest();
                            break;
							
							
					case 6:	Print("To show that a lock or a condition variable can only be destoyed if all clients using it call destroy\n");
					        lockcvdestroytest();
							break;
			}
}