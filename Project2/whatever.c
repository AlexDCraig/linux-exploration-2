/* C Implementation of the Dining Philosophers Problem by Alex Hoffer 
 * The philosophers' names courtesy of Marcel Proust's "Remembrance of Things Past".
 * 
 * Algorithm chosen to implement as solution: Dijkstra's Resource Hierarchy
 * The idea is that we assign partial order to our forks and that we must
 * request these forks in order. All philosophers request their lowest 
 * numbered fork except for the fifth philosopher that uses the highest 
 * and lowest numbered forks, leaving this fifth philosopher
 * susceptible to not being able to eat as often as the others. 
*/

#include <stdio.h> // for randomization
#include <pthread.h> // for threads, locking, etc.
#include <stdbool.h> // for bool

// ***
// Shared Resources Between Philosopher Threads
// ***

// Correspond to the five forks around the table. True if the fork is available, false if not.
// A shared resource that acts as a semaphore.
bool forks[5] = {true, true, true, true, true};

// A mutex provided for each philosopher. These lock global resources so that the relevant philosopher can safely check and modify the forks[] semaphore.
pthread_mutex_t forkLocks[5];

// What will hold our created pthread IDs that correspond to each philosopher
pthread_t philosopherThreadID[5];

// ***
// Our data structure that corresponds to the philosophers who will be executing threads
// ***

struct philosopher
{
	char* name;
	int index;
};

struct philosopher p0 = { "Marcel Proust", 0 };
struct philosopher p1 = { "Mme Verdurin", 1 };
struct philosopher p2 = { "Prince de Guermantes", 2 };
struct philosopher p3 = { "Elstir", 3 };
struct philosopher p4 = { "Baron de Charlus", 4 };

struct philosopher philosophers[5];

// *** 
// Helper functions for our void function that is called for each thread.
// ***

// Check the semaphore for whether a fork is unoccupied
bool isForkAvailable(int position)
{
	return forks[position];
}

// Find the correct index of the right fork.
int getRightForkPosition(int position)
{
	// We must wrap around the array using a modulus because the 4th philosopher's fork to his right is the 0th philosopher's fork to his left
	return (position + 1) % 5;
}

// eat a random time between 2-9 secs
void eat(char* name, int leftForkPos, int rightForkPos)
{
	int random = (rand() % 8) + 2;
	int randomTime = random * 1000000;
	printf("Philosopher: %s. Seat position: %d. Status: EATING for %d seconds. Forks: HAS Left fork, position: %d. HAS Right fork, position: %d.\n", name, leftForkPos, random, leftForkPos, rightForkPos);
	usleep(randomTime);
}

// think a random time between 1-20 secs
void think(char* name, int pos)
{
	int random = (rand() % 20) + 1;
	int randomTime = random * 1000000; // 1000000 microseconds is 1 second, usleep takes microseconds as input
	printf("Philosopher: %s. Seat position: %d. Status: THINKING for %d seconds. Forks: HAS no forks.\n", name, pos, random);
	usleep(randomTime);
}

// ***
// The function we will call from pthread_create. We must 

void* lifeAsAPhilosopher(void* arg)
{
	struct philosopher* phil = (struct philosopher*)arg;
	char* philName = phil->name;
	int philPosition = phil->index;
	
	while(1)
	{
		think(philName, philPosition);

		printf("Philosopher: %s. Seat position: %d. Status: HUNGRY. Forks: Checking left fork, position %d.\n", philName, philPosition, philPosition);
		
		// Tentatively lock the philosopher's left fork in preparation for a feast.
		// This'll fail if it's already locked.
		pthread_mutex_lock(&forkLocks[philPosition]); 
		
		// Check the left fork using the shared bool resource that is locked from the statement above.
		if (isForkAvailable(philPosition) == true) 
		{
			printf("Philosopher: %s. Seat position: %d. Status: HUNGRY. Forks: HAS left fork, position %d.\n", philName, philPosition, philPosition);
			
			// That fork is now unavailable. Alert the semaphore.
			forks[philPosition] = false;

			// Now that the semaphore is alerted, we have no need for the lock.
			pthread_mutex_unlock(&forkLocks[philPosition]);

			// Handle the right fork.
			// Attempt to lock the global resources in order to modify them
			// Check the semaphore to see if his right fork is free.
			// If it is, unlock the global resources AFTER notifying the semaphore the right fork is no longer free.
			// Then, lock both forks, change the global resources, then unlock the forks again.
			int rightForkPos = getRightForkPosition(philPosition);

			pthread_mutex_lock(&forkLocks[rightForkPos]);

			printf("Philosopher: %s. Seat position: %d. Status: HUNGRY. Forks: HAS left fork, position %d, checking on right fork, position: %d.\n", philName, philPosition, philPosition, rightForkPos);
			
			// his right fork is free
			if (isForkAvailable(rightForkPos) == true) 
			{
				forks[rightForkPos] = false;
				
				pthread_mutex_unlock(&forkLocks[rightForkPos]);

				eat(philName, philPosition, rightForkPos);
		
				// Lock the forks
				pthread_mutex_lock(&forkLocks[philPosition]);
				pthread_mutex_lock(&forkLocks[rightForkPos]);
	
				// Adjust the semaphore: now the forks are available
				forks[philPosition] = true;
				forks[rightForkPos] = true;

				// Unlock the forks now that the global resource is adjusted
				pthread_mutex_unlock(&forkLocks[philPosition]);
				pthread_mutex_unlock(&forkLocks[rightForkPos]);	
			}

			// The left fork is free, but the right fork is not. So, give up: set down the left fork, adjust the semaphore, and try again in a bit.
			else			
			{
				printf("Philosopher: %s. Seat position: %d. Status: HUNGRY. Forks: Has left fork, position %d. Can't have right fork, position %d. GIVING UP LEFT FORK.\n", philName, philPosition, philPosition, rightForkPos);
				// unlock his right fork
				pthread_mutex_unlock(&forkLocks[rightForkPos]);

				// Remember: we adjusted the semaphore of the left fork already. Lock the fork, adjust the semaphore again, and unlock it.
				pthread_mutex_lock(&forkLocks[philPosition]);
				forks[philPosition] = true;
				pthread_mutex_unlock(&forkLocks[philPosition]);
			}
		}

		// The left fork is not available. We tried to lock the correct fork, so unlock it. We don't have to adjust the semaphore
		// because we've done nothing to it.
		else
		{
			printf("Philosopher: %s. Seat position: %d. Status: HUNGRY. Forks: Cannot have left fork, position %d, so won't check on his right fork, position %d.\n", philName, philPosition, philPosition, getRightForkPosition(philPosition));
			pthread_mutex_unlock(&forkLocks[philPosition]);
		}		 

		// Nifty function alerts the CPU to relinquish processing time from this thread. Tells the scheduler to place 
		// this thread into the back of the run queue.		
		pthread_yield();
	}
}

void assignValuesToPhilosophers()
{	
	philosophers[0] = p0;
	philosophers[1] = p1;
	philosophers[2] = p2;
	philosophers[3] = p3;
	philosophers[4] = p4;
}

int main()
{
	srand(time(NULL)); // initialize our randomizer
	assignValuesToPhilosophers();

	int k = 0;
	
	// Create a thread for each philosopher, start each thread from lifeAsAPhilosopher.
	for (k; k < 5; k++)
		pthread_create(&philosopherThreadID[k], NULL, lifeAsAPhilosopher, &philosophers[k]);			

	int r = 0;
	
	// Wait for each thread to finish
	for (r; r < 5; r++)
		pthread_join(philosopherThreadID[r], NULL);

	return 0;
}
