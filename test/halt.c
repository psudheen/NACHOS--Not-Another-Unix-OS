#include "syscall.h"
int main()
{
     int otLock[3], j,dummycv;
		 int mv1,mv2,mv3,m4;
		 
		 
    Print("Waiting for network setup\n");
    StartSimulation();
    
    otLock[0]=CreateLock("otlk",4);
		otLock[1]=CreateLock("otlk1",5);
		otLock[2]=CreateLock("otlk2",5);
		for(j=0;j<3;j++)
    Print1("the ot lock index is-------- %d\n",otLock[j]);
		dummycv=CreateCondition("dlcv",4);
		
		
		Acquire(otLock[0]);
		Wait(dummycv,otLock[0]);
		Print("I am  out of wait!!!!\n");
		Release(otLock[0]);
		Print("released the otlock in halt\n");
}
