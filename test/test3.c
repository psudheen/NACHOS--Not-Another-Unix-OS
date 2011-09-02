#include "syscall.h"

void client3()
{
  int Lock1,CV1;

  Lock1=CreateLock("USERLOCK",8);
  CV1=CreateCondition("USERCV",6);
  
  Acquire(Lock1);
  Wait(CV1,Lock1);
  Release(Lock1);
  
 }
 
 
 void Lockacquiretest()
{
  int La;
  
  La=CreateLock("La",2);/*LOCK is created*/
  Acquire(La);
  Print("Enter a key to release the lock\n");
  Scan("%d");
  Release(La);
  
}
 
int main()
{
	int Choice = -1;
	
	  Print((int)"*************************************client 3**************************************** \n");

        
		
		Print("Choice#1:\n");
		Print("TestCase#1: Shows that signal only wakes up one thread\n\n");
		
		Print("Choice#2:\n");
		Print("TestCase#1:shows that broadcast wakes up all the threads\n\n");
		
		Print("Choice#3:\n");
		Print("TestCase#3:Subsequent clients cannot acquire the lock until the previous clients release them \n\n");
		
		
		
		Choice=Scan("%d");
		 switch (Choice)
			{
                    case 1: Print("The thread goes to wait\n");
							client3();
							break;
							
					case 2: Print("The thread goes to wait\n");
							client3();
							break;
							
					case 3: Print("Susequent clients can acquire the lock only when this client releases\n");
					        Lockacquiretest();
                            break;
							
					
			}
}