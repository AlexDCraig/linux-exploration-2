/* Alex Hoffer
 * Concurrency 1 - Producer Consumer Problem
 * April 2017
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mt19937ar.c"

#include <time.h>

typedef int bool;
#define true 1
#define false 0

// *** Definition of structs, threadBuf is our buffer to be shared ***
typedef struct item
{
	int randomNum; // Just a random number
	int waitTime; // Random waiting period between 2 and 9 
} item;

typedef struct buffer
{
	item items[32];
	int size; // how many are currently in the buffer
} buffer;

buffer threadBuf; // Both threads will have access to this data


// *** ASM STUFF: REGISTERS, RANDOM # GENERATION, ETC ***

// Set up registers for random number generation
unsigned int eax, ebx, ecx, edx;

// Source: http://codereview.stackexchange.com/questions/147656/checking-if-cpu-supports-rdrand
// To check if the processor supports rdrand, see if the 30th bit
// in the ecx register is set.
// If it is, return true. If not, return false.
// Call this function ONLY after preparing the registers
bool checkProcessor()
{
	if (ecx & 1<<30)
		return true; // rdrand can be supported
	else
		return false;
} 

// ** Throw these functions away when the ASM is understood **
int itemFirstVal() // between 0 and 10
{
	int r;
	r = (rand() % (10 + 1));
	return r;
}

int itemSecondVal() // between 2 and 9
{
	int r;
	r = (rand() % (9 + 1 - 2)) + 2;
	return r;
}

int producerVal() // between 3 and 7
{
	int r;
	r = (rand() % (7 + 1 - 3)) + 3;
	return r;
}

/*** FUNCTION FOR PRODUCER THREAD ***
a) Wait a random time between 3-7 seconds
b) Generate an item: 1) a random number 2) a random waiting period between 2-9
It's a function pointer in accordance with POSIX standards
*/

void* produceAnItem()
{
		
}

int main()
{
	srand(time(NULL));
	
	pthread_t consumer; // to identify the consumer thread
	pthread_t producer; // " " "
	
	// pthread_create: (ref to ID of thread, pointer to struct with option flags, pointer to function that will be start point of execution for thread, argument passed to the function of execution)
	// For our first thread execution, produce an item.
	pthread_create(&producer, NULL, produceAnItem, NULL);

	return 0;
}
