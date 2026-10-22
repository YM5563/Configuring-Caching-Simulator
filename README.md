# Configurable Cache Simulator

A small cache simulator written in C. You hand it a memory trace and a cache configuration (block size, total size, associativity), and it replays every access through a simulated cache and tells you how many hits, misses, and writebacks you'd get.

The point is to make it easy to answer questions like "what happens to my miss rate if I double the block size?" or "is 8-way really better than 4-way for this workload?" without touching real hardware.

## What it models

- **Set-associative cache** with configurable block size, cache size, and number of ways. Direct-mapped is just `ways = 1`.
- **LRU replacement.** Each set keeps its own LRU stack, so the block that hasn't been touched the longest is the one that gets evicted.
- **Write-back, write-allocate.** Writes mark the block dirty instead of going straight to memory. When a dirty block gets evicted, that counts as a writeback. Write misses pull the block into the cache first.
- Only one level of cache. There's no L2, no prefetching, no timing. It's purely about counting hits and misses.

## Files

| File | What it does |
|------|--------------|
| `cachesim.c` / `cachesim.h` | The cache itself. Figures out index and tag bits, looks up blocks, handles misses and evictions, and reads the trace file. |
| `lrustack.c` / `lrustack.h` | The LRU stack used by each set. A small array where the front is most recently used and the back is least recently used. |
| `lrustacktest.c` | A handful of sanity tests for the LRU stack. |
| `Traces/` | Three sample traces to run against (see below). |
| `Tests/test_outputs.txt` | Expected results for a few configurations, so you can check your build is doing the right thing. |

## Building

There's no Makefile, just two `gcc` lines:

```sh
# the simulator
gcc -o cachesim cachesim.c lrustack.c

# the LRU stack tests (optional)
gcc -o lrustacktest lrustacktest.c lrustack.c
```

## Running it

```sh
./cachesim <trace file> <block size in bytes> <cache size in bytes> <ways>
```

For example, a 64 KB 8-way cache with 32-byte blocks running the gcc trace:

```sh
./cachesim Traces/trace.gcc.txt 32 65536 8
```

Output is a single line of four numbers:

```
1000000, 994989, 5011, 1810
```

| Position | Meaning |
|----------|---------|
| 1 | Total accesses |
| 2 | Hits |
| 3 | Misses |
| 4 | Writebacks (dirty blocks that got evicted) |

Block size, cache size, and ways should all be powers of two. The simulator doesn't check this, so if you pass something weird the index math will quietly go wrong.

## Trace format

Each line in a trace is one memory access:

```
<type> <address in hex> <instruction in hex>
```

| Type | Meaning |
|------|---------|
| 0 | Data read |
| 1 | Data write |
| 2 | Instruction fetch |

The third column (the instruction word) is read but ignored by the simulator. Only the type and address matter. Since there's no split I-cache and D-cache, instruction fetches go through the same cache as data.

### Included traces

| Trace | Accesses | What it is |
|-------|----------|------------|
| `trace.gcc.txt` | 1,000,000 | A slice of gcc compiling something. Fairly typical program behavior. |
| `trace.merge.txt` | 1,048,575 | A merge sort. Lots of sequential streaming through arrays. |
| `trace.random64k.txt` | 262,144 | Random accesses over a 64 KB region. Good for sanity checking, since it fits in a 64 KB cache and produces zero writebacks. |

## Checking your results

`Tests/test_outputs.txt` lists the expected output for nine configurations. A few of them:

| Command | Expected |
|---------|----------|
| `./cachesim Traces/trace.gcc.txt 32 65536 8` | `1000000, 994989, 5011, 1810` |
| `./cachesim Traces/trace.merge.txt 64 65536 4` | `1048575, 1042197, 6378, 1870` |
| `./cachesim Traces/trace.random64k.txt 32 65536 8` | `262144, 260096, 2048, 0` |

If you want to test just the LRU stack on its own:

```sh
./lrustacktest
```

You should see a list of `TestN Succeeded!` lines.

## How it works, briefly

1. On startup, the simulator computes how many sets it needs (`cache size / (block size * ways)`) and how many bits of the address go to the block offset and the set index. Everything above that is the tag.
2. For every access, it pulls out the index and tag, walks the blocks in that set, and looks for a valid block with a matching tag.
3. **On a hit**, it bumps the hit counter, marks the block dirty if this was a write, and moves it to the front of the set's LRU stack.
4. **On a miss**, it asks the LRU stack which block to evict. If that block is valid and dirty, it's a writeback. Then the new tag goes in, the dirty bit is set based on whether this access was a write, and the block becomes most recently used.

That's really all there is to it.
