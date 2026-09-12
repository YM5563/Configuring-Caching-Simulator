#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachesim.h"

// Statistics to keep track, might add more.
counter_t accesses = 0;// Total number of cache accesses
counter_t hits = 0;// Total number of cache hits
counter_t misses = 0; // Total number of cache misses
counter_t writebacks = 0;// Total number of writebacks

/**
 * Function to perform a very basic log2. NOT A FULL LOG FUNCTION.
 * But the <math.h> library was having issues
 * 
 * @param x number you want the log of.
 * @returns floor(log_2(x))
 */
int simple_log_2(int x) {
    int val = 0;
    while (x > 1) {
        x /= 2;
        val++;
    }
    return val; 
}

cache_set_t* cache;// Data structure for the cache
int block_size;    // Block size
int cache_size;    // Cache size
int ways; // Ways
int num_sets;// Number of sets
int num_offset_bits;// Number of offset bits
int num_index_bits;// Number of index bits. 

/**
 * Function to intialize the cache simulator with the given cache parameters. 
 * 
 * @param _block_size block size in bytes
 * @param _cache_size cache size in bytes
 * @param _ways associativity
 */
void cachesim_init(int _block_size, int _cache_size, int _ways) {
    // Set cache parameters to global variables
    block_size = _block_size;
    cache_size = _cache_size;
    ways = _ways;

    //Calculate some of the fields 
    num_offset_bits = simple_log_2(block_size);
    num_sets        = cache_size / (block_size * ways);
    num_index_bits  = simple_log_2(num_sets);

    //allocate space
    cache = (cache_set_t*) malloc(num_sets * sizeof(cache_set_t));

    //initialize for each set and blocks, the tag, valid, and dirty bit
    for (int s = 0; s < num_sets; s++) {
        cache[s].size = ways;
 
        cache[s].blocks = (cache_block_t*) malloc(ways * sizeof(cache_block_t));
        for (int b = 0; b < ways; b++) {
            cache[s].blocks[b].tag   = 0;
            cache[s].blocks[b].valid = 0;
            cache[s].blocks[b].dirty = 0;
        }
 
        // Allocate and initialize the lru
        cache[s].stack = init_lru_stack(ways);
    }
}

/**
 * Function to perform a single memory access to the cache. Update the statistics 
 * (accesses, hits, misses, writebacks) and update the cache data structure
 * 
 * @param physical_addr address to use for the memory access. 
 * @param access_type type of access - 0 (data read), 1 (data write) or 
 *      2 (instruction read). (MEMREAD, MEMWRITE, IFETCH)
 */
void cachesim_access(addr_t physical_addr, int access_type) {
    
    //update access counter;
    accesses++;

    //get the index and tag from the given param
    int index = (int)((physical_addr >> num_offset_bits) & (num_sets - 1));
    int tag   = (int)( physical_addr >> (num_offset_bits + num_index_bits));
 
    cache_set_t* set = &cache[index];

    //initialise hit as -1 first
    int hit_block = -1;

    for (int b = 0; b < ways; b++) {

        //ensure to check the valid bit
        if (set->blocks[b].valid && set->blocks[b].tag == tag) {
            hit_block = b;
            break;
        }
    }

    if (hit_block != -1) { //means we hit;
        hits++;
 
        // if we are doing write, have to set it dirty
        if (access_type == MEMWRITE) {
            set->blocks[hit_block].dirty = 1;
        }
 
        //now this is mru
        lru_stack_set_mru(set->stack, hit_block);
 
    } else { //means we didn't hit
        misses++;
 
        //get the evicting block
        int victim = lru_stack_get_lru(set->stack);
 
        //make sure if its valid and dirty, we writeback
        if (set->blocks[victim].valid && set->blocks[victim].dirty) {
            writebacks++;
        }
 
        // put the new block in the victim slot
        set->blocks[victim].tag   = tag;
        set->blocks[victim].valid = 1;
        // set to dirty only if this is a write
        set->blocks[victim].dirty = (access_type == MEMWRITE) ? 1 : 0;
 
        // put that into the mru now since its new
        lru_stack_set_mru(set->stack, victim);
    }

}

/**
 * Function to free up any dynamically allocated memory
 */
void cachesim_cleanup() {

    //go through the stack and the blocks
    for (int s = 0; s < num_sets; s++) {
        lru_stack_cleanup(cache[s].stack);
        free(cache[s].blocks);
    }
    //free the cache at the end
    free(cache);
}

/**
 * Function to print cache statistics
 */
void cachesim_print_stats() {
    printf("%llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);  
}

/**
 * Function to open the trace file
 */
FILE *open_trace(const char *filename) {
    return fopen(filename, "r");
}

/**
 * Read in next line of the trace
 * 
 * @param trace file handler for the trace
 * @return 0 when error or EOF and 1 otherwise. 
 */
int next_line(FILE* trace) {
    if (feof(trace) || ferror(trace)) return 0;
    else {
        int t;
        unsigned long long address, instr;
        fscanf(trace, "%d %llx %llx\n", &t, &address, &instr);
        cachesim_access(address, t);
    }
    return 1;
}

/**
 * Main function. 
 * 
 * @param argc number of arguments
 * @param argv Argument values
 * @returns 0 on success. 
 */
int main(int argc, char **argv) {
    FILE *input;

    if (argc != 5) {
        fprintf(stderr, "Usage:\n  %s <trace> <block size(bytes)>"
                        " <cache size(bytes)> <ways>\n", argv[0]);
        return 1;
    }
    
    input = open_trace(argv[1]);
    cachesim_init(atol(argv[2]), atol(argv[3]), atol(argv[4]));
    while (next_line(input));
    cachesim_print_stats();
    cachesim_cleanup();
    fclose(input);
    return 0;
}

