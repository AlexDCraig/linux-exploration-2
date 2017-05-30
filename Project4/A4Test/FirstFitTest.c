// for printing
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Link in the system calls from the syscall_32.tbl
// system call 353 refers to UNOCCUPIED MEMORY
// system call 354 refers to OCCUPIED MEMORY
#define slob_used syscall(354)
#define slob_free syscall(353)

void fill(int* data)
{
	int i;
	for (i = 0; i < 500; i++)
	{
		int random = rand() % 1000;
		data[i] = random;
	}
}

// Our test driver.
int main()
{
	printf("Testing First Fit slob file.\n");
	time_t t;
	srand((unsigned)time(&t));
	float frag;

	int k;

	FILE* firstFitFile;
	firstFitFile = fopen("FirstFitResults.txt", "w+");
	int round = 1;

	for (k = 0; k < 3; k++)
	{
		int dummyData1[500];
		int dummyData2[500];
		int dummyData3[500];
		int dummyData4[500];
		int dummyData5[500];

		fill(dummyData1);
		fill(dummyData2);
		fill(dummyData3);
		fill(dummyData4);
		fill(dummyData5);
	
		fprintf(firstFitFile, "%s %d %s", "LOOP ", round, ":\n"); 
		fprintf(firstFitFile, "%s %lu %s", "UNUSED PAGES: ", slob_free, " \n");
		fprintf(firstFitFile, "%s %lu %s", "USED PAGES: ", slob_used, " \n");
		fprintf(firstFitFile, "%s %f %s", "TOTAL NUMBER OF PAGES: ", ((float)slob_free + (float)slob_used), " \n");

		// Fragmentation suffered:
		// ((free memory) / (free memory + used memory)) * 100
		fprintf(firstFitFile, "%s %f %s", "PERCENT OF ALLOCATED SPACE THAT IS UNUSED (INTERNAL FRAGMENTATION): ",  ((((float)slob_free) / ((float)slob_free + (float)slob_used)) * 100), " \n\n");
		
		round++;
	}

	fclose(firstFitFile);

	return 0;
}
