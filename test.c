#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
Kipling Gillespie
CS485
11/21/2017
This program is run by mysh just for testing purposes. 
*/

int main(int argc, char** argv)
{
	//If no argument is given it just prints "Hello WOrld!, This is a test!\n
	// other wise if passed an integer it waits for argv[1] seconds before printing the value of argv[1].
	if(argc > 1)
	{
		sleep(atoi(argv[1]));
		fprintf(stdout, "%s\n", argv[1]);
	}
	else
		fprintf(stdout, "%s", "Hello World!, This is a test!\n");
	
	return 0;
}
