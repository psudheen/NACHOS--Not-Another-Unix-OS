#include <stdio.h>
#include <sys/time.h>

int main()
{
		struct timeval tval;  
	struct timezone tzone;  
	struct tm *tm;  
	int myTime;
	int i;
	gettimeofday(&tval, &tzone);  
	tm=localtime(&tval.tv_sec); 
	for(i=0;i<10;i++)
	{
	myTime=(unsigned int)(tval.tv_usec + tval.tv_sec*1000000);
	printf("%d\n",myTime);
	}
	return 1;
	/* return((unsigned int)(tval.tv_usec + tval.tv_sec*1000000));   */
}