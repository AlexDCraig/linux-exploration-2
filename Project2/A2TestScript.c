#include <stdlib.h>
#include <stdio.h>

int main()
{
	while(1)
	{
	printf("From test program: Print1\n");
	printf("From test program: Print2\n");
	printf("From test program: Print3\n");
	printf("From test program: Enter an integer\n");
	int rando1, rando2, rando3, rando4, rando5;
	scanf("%d", &rando1);
	printf("From test program: You wrote %d\n", rando1);
	printf("From TP: enter an integer\n");
	scanf("%d", &rando2);
	printf("From TP: you entered: %d\n", rando2);
	printf("From TP: enter an integer\n");
	scanf("%d", &rando3);
	printf("From TP: you entered: %d\n", rando3);
	printf("From TP: enter an integer\n");
	scanf("%d", &rando4);
	printf("From TP: you entered: %d\n", rando4);		
	}

	return 0;
}

