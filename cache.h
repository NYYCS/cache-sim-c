#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned long mem_addr_t;

struct cache_addr
{
    uint64_t tag;
    uint32_t set;
    uint32_t block_offset;
};

struct cache_cfg
{
    uint32_t size;       // in KB
    uint32_t block_size; // in bytes
    uint32_t assoc;
    uint32_t cost;
    uint32_t is_lru;
    uint32_t is_wa;
};

struct cache_line
{
    uint64_t tag;
    uint32_t valid;
    uint32_t dirty;
    uint8_t lru;
};

struct cache_set
{
    struct cache_line *lines;
};

struct cache
{
    struct cache_cfg cfg;
    struct cache_set *sets;
    struct cache_addr cache_addr;
    mem_addr_t mem_addr;
    uint32_t num_sets;
    uint32_t num_lines;
    uint8_t flags;
    uint64_t cycles;
};

#define CACHE_HIT 1
#define CACHE_MISS 2
#define CACHE_EVICT 4

void init_cache(char *cfg, struct cache *cache);
void access_cache(mem_addr_t mem_addr, struct cache *cache);

#endif
