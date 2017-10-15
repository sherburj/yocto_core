#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <semaphore.h>
#include <stdio.h>

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

#define BUFFER_SIZE 32
#define TRUE		1
#define RAND_DIVISOR 100000000

typedef int buffer_item;
buffer_item buffer[BUFFER_SIZE][2];
int counter, consumer_count;

pthread_mutex_t mutex;
sem_t full, empty;
pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

///////// TWISTER FUNCTIONS ////////////

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void) 
{ 
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 
/* These real versions are due to Isaku Wada, 2002/01/09 added */
int getRand(int i, int j){
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	char vendor[13];
	eax = 0x01;

	
	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );

	
	int num = 0;
	unsigned int rnd = 0;
	if(ecx & 0x40000000){
		//Use rdrand
		__asm__ __volatile__("rdrand %0" : "=r" (rnd));
		num = (i + (rnd % (j-i)));
	}
	else{
		//USE mt19937
		num = (i + (genrand_int32() % ( j - i )));
		//printf("num: %i\n", num);
	}		
	return num;
}

///////// PTHREAD FUNCTIONS ///////////

void initializeData() {
	//mutex lock
	pthread_mutex_init(&mutex, NULL);
	
	//full semaphore set to 0
	sem_init(&full, 0, 0);
	
	//empty semaphore set to BUFFER_SIZE
	sem_init(&empty, 0, BUFFER_SIZE);

	//default attributes
	pthread_attr_init(&attr);

	//init buffer
	counter = 0;
	consumer_count = 0;
}

//Producer Thread
void *producer(void *tid){
	buffer_item item[2];
	int i;
	
	while(TRUE) {
		// SLEEP FOR 3-7 SECONDS
		sleep(getRand(2,9));
		
		// GENERATE RANDOM NUMBER
		i = getRand(3, 7);
		item[0] = getRand(1, 100);
		item[1] = i;
				
		//check for empty lock
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		
		if(insert_item(item)) { //returns -1 if function errors out
			fprintf(stderr, "Producer Errored\n");
		}
		else {
			printf("Producer: %i with time %i \n", item[0], item[1]);
		}
		
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}

}

//Consumer Thread
void *consumer(void *tid){
	buffer_item item[2];
	
	while(TRUE) {
		
		//check for empty lock
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		if(remove_item(item)) { //returns -1 if function errors out
			//fprintf(stderr, "No Item for Consumer\n");
		}
		else {
			printf("Consumer: %i with consume time %i\n", item[0], item[1]);
			
		}
		
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
		
		sleep(item[1]);
	}

}

// Add an item to the buffer
int insert_item(buffer_item item[]) {
	//printf("\n%i\n\n", item);
	if(counter < BUFFER_SIZE) {
		buffer[counter][0] = item[0];
		buffer[counter][1] = item[1];
		counter++;

		return 0;
   }
   else { //breaks if buffer full
		return -1;
   }
}

// Remove an item from the buffer
int remove_item(buffer_item item[]) {
	if(counter > 0) {
		item[0] = buffer[(counter-1)][0];
		item[1] = buffer[(counter-1)][1];
		int i, j;
		for (i=0; i<256; i++) {
			if (!(buffer[i])) {
				for (j=i; j<255; j++) {
					buffer[j][0] = buffer[j+1][0];
					buffer[j][1] = buffer[j+1][1];
				}
			}
		}
		counter--;
		return 0;
   }
   else { //breaks if buffer empty
		return -1;
   }
}

////////////// MAIN ///////////////////

int main(int argc, char *argv[]) {
	
	initializeData();
	unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
	unsigned long length = 4;
	init_by_array(init, length);
	pthread_create(&tid, &attr, producer, NULL); //put a for loop around this for multiple
	pthread_create(&tid, &attr, consumer, NULL); //put a for loop around this for multiple	

	sleep(100);
	printf("%i items left in the buffer\n", counter);
	printf("Program Exit\n");

	return 0;
}
