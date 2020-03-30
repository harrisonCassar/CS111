#include <stdio.h>
#include <time.h>

int main()
{
	time_t rawtime;
	time_t currenttime;
	struct tm *info;
	time( &rawtime );

	for (;;)
	{
		time(&currenttime);
		info = localtime(&currenttime);

		printf("Difference: %g",difftime(currenttime,rawtime));
		printf("Current local time and date: %s", asctime(info));
	}

	return 0;
}