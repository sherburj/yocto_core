#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#include<stdlib.h>
#include<time.h>
 
#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (ph_num+4)%N
#define RIGHT (ph_num+1)%N
 
time_t t; 
 
sem_t mutex;
sem_t S[N];
 
void * philospher(void *num);
void take_fork(int);
void put_fork(int);
void test(int);
 
int state[N];

char aristotle[10]={"Aristotle\0"};
char plato[6]={"Plato\0"};
char confucius[10]={"Confucius\0"};
char jaden_smith[12]={"Jaden Smith\0"};
char socrates[9]={"Socrates\0"};

char *phil_name[N]={aristotle, plato, confucius, jaden_smith, socrates};
int phil_num[N]={0,1,2,3,4};
 
int main()
{
	srand((unsigned) time(&t)); 
    int i;
    pthread_t thread_id[N];
    sem_init(&mutex,0,1);
    for(i=0;i<N;i++)
        sem_init(&S[i],0,0);
    for(i=0;i<N;i++)
    {
        pthread_create(&thread_id[i],NULL,philospher,&phil_num[i]);
        printf("%s is thinking\n",phil_name[i]);
    }
    for(i=0;i<N;i++)
        pthread_join(thread_id[i],NULL);
}
 
void *philospher(void *num)
{
    while(1)
    {
        int *i = num;
        sleep((rand()%19)+1);
        take_fork(*i);
        sleep(0);
        put_fork(*i);
    }
}
 
void take_fork(int ph_num)
{
    sem_wait(&mutex);
    state[ph_num] = HUNGRY;
    printf("%s is Hungry\n", phil_name[ph_num]);
    test(ph_num);
    sem_post(&mutex);
    sem_wait(&S[ph_num]);
    sleep((rand()%7)+2);
}
 
void test(int ph_num)
{
    if (state[ph_num] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
    {
        state[ph_num] = EATING;
        sleep(2);
        printf("%s picks up fork %d and %d\n",phil_name[ph_num],LEFT+1,ph_num+1);
        printf("%s is Eating\n",phil_name[ph_num]);
        sem_post(&S[ph_num]);
    }
}
 
void put_fork(int ph_num)
{
    sem_wait(&mutex);
    state[ph_num] = THINKING;
    printf("%s sets down fork %d and %d down\n",phil_name[ph_num],LEFT+1,ph_num+1);
    printf("%s is thinking\n",phil_name[ph_num]);
    test(LEFT);
    test(RIGHT);
    sem_post(&mutex);
}