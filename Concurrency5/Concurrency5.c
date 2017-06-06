#include<stdio.h>
#include<sys/types.h>
#include<semaphore.h>
#include<pthread.h>

int table_used = 1;
int table_items[2];
char *ingredients[] = {"tobacco", "paper", "matches"};
sem_t table;

void *agent()
{
   int item1 = 0;
   int item2 = 0;

   while(1)
   {
      sem_wait(&table);
      if(table_used==1)
      {
         item1 = rand() % 3;
         item2 = rand() % 3;

         if (item1!=item2)
         {
            table_items[0] = item1;
            table_items[1] = item2;

            printf("The agent has placed %s and %s on the table\n", ingredients[item1], ingredients[item2]);
            table_used = 0;
            sleep(2);
         }
       }
    sem_post(&table);
    }
}

void *smokers(int i)
{
   while(1)
    {
      sleep(1);
      sem_wait(&table);

      if (table_used==0)
      {
         if (table_items[0]!=i && table_items[1]!=i)
         {
            if(i==0)
            {
                printf("Smoker that had tobacco has picked up the items from the table(Smoker 1)\n");
                sem_post(&table);
                table_used = 1;
                sleep(3);
                printf("\tSmoker that had tobacco has finished smoking(Smoker 1)\n");
            }
            else if(i==1)
            {
                printf("Smoker that had paper has picked up the items from the table(Smoker 2)\n");
                sem_post(&table);
                table_used = 1;
                sleep(3);
                printf("\tSmoker that had paper has finished smoking(Smoker 2)\n");
            }
            else if(i==2)
            {
                printf("Smoker that had matches has picked up the items from the table(Smoker 3)\n");
                sem_post(&table);
                table_used = 1;
                sleep(3);
                printf("\tSmoker that had matches has finished smoking(Smoker 3)\n");
            }
          }
      }
      sem_post(&table);
   }
}

int main()
{
   pthread_t smoker1_thread, smoker2_thread, smoker3_thread, agent_thread;
   sem_init(&table, 0, 1);

   int firstSmoker = 0;//has tobacco
   int secondSmoker = 1;//has paper
   int thirdSmoker = 2;//has matches

   pthread_create(&agent_thread, NULL, agent, NULL);

   pthread_create(&smoker1_thread, NULL, smokers, firstSmoker);
   pthread_create(&smoker2_thread, NULL, smokers, secondSmoker);
   pthread_create(&smoker3_thread, NULL, smokers, thirdSmoker);

   pthread_join(agent_thread, NULL);
   pthread_join(smoker1_thread, NULL);
   pthread_join(smoker2_thread, NULL);
   pthread_join(smoker3_thread, NULL);
}
