#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "buffer.h"

#define NULL_CHAR '\0'

static ring_buffer_421_t buffer;
static sem_t mutex;
static sem_t fill_count;
static sem_t empty_count;


long init_buffer_421(void) {
	// Ensure we're not initializing a buffer that already exists.
	if (buffer.read || buffer.write) {
		printf("init_buffer_421(): Buffer already exists. Aborting.\n");
		return -1;
	}

	// Create the root node.
	node_421_t *node;
	node = (node_421_t *) malloc(sizeof(node_421_t));
	// Create the rest of the nodes, linking them all together.
	node_421_t *current;
	int i;
	current = node;
	// Note that we've already created one node, so i = 1.
	for (i = 1; i < SIZE_OF_BUFFER; i++) {
		current->next = (node_421_t *) malloc(sizeof(node_421_t));
		current = current->next;
	}
	// Complete the chain.
	current->next = node;
	buffer.read = node;
	buffer.write = node;
	buffer.length = 0;

	// Initialize semaphores
	sem_init(&mutex, 0, 1);
	sem_init(&fill_count, 0, 0);  // No data in buffer yet
	sem_init(&empty_count, 0, SIZE_OF_BUFFER);  // Buffer is empty

	return 0;
}


long enqueue_buffer_421(char * data) {
	if (!buffer.write) {
		printf("The buffer does not exist. Aborting.\n");
		return -1;
	}
	// Decrement mutex semaphore to 0 and empty_count by 1 for new data space
	sem_wait(&empty_count);
	sem_wait(&mutex);
	
	// Copy 1024 bytes of data into write node's data
	memcpy(buffer.write->data, data, DATA_LENGTH);
	
	// Advance the write pointer
	buffer.write = buffer.write->next;
	buffer.length++;

	// Increment mutex semaphore back to 1 and fill_count by 1
	sem_post(&mutex);
	sem_post(&fill_count);
	
	return 0;
}


long dequeue_buffer_421(char * data) {
	// Cannot dequeue from an uninstantiated buffer
	if (!buffer.read) {
		printf("The buffer does not exist. Aborting.\n");
		return -1;
	}
	// Change mutex back to 0
	sem_wait(&fill_count);
	sem_wait(&mutex);
	
	// Copy data from read pointer to data argument and clear old data
	memcpy(data, buffer.read->data, DATA_LENGTH);
	memset(buffer.read->data, NULL_CHAR, DATA_LENGTH);
	
	// Advance the read pointer and decrease the buffer length
	buffer.read = buffer.read->next;
	buffer.length--;
	
	// Change mutex back to 1
	sem_post(&mutex);
	sem_post(&empty_count);
	
	return 0;
}


long delete_buffer_421(void) {
	// Tip: Don't call this while any process is waiting to enqueue or dequeue.
	if (!buffer.read) {
		printf("delete_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}
	// Get rid of all existing nodes.
	node_421_t *temp;
	node_421_t *current = buffer.read->next;
	while (current != buffer.read) {
		temp = current->next;
		free(current);
		current = temp;
	}
	
	// Free the final node.
	free(current);
	current = NULL;

	// Reset the buffer.
	buffer.read = NULL;
	buffer.write = NULL;
	buffer.length = 0;
	
	return 0;
}


void print_semaphores(void) {
	// You can call this method to check the status of the semaphores.
	// Don't forget to initialize them first!
	// YOU DO NOT NEED TO IMPLEMENT THIS FOR KERNEL SPACE.
	int value;
	sem_getvalue(&mutex, &value);
	printf("sem_t mutex = %d\n", value);
	sem_getvalue(&fill_count, &value);
	printf("sem_t fill_count = %d\n", value);
	sem_getvalue(&empty_count, &value);
	printf("sem_t empty_count = %d\n", value);
	return;
}

