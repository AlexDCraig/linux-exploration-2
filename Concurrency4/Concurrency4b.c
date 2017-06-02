/* compile the program like 'gcc Concurrency4b.c -lpthread'*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define chairs 8 //number of chairs for customers to wait in

void *customer();
void *barberShop();
void *waiting_Room();
void checkQueue();
void get_hair_cut(void);
void cut_hair(void);

pthread_mutex_t queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sleepa = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barberSleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberAwake = PTHREAD_COND_INITIALIZER;

int curr=0, sleeping =0;

int main(int argc, char *argv[])
{
		//seed random function
		int seed = time(NULL);
		srand(seed);

		pthread_t barber_thread,customer_thread,timer_thread;
		pthread_attr_t barberAttr, timerAttr;
		pthread_attr_t customerAttr;

		//define barber, and customer default attributes
		pthread_attr_init(&timerAttr);
		pthread_attr_init(&barberAttr);
		pthread_attr_init(&customerAttr);

		printf("\n");

		//create customer thread
		pthread_create(&customer_thread,&customerAttr,customer,NULL);

		//create barber thread
		pthread_create(&barber_thread,&barberAttr,barberShop,NULL);

		pthread_join(barber_thread,NULL);
		pthread_join(customer_thread,NULL);

		return 0;
}

void *customer()
{
		int i=0;
		fflush(stdout);

		pthread_t customer[chairs+1];
		pthread_attr_t customerAttr[chairs+1];

		while(i <= chairs) /*if there are no free chairs, leave*/
		{
				i++;
				pthread_attr_init(&customerAttr[i]);

				sleep(1);
				while((rand()%2) != 1)
				{
						sleep(2);
				}
				pthread_create(&customer[i],&customerAttr[i],waiting_Room,NULL);
		}
		pthread_exit(0);// when shop is full don't wait
}

void *waiting_Room()
{
		//take seat
		pthread_mutex_lock(&queue);
		checkQueue();

		sleep(5);
		waiting_Room();
}

void *barberShop()
{
		while(1)
		{
				if(curr==0)
				{
                        printf("The Barber Shop is now empty and the barber is going to sleep.\n");
                        fflush(stdout);

                        pthread_mutex_lock(&sleepa);
                        sleeping=1;
                        pthread_cond_wait(&barberSleep,&sleepa);
                        sleeping=0;
                        pthread_mutex_unlock(&sleepa);

                        printf("Barber has woken up.\n");
                        fflush(stdout);
				}
				else
				{
						cut_hair();
						curr--;
						pthread_cond_signal(&barberAwake);
				}
		}
		pthread_exit(0);
}

void checkQueue()
{
		curr++;
		printf("Customer has entered Barber Shop. There are %d Customer(s) waiting in store.\n",curr);
		fflush(stdout);
		if(curr < chairs)
		{
				if(sleeping==1)
				{
						printf("Barber is sleeping. Customer wakes him up.\n");
						fflush(stdout);
						pthread_cond_signal(&barberSleep);
				}
				printf("Customer takes a seat.\n");
				fflush(stdout);
				pthread_mutex_unlock(&queue);
				pthread_mutex_lock(&wait);
				pthread_cond_wait(&barberAwake,&wait);
				pthread_mutex_unlock(&wait);
                get_hair_cut();
				return;
		}
		if(curr >= chairs)
		{
				printf("No more chairs in Waiting Room. Customer has left.\n");
				fflush(stdout);
				curr--;
				pthread_mutex_unlock(&queue);
				return;
		}
}
void cut_hair(void)
{
    printf("\tBarber is cutting the Customers hair now\n");
    fflush(stdout);

    //sleep for same time as get_hair_cut
    sleep(5);
    printf("\tCustomer finished getting their hair cut\n");
    fflush(stdout);
}

void get_hair_cut(void)
{
    printf("\tCustomer is getting a hair cut now\n");
    fflush(stdout);

    //sleep for same time as cut_hair
    sleep(5);
    printf("\tBarber finished getting haircut\n");
    fflush(stdout);
}
