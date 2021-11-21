#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "buffer.h"

#define NUM_VALUES "0123456789"
#define NULL_CHAR '\0'
#define NUM_CYCLES 100000


void *producer();
void *consumer();

int main() {

	/* TEST WITHOUT PTHREAD
	init_buffer_421();
	printf("Initialized buffer\n");
	
	for (int i = 0; i < 10; ++i) {
		char my_data[DATA_LENGTH+1];
		memset(my_data, NUM_VALUES[i%10], sizeof(char)*DATA_LENGTH);
		my_data[DATA_LENGTH] = NULL_CHAR;
		
		print_semaphores();
		printf("Enqueuing %s\n\n", my_data);
		enqueue_buffer_421(my_data);
		print_semaphores();
		
		memset(my_data, NULL_CHAR, sizeof(char)*DATA_LENGTH);
		
		dequeue_buffer_421(my_data);
		printf("Dequeuing %s\n\n", my_data);
	}
	delete_buffer_421();
	*/ // END OF TEST W/O PTHREAD
	
	
	// Create and initialize the buffer
	init_buffer_421();
	printf("Initialized buffer\n");
	
	// Instantiate two pthreads, one for producer and one for consumer
	pthread_t produce_thread, consume_thread;
	
	// Start both pthreads, call producer or consumer for respective threads
	pthread_create(&produce_thread, NULL, producer, NULL);
	pthread_create(&consume_thread, NULL, consumer, NULL);
	
	// Make sure threads finish after executing their respective functions
	pthread_join(produce_thread, NULL);
	pthread_join(consume_thread, NULL);
	
	delete_buffer_421();

	return 0;
}


void *producer() {
	// Instantiate a char array for the enqueue data
	char in_data[DATA_LENGTH];
	
	//Enqueue data NUM_CYCLES number of times
	for (int i = 0; i < NUM_CYCLES; ++i) {
		
   		// Intialize seed for random number generator, reinitializes each loop
   		srand((unsigned)time(NULL));
		
		// Set the data from in_data to a value 0-9 DATA_LENGTH number of times
		memset(in_data, NUM_VALUES[i%10], sizeof(char)*DATA_LENGTH);
		
		print_semaphores();
		// Call enqueue_buffer_421 to modify the buffer
		enqueue_buffer_421(in_data);
		
		// Call usleep to make the produce sleep for a random num of milliseconds
		usleep((unsigned)(rand() % 100000));
	}
}


void *consumer() {
	// Instantiate char array for dequeue data and set last char to NULL_CHAR
	char out_data[DATA_LENGTH+1];
	out_data[DATA_LENGTH] = NULL_CHAR;
	
	//D3queue data NUM_CYCLES number of times
	for (int i = 0; i < NUM_CYCLES; ++i) {	
   		// Intialize seed for random number generator, reinitializes each loop
   		srand((unsigned)(time(NULL)+1));
		
		// Clear out_data before dequeuing?
		//memset(out_data, NULL_CHAR, sizeof(char)*DATA_LENGTH);
		
		//Call dequeue_buffer_421 to get data from the buffer
		dequeue_buffer_421(out_data);
		puts(out_data);
		
		// Call usleep to make the produce sleep for a random num of milliseconds
		usleep((unsigned)(rand() % 100000));
	}
}

