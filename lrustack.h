#ifndef __LRUSTACK_H
#define __LRUSTACK_H

/**
 * struct to hold a cache set's LRU stack.
 */
typedef struct lru_stack_t {
	int size;  // Corresponds to the associativity
    int* order; //just keep an array where the rigth most index is LRU and left most is MRU
} lru_stack_t;

/**
 * Function to initialize an LRU stack for a cache set with a given <size>.
 * 
 * @param size size of the LRU stack to initialize. 
 * @return the dynamically allocated stack. 
 */
lru_stack_t* init_lru_stack(int size);

/**
 * Function to get the index of the least recently used cache block, as indicated by <stack>.
 * 
 * @param stack stack to run the operation on.
 * @return the index of the LRU cache block.
 */
int lru_stack_get_lru(lru_stack_t* stack);

/**
 * Function to mark the cache block with index <n> as MRU in <stack>.
 * 
 * @param stack stack to run the operation on.
 * @param n the index to promote to MRU.  
 */
void lru_stack_set_mru(lru_stack_t* stack, int n);

/**
 * Function to free up any memory you dynamically allocated for <stack>
 * 
 * @param stack the stack to free
 */
void lru_stack_cleanup(lru_stack_t* stack);

#endif