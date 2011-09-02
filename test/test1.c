#include "syscall.h"




/*client 1 functions show the proper functioning of signal and wait calls*/
void client1()
{

 int Lock1,CV1,CV2;
 Lock1=CreateLock("USERLOCK",8);
 CV1=CreateCondition("USERCV",6);

 
 
  Acquire(Lock1);
  Wait(CV1,Lock1);
  Release(Lock1);
  
}

void client3()
{
  int Lock1,CV1;

  Lock1=CreateLock("USERLOCK",8);
  CV1=CreateCondition("USERCV",6);
  
  Acquire(Lock1);
  Wait(CV1,Lock1);
  Release(Lock1);
  
 }
 
/* Negative Test cases for Createlock and Createcv*/
 
 void negativecase1()
 {
   int Lock1,CV1;
   
   Lock1=CreateLock("USERLOCK",-6);/*Invalid Addr Length*/
   Lock1= CreateLock("USERLOCK",65000);/*Address Length exceeded address size*/
   CV1=CreateCondition("USERCV",-2);/*Invalid Addr Length*/
   CV1=CreateCondition("USERCV",400000);/*Address Length exceeded address size*/
   CV1=CreateCondition("USERCV",0);/*Invalid Address Length*/
  
  }
  
  /*Negative Test cases for Destroylock and Destroycv*/
  
 void negativecase2()
  { 
      int Lock9=-1;
	  int Lock1;
	  int CV1;
	  int CV9=-1;
      DestroyLock(-1);/*InValid Lock Index*/
	  DestroyLock(3000);/*InValid Lock Index*/
	  DestroyLock(Lock9);/*Cannot delete a lock whic is not created */
	  Lock1= CreateLock("USERLOCK",8);
	  Acquire(Lock1);
	  DestroyLock(Lock1);/*trying to delete a lock whose usage counter is non zero*/
	  DestroyCondition(-1);/*InValid CV Index*/
	  DestroyCondition(3000);/*InValid CV Index*/
	  DestroyCondition(CV9);/*Cannot delete a CV which is not created */
	  Release(Lock1);
	  DestroyLock(Lock1);
	  DestroyLock(Lock1);/* try to delete a lock which is already deleted*/
	  CV1=CreateCondition("cv1",3);
	  DestroyCondition(CV1);
	  DestroyCondition(CV1);/*trying to delete a cv which is already deleted*/
	  
	  
  }
  
  /*Acquire and release negative test cases*/
  
  void negativecase3()
  {
    int L2;
	int Lock9,Lock8;
	Acquire(Lock9);/*trying to acquire a lock which is not created*/
	Release(Lock8);/*Releasing a lock which is not created*/
	L2= CreateLock("l2",2);/*LOCK is created*/
	DestroyLock(L2);
	Acquire(L2);/*tring to acquire a lock which is deleted*/
	Release(L2);/*Trying to release a lock which is already deleted*/
	
	
  }
  
  /*Negative test cases for signal and wait */
  
  void negativecase4()
  {
     int n1,c1;
	 
	 /*n1=CreateLock("n1",2);*/
	 c1=CreateCondition("c1",2);
	 Signal(c1,n1);/*signalling without owning the lock n2*/
	 Wait(c1,n1);/*going on wait without acquring n1*/
	 
   }
 
 /*Positive test case for creation and deletion of LOCK AND CONDITION*/
 
void positivetestcase1()
{
  int L1,cv1;
  
  L1=CreateLock("USERLOCK",8);/*LOCK is created*/
  cv1=CreateCondition("USERCV",6);/*CV is created*/
  DestroyLock(L1);/*Lock is destroyed*/
  DestroyCondition(cv1);/*CV is destroyed*/
  
  
  
}

/* Proper functioning of ACQUIRE and RELEASE LOCK*/

void positivetestcase2()
 
 {
       int Lock1,Lock2;
	   Lock1= CreateLock("USERLOCK",8);/*LOCK is created*/
	   Lock2= CreateLock("USERLOCK1",9);/*LOCK is created*/
	   Acquire(Lock1);
	   Acquire(Lock2);
	   Release(Lock1);
	   Release(Lock2);
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
  cvd=CreateCondition("cvd",3);
}

/*To show that a lock or a condition variable can only be destoyed if all clients using it call destroy*/

void lockcvdestroytest()
{
   int lockd,cvd;
   
  lockd=CreateLock("lockd",5);
  cvd=CreateCondition("cvd",3);
  
  Print("Please press any number after you run client 2, for successfull deletion of the lock and cv\n");
  Scan("%d");
  DestroyLock(lockd);
  DestroyCondition(cvd);
  
  
}
  
 
int main()
{
	int Choice = -1;
	
	   Print((int)"*************************************client 1**************************************** \n");

        Print("enter the test case choice \n"); 
		
		
		/*Print("TestCase#1:  Shows that signal only wakes up one thread,this thread goes on wait\n\n");
		
		
		Print("TestCase#2:  shows that broadcast wakes up all the threads,this thread goes on wait\n\n");
		
		
		Print("TestCase#3:  Tests the proper functioning of signal and wait \n\n");
		
		
		Print("TestCase#4:  Negative Test cases for Createlock and Createcv\n\n");
		
		Print("TestCase#5:  Negative Test cases for Destroylock and Destroycv\n\n");
		
		
		Print("TestCase#6:  Proper functioning of Create and Destroy Lock and condition\n\n");
		
		Print("TestCase#7:  Proper functioning of Acquire and Release \n\n");
		
		
		Print("TestCase#8:  Negative test cases for acquire and release \n\n");
		
		
		Print("TestCase#9:  Negative test cases for signal and wait \n\n");
		
		Print("TestCase#10: Subsequent clients cannot acquire the lock until the previous clients release them \n\n");
		
		Print("TestCase#11: To show that a lock or a CV cannot be destroyed by other client if this client is using it \n\n");
		
		Print("TestCase#12: To show that a lock or a condition variable can only be destoyed if all clients using it call destroy \n\n"); */
		
		
		
		Choice=Scan("%d");
		 switch (Choice)
			{
                    case 1: 
					        client3();
							break;
							
					case 2: client3();
							break; 
							
					case 3: Print("This test case checks the proper functioning of wait and signal RPC\n");
					        client1();
					       
                            break;
							
					case 4: Print("The following are the negative test cases which will return error messages\n\n");	
                            negativecase1();
                            break; 	

                    case 5: Print("The following are the negative test cases which will return error messages\n\n\\");	
                            negativecase2();
                            break; 	

                    case 6:	Print("Lock and CV are suceesfully created and deleted\n\n");	
                            positivetestcase1();					
                            break;	
                    
                    case 7:	Print("Locks are suceefully acquired and released\n\n");	
                            positivetestcase2();					
                            break;	
                     
                    case 8:	Print("We get error messages\n\n\n");	
                            negativecase3();					
                            break;

				    case 9:	Print("We get error messages\n\n\n");	
                            negativecase4();					
                            break;	
							
					case 10:Print("Susequent clients can acquire the lock only when this client releases\n");
					        Lockacquiretest();
							break;
							
					case 11:Print("To show that a lock or a CV cannot be destroyed by other client if this client is using it\n");
					        Lockdestroytest();
							break;
							
					case 12:Print("To show that a lock or a condition variable can only be destoyed if all clients using it call destroy\n");
					        lockcvdestroytest();
							break;
			}
}