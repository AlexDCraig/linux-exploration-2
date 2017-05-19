#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/* Synchronization variables */

// Our three kinds of threads.
pthread_t searcherThread[2]; // these examine the list, can execute concurrenctly with each other
pthread_t deleterThread[2]; // these remove items from somewhere in the list, only 1 of these can access the list at once, and must lock the list when they do
pthread_t inserterThread[2]; // these add new items to the end of the list, they have to lock the list when they're doing it, 1 insert can be running in parallel with any # of searches

// Our two kinds of Mutual Exclusion semaphores (mutexes). Don't need a third because the searcherThread requires no lock.
pthread_mutex_t inserterMutex;
pthread_mutex_t deleterMutex;

// Our two kinds of semaphores that will be used to notify other threads when something needs to happen.
sem_t deleterInserterSem;
sem_t deleterSearcherSem; // We need these to notify other kinds of threads when delete isn't meddling with data. 


/* Linked list methods adapted from http://www.cprogramming.com/snippets/source-code/singly-linked-list-insert-remove-add-count */

struct node
{
    int data;
    struct node *next;
}*head;
  
void addToEnd(int num)
{
    struct node *temp,*right;
    temp= (struct node *)malloc(sizeof(struct node));
    temp->data=num;
    right=(struct node *)head;
    while(right->next != NULL)
    right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}
 
 
 
void addToStart(int num)
{
    struct node *temp;
    temp=(struct node *)malloc(sizeof(struct node));
    temp->data=num;
    if (head== NULL)
    {
    head=temp;
    head->next=NULL;
    }
    else
    {
    temp->next=head;
    head=temp;
    }
}

void addAfterLocation(int num, int loc)
{
    int i;
    struct node *temp,*left,*right;
    right=head;
    for(i=1;i<loc;i++)
    {
    left=right;
    right=right->next;
    }
    temp=(struct node *)malloc(sizeof(struct node));
    temp->data=num;
    left->next=temp;
    left=temp;
    left->next=right;
    return;
}
 
void insert(int num)
{
    int c=0;
    struct node *temp;
    temp=head;
    if(temp==NULL)
    {
    addToStart(num);
    }
    else
    {
    while(temp!=NULL)
    {
        if(temp->data<num)
        c++;
        temp=temp->next;
    }
    if(c==0)
        addToStart(num);
    else if(c<sizeOfList())
        addAfterLocation(num,++c);
    else
        addToEnd(num);
    }
}
 
 
 
int deleteFromList(int num)
{
    struct node *temp, *prev;
    temp=head;
    while(temp!=NULL)
    {
    if(temp->data==num)
    {
        if(temp==head)
        {
        head=temp->next;
        free(temp);
        return 1;
        }
        else
        {
        prev->next=temp->next;
        free(temp);
        return 1;
        }
    }
    else
    {
        prev=temp;
        temp= temp->next;
    }
    }
    return 0;
}

void initializeList();

void deleteFromLocation(int loc)
{

	pthread_t ID = pthread_self();

 	if (head == NULL)
	{
		printf("DELETER, ID %lu: Cannot delete index, list is empty.\n", (unsigned long) ID);
		return;
	}

	struct node* temp = head;
      
	if (loc == 0)
	{
		printf("DELETER, ID %lu: Deleting head, which is value %d\n", (unsigned long) ID, head->data);
		head = temp->next;
		free(temp);
		initializeList();
		return;
	}

	int i = 0;

	for (i; temp != NULL && i<loc-1; i++)
		temp = temp->next;

	if (temp == NULL || temp->next == NULL)
	{
		printf("DELETER, ID %lu: Invalid index\n", (unsigned long) ID);
		return;
	}

	struct node* next = temp->next->next;
	int data = temp->next->data;

	free(temp->next);

	temp->next = next;	

	printf("DELETER, ID %lu: Deleted index %d which held value %d\n", (unsigned long) ID, loc, data);	 

	return;
}
 
 
void printList(struct node *r)
{
    r=head;
    if(r==NULL)
    {
    return;
    }
    while(r!=NULL)
    {
    printf("%d ",r->data);
    r=r->next;
    }
    printf("\n");
}
 
 
int sizeOfList()
{
    struct node *n;
    int c=0;
    n=head;
    while(n!=NULL)
    {
    n=n->next;
    c++;
    }
    return c;
}

void findValue(int val)
{
	pthread_t ID = pthread_self();
	int found = 0;


    struct node* r=head;
    if(r==NULL)
    {
    printf("SEARCHER, ID %lu: The list is empty. Cannot find %d.\n", (unsigned long) ID, val);
    return;
    }
    while(r!=NULL)
    {
	if (r->data == val)
	{
		printf("SEARCHER, ID %lu: Found value %d \n", (unsigned long) ID,r->data);
		found = 1;
		break;
	}

    	r=r->next;
    }
 	if (found == 0)
  	printf("SEARCHER, ID %lu: Value %d is not in the list\n", (unsigned long) ID, val);

}

/* Methods for the threads to load when they are created. */

// Algo: 1) Come up with random #
// 2) Lock the semaphore (decrement the sem) using sem_wait(&deleterSearcherSem), this blocks is the semaphore is at value 0, meaning it can't be decremented further
// 3) Look for the number in the list
// 4) Unlock the semaphore (increment the sem) using sem_post(&deleterSearcherSem), this tells any thread that was blocked to come along and lock the sem
// 5) sleep for a bit
void* searcher()
{
	while(1)
	{
	int value;
	sem_getvalue(&deleterSearcherSem, &value);

	pthread_t ID = pthread_self();
	
	if (value == 0)
		printf("SEARCHER, ID %lu: Is blocked. Will wait for semaphore to alert it.\n", (unsigned long) ID);
	
	sem_wait(&deleterSearcherSem);
	
	int random = rand() % 6;

	findValue(random);		

	sem_post(&deleterSearcherSem);	

	sleep(3);
	}
}

// Algo: 1) Wait until both semaphores available to lock to make sure inserter or search methods aren't using the resources at the same time: sem_wait(&deleterInserterSem), sem_wait(&deleterSearcherSem)
// 2) Lock the deleterMutex: pthread_mutex_lock(&deleterMutex) to make sure nothing else will touch the list
// 3) Pick a random index and delete it
// 4) Unlock the deleterMutex: pthread_mutex_unlock(&deleterMutex) so that other threads can access resources
// 5) Unlock the other semaphores, telling them they can use the thread:
// sem_post(&deleterInsertSem), sem_post(&deleterSearcherSem)
// 6) sleep for a bit
void* deleter()
{
	while(1)
	{
	pthread_t ID = pthread_self();
	

	int value;
	sem_getvalue(&deleterSearcherSem, &value);

	if (value == 0)
		printf("DELETER, ID %lu: Is blocked. Will wait for semaphore to alert it.\n", (unsigned long) ID);
	
	sem_wait(&deleterSearcherSem);

	sem_getvalue(&deleterInserterSem, &value);

	if (value == 0)
		printf("DELETER, ID %lu: Is blocked. Will wait for semaphore to alert it.\n", (unsigned long) ID);
	
	sem_getvalue(&deleterInserterSem, &value);
	
	sem_wait(&deleterInserterSem);

	pthread_mutex_lock(&deleterMutex);

	int randomIndex = rand() % sizeOfList(head);
	deleteFromLocation(randomIndex);

	pthread_mutex_unlock(&deleterMutex);
	sem_post(&deleterSearcherSem);
	sem_post(&deleterInserterSem);

	sleep(6);
	}
}

// Algo: 1) Wait for the deleter semaphore to be available:
// sem_wait(&deleterInserterSem)
// 2) Lock the inserterMutex 
// pthread_mutex_lock(&inserterMutex)
// 3) Add random number to end of list
// 4) Unlock deleter semaphore, informing any waiting threads:
// sem_post(&deleterInserterSem)
// 5) Unlock inserterMutex
// pthread_mutex_unlock(&inserterMutex)
void* inserter()
{
	while(1)
	{
	pthread_t ID = pthread_self();
	
	int value;
	sem_getvalue(&deleterInserterSem, &value);

	if (value == 0)
		printf("INSERTER, ID %lu: Is blocked. Will wait for semaphore to alert it.\n", (unsigned long) ID);

	sem_wait(&deleterInserterSem);
	pthread_mutex_lock(&inserterMutex);

	int randomNumber = rand() % 6;
	addToEnd(randomNumber);
	printf("INSERTER, ID %lu: Added %d to end of list\n", (unsigned long) ID, randomNumber);	

	sem_post(&deleterInserterSem);
	pthread_mutex_unlock(&inserterMutex);
	sleep(3);
	}
}

void initializeList()
{
	head = malloc(sizeof(struct node));
	head->next = NULL;
	int newData = rand() % 5;
	head->data = newData;
}

int main()
{
	srand(time(NULL));

	initializeList();

	pthread_mutex_init(&inserterMutex, NULL);
	pthread_mutex_init(&deleterMutex, NULL);

	sem_init(&deleterInserterSem, 0, 1);
	sem_init(&deleterSearcherSem, 0, 1);

	int k = 0;
	for (k; k < 2; k++)
	{
		pthread_create(&searcherThread[k], NULL, searcher, NULL);
		pthread_create(&deleterThread[k], NULL, deleter, NULL);
		pthread_create(&inserterThread[k], NULL, inserter, NULL);
	}

	k = 0;
	for (k; k < 2; k++)
	{
		pthread_join(searcherThread[k], NULL);
		pthread_join(deleterThread[k], NULL);
		pthread_join(inserterThread[k], NULL);
	}

	return 0;
}
