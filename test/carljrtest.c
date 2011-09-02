#include "syscall.h"

void main()
 {
	Print("Number of OrderTakers=4\n");
	Print("Number of Waiters =3\n");
	Print("Number of Cooks=4\n");
	Print("Number of Customers=20\n");
	Exec("../test/carljrsim",17);
	Exec("../test/carljrsim",17);
	Exit(1);
}