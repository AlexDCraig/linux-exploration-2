#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/* Synchronization variables */

// Our three kinds of threads.
pthread_t searcherThread[3]; // these examine the list, can execute concurrenctly with each other
pthread_t deleterThread[3]; // these remove items from somewhere in the list, only 1 of these can access the list at once, and must lock the list when they do
pthread_t inserterThread[3]; // these add new items to the end of the list, they have to lock the list when they're doing it, 1 insert can be running in parallel with any # of searches

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
    else if(c<count())
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

/* Methods for the threads to load when they are created. */

// Algo: 1) Come up with random #
// 2) Lock the semaphore (decrement the sem) using sem_wait(&deleterSearcherSem), this blocks is the semaphore is at value 0, meaning it can't be decremented further
// 3) Look for the number in the list
// 4) Unlock the semaphore (increment the sem) using sem_post(&deleterSearcherSem), this tells any thread that was blocked to come along and lock the sem
// 5) sleep for a bit
void* searcher()
{
		
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

}

int main()
{
	srand(time(NULL));

	pthread_mutex_init(&inserterMutex);
	pthread_mutex_init(&deleterMutex);

	sem_init(&deleterInserterSem);
	sem_init(&deleterSearcherSem);

	int k = 0;
	for (k; k < 3; k++)
	{
		pthread_create(&searcherThread[k], NULL, searcher);
		pthread_create(&deleterThread[k], NULL, deleter);
		pthread_create(&inserterThread[k], NULL, inserter);
	}

	k = 0;
	for (k; k < 3; k++)
	{
		pthread_join(searcherThread[k], NULL);
		pthread_join(deleterThread[k], NULL);
		pthread_join(inserterThread[k], NULL);
	}

	return 0;
}
