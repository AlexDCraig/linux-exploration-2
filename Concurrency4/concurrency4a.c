// Alex Hoffer
// Concurrency 4a
// Description: A shareable resource where: 
// a) a maximum of two processes can use the resource simultaneously
// b) once three processes start using the resource, all of the three
// processes must stop using the resource before any process can use the resource

#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

struct sharedResource
{
	double data;
	sem_t resourceSemaphore; // So that we can maintain exclusive access to the shared resource and so we can alert waiting threads when it is available.
};

struct sharedResource s1;
int currentThreadsAccessingResource;

// What each thread will execute. An infinite loop. 
void* accessResource()
{
	pthread_t ID = pthread_self();	

	// Toggle. 0 indicates there's no need to wait, 1 indicates needs to wait.
	int toggle = 1;

	if (currentThreadsAccessingResource > 0)
		toggle = 0; 

	while (toggle == 1)
	{
		printf("Thread %lu Waiting...\n", (unsigned long)ID);
		sleep(rand() % 10);

		if (currentThreadsAccessingResource == 3)
		{
			printf("Thread %lu Done waiting...\n", (unsigned long) ID);
			toggle = 0;
		}
	}

	sem_wait(&s1.resourceSemaphore);
	currentThreadsAccessingResource -= 1;

	printf("Thread %lu Working on the resource.\n", (unsigned long) ID);
	sleep(rand() % 10);
	printf("Thread %lu done working on the resource.\n", (unsigned long) ID);

	sem_post(&s1.resourceSemaphore);
	currentThreadsAccessingResource += 1;
}

int main()
{
	srand(time(NULL));

	// Create the three threads.
	pthread_t thread1,
	thread2,
	thread3,
	thread4,
	thread5,
	thread6;

	pthread_t threads[6] = { thread1, thread2, thread3, thread4, thread5, thread6 };

	// Initialize the shared resource.
	// sem_init(address of semaphore, 0 to indicate that the semaphore
	// is shared between threads, 3 to specify the current value of the
	// semaphore to be 3 indicating there are three slots for use at once)
	s1.data = -1;
	sem_init(&s1.resourceSemaphore, 0, 3);
	currentThreadsAccessingResource = 3;

	// Create the threads.
	// pthread_create(address of thread, thread attributes (NULL if default), address of void method to start thread from, address of void argument to pass to the void method). We don't need to pass any argument, so NULL, because we have a global resource that all threads have access to.		
	int k;

	printf("Creating 6 threads.\n");

	for (k = 0; k < 6; k++)
		pthread_create(&threads[k], NULL, accessResource, NULL);  	

	while (1) { }

	return 0;
}
