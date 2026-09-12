#include <stdlib.h>
#include "lrustack.h"

/**
 * Function to initialize an LRU stack for a cache set with a given <size>. 
 * 
 * @param size size of the LRU stack to initialize. 
 * @return the dynamically allocated stack. 
 */
lru_stack_t* init_lru_stack(int size) {

    //  malloc the space for the stack
	lru_stack_t* stack = (lru_stack_t*) malloc(sizeof(lru_stack_t));

    //  Set the stack size the caller passed in
	stack->size = size;
        
    //MRU AT FRONT
    stack->order = (int*) malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        stack->order[i] = i;
    }
	return stack;
}

/**
 * Function to get the index of the least recently used cache block, as indicated by <stack>.
 * 
 * @param stack stack to run the operation on.
 * @return the index of the LRU cache block.
 */
int lru_stack_get_lru(lru_stack_t* stack) {

    return stack->order[stack->size - 1];
}

/**
 * Function to mark the cache block with index <n> as MRU in <stack>. 
 * 
 * @param stack stack to run the operation on.
 * @param n the index to promote to MRU.  
 */
void lru_stack_set_mru(lru_stack_t* stack, int n) {
    //find the position os the given n;
    int pos = 0;
    for (int i = 0; i < stack->size; i++) {
        if (stack->order[i] == n) {
            pos = i;
            break;
        }
    }

    //remove the found position by shifting everything to the left of it by one
    for (int i = pos; i > 0; i--) {
        stack->order[i] = stack->order[i - 1];
    }

    // put the n back into the first position
    stack->order[0] = n;
}

/**
 * Function to free up any memory you dynamically allocated for <stack>
 * 
 * @param stack the stack to free
 */
void lru_stack_cleanup(lru_stack_t* stack) {

    free(stack->order);

    free(stack);        // Free the stack struct we malloc'd
}