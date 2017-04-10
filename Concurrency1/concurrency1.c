/* Alex Hoffer
 * Concurrency 1 - Producer Consumer Problem
 * April 2017
 * pthread resources: 
 * a) https://computing.llnl.gov/tutorials/pthreads/
 * b) Ben Brewster's OS 1 notes
 * c) Michael Kerrisk's Linux Programming Interface book
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

// *** Definition of structs, threadBuf is our global buffer to be shared, initialize mutex to protect our global buffer ***
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

static buffer threadBuf; // Both threads will have access to this data

// Initialize the mutex we will be using to lock access to our global data in order to synchronize threads
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	

// Condition variables let one thread inform other threads about changes in the state of a shared variable and allows other threads to BLOCK in order to wait for this notification
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

// Pthread IDs for identification
static pthread_t consumer; // to identify the consumer thread
static pthread_t producer; // " " "

// *** CHECK STATUS OF BUFFER ***
bool isBufFull()
{
	if (threadBuf.size == 32)
		return true;

	else
		return false;
}

bool isBufEmpty()
{
	if (threadBuf.size == 0)
		return true;

	else
		return false;
}

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
If buffer is full, block until consumer removes an item
*/

void* produceAnItem()
{
 	// Lock the buffer (mutexes)
 	int mutexInfo;
	mutexInfo = pthread_mutex_lock(&mutex);
	 
 	// Use pthread conditions, pthread_cond_wait to wait until 
 	// there's space in the buffer
 	if (isBufFull() == true)
		pthread_cond_wait(&condition, &mutex); // let this thread sleep until consumer takes an item
		
 	// Buffer is not full, generate random number between 3-7 secs then wait that amount of time
 	else
	{
		int randomWaitTime = producerVal();
		printf("PRODUCER THREAD: Sleeping %d seconds before producing", randomWaitTime);
		sleep(randomWaitTime);

 		// create random item
 		item randomItem;
		randomItem.randomNum = itemFirstVal();
		randomItem.waitTime = itemSecondVal();

		// add item to buffer
		threadBuf.items[size] = randomItem;

 		// increment buffer size
		threadBuf.size++;

 		// Signal to the consumer there's an item
 		pthread_cond_signal(&condition);
 	
		// Unlock the buffer
 		mutexInfo = pthread_mutex_unlock(&mutex);

		if (mutexInfo == 0)
		{
			fprintf(stderr, "Error unlocking mutex. Exiting...\n");
			exit(-1);
		}
	}			
}

/*** FUNCTIONS FOR CONSUMER THREAD ***
*/

void* consumeAnItem()
{
 	// Lock the buffer (mutexes)
 	int mutexInfo = pthread_mutex_lock(&mutex);
 
 // Use pthread conditions, pthread_cond_wait to wait until
 // the buffer is not empty
 // Access the index
 // print out first value
 // wait the second value
 // print second value
 // write a function that erases that index, adjusts the array/size element
 // Use pthread conditions, pthread_cond_signal to wake up producer
 	
	// Unlock the buffer
	mutexInfo = pthread_mutex_unlock(&mutex);

	if (mutexInfo == 0)
	{
		fprintf(stderr, "Error unlocking mutex. Exiting...\n");
		exit(-1);
	}	
}

int main()
{
	srand(time(NULL));

	threadBuf.size = 0; // initialize our buffer to empty
	
	// pthread_create: (ref to ID of thread, pointer to struct with option flags, pointer to function that will be start point of execution for thread, argument passed to the function of execution)
	// For our first thread execution, produce an item.
	pthread_create(&producer, NULL, produceAnItem, NULL);

	return 0;
}
