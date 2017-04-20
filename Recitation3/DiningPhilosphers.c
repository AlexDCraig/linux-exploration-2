#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int think()
{
	
}

int eat()
{
	int r;
	r = (rand() % (9 + 1 - 2)) + 2;
	return r;

}

void dine()
{
	while(1)
	{
		think();
		get_forks();
		eat();
		put_forks();
	}
}

int main()
{
	srand(time(NULL));


	dine();
	return 0;
}
