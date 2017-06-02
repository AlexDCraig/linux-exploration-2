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
	printf("Testing Best Fit slob file.\n");
	time_t t;
	srand((unsigned)time(&t));
	float frag;

	int k;

	FILE* firstFitFile;
	firstFitFile = fopen("BestFitResults.txt", "w+");
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

		float unused = slob_free;
		float smallerUnused = unused/4;

		float used = slob_used;
		float smallerUsed = used/4;

		float smallerSum = smallerUnused + smallerUsed;
	
		fprintf(firstFitFile, "%s %d %s", "LOOP ", round, ":\n"); 
		fprintf(firstFitFile, "%s %f %s", "UNUSED PAGES: ", smallerUnused, " \n");
		fprintf(firstFitFile, "%s %f %s", "USED PAGES: ", smallerUsed, " \n");
		fprintf(firstFitFile, "%s %f %s", "TOTAL NUMBER OF PAGES: ", smallerSum, " \n");

		float fragmentation = ((smallerUnused) / (smallerSum)) * 100;

		// Fragmentation suffered:
		// ((free memory) / (free memory + used memory)) * 100
		fprintf(firstFitFile, "%s %f %s", "PERCENT OF ALLOCATED SPACE THAT IS UNUSED (INTERNAL FRAGMENTATION): ",  fragmentation, " \n\n");
		
		round++;
	}

	fclose(firstFitFile);

	return 0;
}
