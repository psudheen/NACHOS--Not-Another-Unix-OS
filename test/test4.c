#include "syscall.h"

 void client6()
{
   int Lock1,CV1;
   Lock1=CreateLock("USERLOCK",8);
   CV1=CreateCondition("USERCV",6);
  
  Acquire(Lock1);
   Signal(CV1,Lock1);
   Release(Lock1);
  
  
 }
 
 
  void client7()
 {
      int Lock1,CV1;
  
	  Lock1=CreateLock("USERLOCK",8);
	  CV1=CreateCondition("USERCV",7);
	  
	  Acquire(Lock1);
	  Broadcast(CV1,Lock1);
	  Release(Lock1);
  
  }
  
  
int main()
{
	int Choice = -1;
	
	  Print((int)"*************************************client 4****************************************\n");

        
		
		Print("Choice#1:\n");
		Print("TestCase#1:Signals RPC is implemented, this test case shows signal RPC wakes up only 1 thread\n\n");
		
		Print("Choice#2:\n");
		Print("TestCase#1:Broadcast RPC is implemented, this test case shows broadcast RPC wakes up all the thread\n\n");
		
		
		
		Choice=Scan("%d");
		 switch (Choice)
			{
                    case 1: 
							client6();
							
							break;
							
					case 2: 
							client7();
							
							break;
					
			}
}