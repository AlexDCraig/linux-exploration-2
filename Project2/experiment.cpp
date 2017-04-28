#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class Fork
{
private:
	int forkID;
public:
	Fork()
	{
		forkID = -1;
	}

	Fork(const int id)
	{
		forkID = id;
	}

	int getForkID()
	{
		return forkID;
	}
};

class Philosopher
{
private:
	string name;
	int position;
public:
	Philosopher()
	{
		name = "";
		position = -1;
	}

	Philosopher(string name, int pos)
	{
		this->name = name;
		position = pos;
	}

	string getName()
	{
		return name;
	}

	int getPosition()
	{
		return position;
	}

	int getPhilToMyLeft()
	{
		return (position + 4) % 5;
	}

	int getPhilToMyRight()
	{
		return (position + 1) % 5;
	} 	
};

void* live (void*);
void get_fork (Philosopher);

class Table
{
public:
	Philosopher philosophers[5];
	Fork forks[5];
	sem_t mutex;
	sem_t mutexes[5];
	pthread_t threadIDs[5];
	int states[5]; // 0 for thinking, 1 for hungry, 2 for eating
	
	Table()
	{	
		Philosopher p0("Proust", 0);
		Philosopher p1("Hugo", 1);
		Philosopher p2("Balzac", 2);
		Philosopher p3("Flaubert", 3);
		Philosopher p4("Foucault", 4);

		philosophers[0] = p0;
		philosophers[1] = p1;
		philosophers[2] = p2;
		philosophers[3] = p3;
		philosophers[4] = p4;

		Fork f0(0);
		Fork f1(1);
		Fork f2(2);
		Fork f3(3);
		Fork f4(4);

		forks[0] = f0;
		forks[1] = f1;
		forks[2] = f2;
		forks[3] = f3;
		forks[4] = f4;

		for (int i = 0; i < 5; i++)
			states[i] = 0;

		// initialize the master lock, share the lock between threads
		sem_init(&mutex, 0, 1);

		// initialize the rest of the locks
		for (int k = 0; k < 5; k++)
			sem_init(&mutexes[k], 0, 0);
	
	}

	void put_forks(Philosopher* p1)
	{
		sem_wait(&mutex);
		states[p1->getPosition()] = 0;
		cout << p1->getName() << " has finished eating and is now thinking" << endl;
		canIEat(&philosophers[p1->getPhilToMyLeft()]);
		canIEat(&philosophers[p1->getPhilToMyRight()]);
		sem_post(&mutex);
	}

	static void* live (void* phil)
	{
		while (1)	
		{
			Philosopher* p1 = static_cast<Philosopher*>(phil);
			sleep(1);
			get_forks(p1);
			sleep(0);
			put_forks(p1);
		}

	}

	void canIEat(Philosopher* p1)
	{
		int indexOfLeft = p1->getPhilToMyLeft();
		int indexOfRight = p1->getPhilToMyRight();

		if (states[p1->getPosition()] == 1 && states[p1->getPhilToMyLeft()] != 2 && states[p1->getPhilToMyRight()] != 2)
		{
			states[p1->getPosition()] = 2;
			sleep(2);
			cout << p1->getName() << " takes the forks to the left and right and is now eating" << endl;
			sem_post(&mutexes[p1->getPosition()]);
		}
	}		 

	void get_forks (Philosopher* p1)
	{
		sem_wait(&mutex);
		states[p1->getPosition()] = 1;
		cout << p1->getName() << " is hungry" << endl;
		canIEat(p1);			
		sem_post(&mutex);
		sem_wait(&mutexes[p1->getPosition()]);
		sleep(1);
	}
		

	void playGame()
	{
		// Create a thread for each philosopher, execute the live function from the thread, pass the live function an argument consisting of the philosopher object
		for (int i = 0; i < 5; i++)
		{
			pthread_create(&threadIDs[i], NULL, live, &philosophers[i]);
			cout << philosophers[i].getName() << " is thinking..." << endl;
		}

		for (int k = 0; k < 5; k++)
			pthread_join(threadIDs[k], NULL);

	}
};

int main()
{
	Table t1;
	
	t1.playGame();

}


