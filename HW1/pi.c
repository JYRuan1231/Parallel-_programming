#include <stdio.h>   
#include <pthread.h>
#include <stdlib.h>
unsigned long long number_of_tosses_to_thread;

void* pi(void* arg) {	
	unsigned long long *result = (unsigned long long *) arg;
	double distance_squared, x, y;	
	int toss,count = 0;
	unsigned seed = rand();
	
    for (toss = 0; toss < number_of_tosses_to_thread; toss++) {
		
        x =  (double)rand_r(&seed)/(RAND_MAX + 1.0);
		y =  (double)rand_r(&seed)/(RAND_MAX + 1.0);
		 
        distance_squared = x*x + y*y;
		
        if (distance_squared <= 1)
            count++;
    }
	*result = count;
	return NULL;
}

int main(int argc, char **argv)
{
	int count_cpu;	
    double pi_estimate;
    unsigned long long  number_of_cpu, number_of_tosses, toss,total_number_in_circle=0;
    if ( argc < 2) {
        exit(-1);
    }	
	
    number_of_cpu = atoi(argv[1]);
    number_of_tosses = atoi(argv[2]);
	
    if (( number_of_cpu < 1) || ( number_of_tosses < 0)) {
        exit(-1);
    }
	
	pthread_t threads[number_of_cpu];
	unsigned long long number_in_circle[number_of_cpu];	
	srand(time(NULL));			
	number_of_tosses_to_thread = number_of_tosses/number_of_cpu;
	
		
	
	for(count_cpu = 0;count_cpu<number_of_cpu;count_cpu++){
		int count_affinity = count_cpu+1;
		number_in_circle[count_cpu]=0;
		pthread_create(&threads[count_cpu], NULL, &pi, (void *) &number_in_circle[count_cpu]);
	}
	for(count_cpu = 0;count_cpu<number_of_cpu;count_cpu++){
		pthread_join(threads[count_cpu], NULL);
		total_number_in_circle += number_in_circle[count_cpu];
	}
	

		
    pi_estimate = 4*total_number_in_circle/((double) number_of_tosses);
    printf("%lf", pi_estimate);
    return 0;
}
