#include <stdio.h>
#include <pthreads.h>
using namespace std;

void* lifeAsAPhilosopher(void*);
void createThreads();

class Fork
{
public:
	bool isAvailable;

	Fork()
	{
		isAvailable = true;
	}
};

class Philosopher
{
public:
	string name;
	pthread_t threadID;
	int position;

	Philosopher()
	{
		name = "";
		position = 0;
	}

	Philosopher(string name, int position)
	{
		this->name = name;
		this->position = position;
	}
};

class Table
{
public:
	Philosopher philosophers[5];
	Fork forks[5];
	
	Table()
	{
		Philosopher p0("Marcel Proust", 0);
		Philosopher p1("Victor Hugo", 1);
		Philosopher p2("Gustave Flaubert", 2);
		Philosopher p3("Jean-Jacques Rousseau", 3);
		Philosopher p4("Michel Foucault", 4);

		philosophers[0] = p0;
		philosophers[1] = p1;
		philosophers[2] = p2;
		philosophers[3] = p3;
		philosophers[4] = p4;

		Fork f0, f1, f2, f3, f4;

		forks[0] = f0;
		forks[1] = f1;
		forks[2] = f2;
		forks[3] = f3;
		forks[4] = f4;
	}
};

void createThreads(Table* table)
{
	for (int i = 0; i < 5; i++)
		pthread_create(table->&philosophers[i].threadID, 0, lifeAsAPhilosopher, &table);

	joinThreads(table);
}

void joinThreads(Table* table)
{
	for (int i = 0; i < 5; i++)
		pthread_join(table->&philosophers[i].threadID, NULL);
}

void* lifeAsAPhilosopher(void* voidTable)
{
	Table* table = *(Table)voidTable;
	


