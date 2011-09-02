/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int A[1024];	/* size of physical memory; with code, we'll run out of space!*/
int B[1024];

int
main1()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)		
        A[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
		   Print1("the output of sorting operation is =%d\n",A[0]);
    Exit(A[0]);		/* and then we're done -- should be 0! */
}
int
main2()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)		
        B[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (B[j] > B[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = B[j];
	      B[j] = B[j + 1];
	      B[j + 1] = tmp;
    	   }
		   Print1("the output of sorting operation is =%d\n",A[0]);
    Exit(B[0]);		/* and then we're done -- should be 0! */
}
  void main()
  {
	Fork(main1);
	/* Fork(main2);*/
 } 