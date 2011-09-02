#include "syscall.h"

void main()
 {
	/* Print("Number of OrderTakers=4\n");
	Print("Number of Waiters =3\n");
	Print("Number of Cooks=4\n");
	Print("Number of Customers=20\n");
	Exec("../test/carljrsim",17);
	Exec("../test/carljrsim",17); */
	/* Print("exec of matmult\n");	
    Exec("../test/matmult",15); */
	/* Exec("../test/halt",15); */
	
	/*Exit(1);*/ 
	int i,j,k;
/* 	i=CreateMV("TestMV",6);
	i=CreateMV("TestMV",6);
	i=CreateMV("TestMV",6);
	i=CreateMV("TestMV",6);
	
	SetMV(i,10);
	SetMV(i,10000000000000000000);
	SetMV(i,-100);
	j=GetMV(i);
		Print1("%d after set value\n", j);
	DestroyMV(i);
	DestroyMV(i);
	DestroyMV(i); */
	
	
/* 	i=CreateLock("MyLock",6);
	DestroyLock(i);
	i=CreateLock("MyLock",6);
	j=CreateLock("MyLock",6);
	DestroyLock(j);
	DestroyLock(i); */
	
	i=CreateLock("MyLock",6);
	Acquire(i);
	DestroyLock(i);
	Release(i);
	DestroyLock(i);
	
}