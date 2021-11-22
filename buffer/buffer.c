#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#include "buffer.h"

#define NULL_CHAR '\0'

static ring_buffer_421_t buffer;

// Define/initialize the semaphores we want to use
DEFINE_SEMAPHORE(mutex);
struct semaphore fill_count = __SEMAPHORE_INITIALIZER(fill_count, 0);
struct semaphore empty_count = __SEMAPHORE_INITIALIZER(empty_count, SIZE_OF_BUFFER);


SYSCALL_DEFINE0(init_buffer_421) {
	// Ensure we're not initializing a buffer that already exists.
	if (buffer.read || buffer.write) {
		printk("init_buffer_421(): Buffer already exists. Aborting.\n");
		return -1;
	}

	// Create the root node.
	node_421_t *node;
	node = (node_421_t *)kmalloc(sizeof(node_421_t), GFP_KERNEL);
	
	// Create the rest of the nodes, linking them all together.
	node_421_t *curr_node;
	int i;
	curr_node = node;
	// Note that we've already created one node, so i = 1.
	for (i = 1; i < SIZE_OF_BUFFER; i++) {
		curr_node->next = (node_421_t *)kmalloc(sizeof(node_421_t), GFP_KERNEL);
		curr_node = curr_node->next;
	}
	// Complete the chain.
	curr_node->next = node;
	buffer.read = node;
	buffer.write = node;
	buffer.length = 0;

	return 0;
}


SYSCALL_DEFINE1(enqueue_buffer_421, char*, data) {
	if (!buffer.write) {
		printk("enqueue_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}
	// Decrement mutex semaphore to 0 and empty_count by 1 for new data space
	down(&empty_count);
	down(&mutex);
	
	// Copy 1024 bytes of data into write node's data
	copy_from_user(buffer.write->data, data, DATA_LENGTH);
	
	// Advance the write pointer
	buffer.write = buffer.write->next;
	buffer.length++;

	// Increment mutex semaphore back to 1 and fill_count by 1
	up(&mutex);
	up(&fill_count);
	
	return 0;
}


SYSCALL_DEFINE1(dequeue_buffer_421, char*, data) {
	// Cannot dequeue from an uninstantiated buffer
	if (!buffer.read) {
		printk("dequeue_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}
	
	// Decrement the fill_count and change mutex back to 0
	down(&fill_count);
	down(&mutex);
	
	// Copy data from read pointer to data argument and clear old data
	copy_to_user(data, buffer.read->data, DATA_LENGTH);
	memset(buffer.read->data, NULL_CHAR, DATA_LENGTH);
	
	// Advance the read pointer and decrease the buffer length
	buffer.read = buffer.read->next;
	buffer.length--;
	
	// Increment the empty count and change mutex back to 1
	up(&mutex);
	up(&empty_count);
	
	return 0;
}


SYSCALL_DEFINE0(delete_buffer_421) {
	// Tip: Don't call this while any process is waiting to enqueue or dequeue.
	if (buffer.read) {
		// Get rid of all existing nodes.
		node_421_t *temp;
		node_421_t *curr_node;
		
		curr_node = buffer.read->next;
		while (curr_node != buffer.read) {
			temp = curr_node->next;
			kfree(curr_node);
			curr_node = temp;
		}
		
		// Free the final node.
		kfree(curr_node);
		curr_node = NULL;

		// Reset the buffer.
		buffer.read = NULL;
		buffer.write = NULL;
		buffer.length = 0;
		
		return 0;
	} 
	else { // If buffer.read is NULL, print error and return -1
		printk("delete_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}
}

/*
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
*/


