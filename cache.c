#include <stdio.h>
#include <math.h>

#include "cache.h"

void init_cache(char *cfg, struct cache *cache)
{
    FILE *file = fopen(cfg, "r");
    fscanf(file, "%d", &cache->cfg.block_size);
    fscanf(file, "%d", &cache->cfg.assoc);
    fscanf(file, "%d", &cache->cfg.size);
    fscanf(file, "%d", &cache->cfg.is_lru);
    fscanf(file, "%d", &cache->cfg.cost);
    fscanf(file, "%d", &cache->cfg.is_wa);
    fclose(file);
    
    cache->num_sets = (cache->cfg.assoc == 0) ? 1 : (cache->cfg.size * 1024) / (cache->cfg.block_size * cache->cfg.assoc);
    cache->num_lines = (cache->cfg.assoc == 0) ? (cache->cfg.size * 1024) / cache->cfg.block_size : cache->cfg.assoc;

    cache->sets = (struct cache_set *)malloc(sizeof(struct cache_set) * cache->num_sets);
    for (int i = 0; i < cache->num_sets; i++)
    {
        cache->sets[i].lines = (struct cache_line *)malloc(sizeof(struct cache_line) * cache->num_lines);
        for (int j = 0; j < cache->num_lines; j++)
        {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].tag = 0;
            cache->sets[i].lines[j].lru = 0;
            cache->sets[i].lines[j].dirty = 0;
        }
    }
}

void access_cache(mem_addr_t mem_addr, struct cache *cache)
{
    cache->cycles = 1;
    cache->flags = 0;
    cache->mem_addr = mem_addr;

    int offset_bits = log2(cache->cfg.block_size);
    int index_bits = log2(cache->num_sets);

    cache->cache_addr.block_offset = mem_addr & ((1 << offset_bits) - 1);
    mem_addr >>= offset_bits;
    cache->cache_addr.set = mem_addr & ((1 << index_bits) - 1);
    mem_addr >>= index_bits;
    cache->cache_addr.tag = mem_addr;

    struct cache_set *set = &cache->sets[cache->cache_addr.set];
    struct cache_line *line;
    for (int i = 0; i < cache->num_lines; i++)
    {
        line = &set->lines[i];
        if (line->valid && line->tag == cache->cache_addr.tag)
        {
            line->lru = 0;
            if (cache->cfg.is_lru)
            {
                for (int j = 0; j < cache->num_lines; j++)
                {
                    if (j != i && set->lines[j].valid && set->lines[j].lru < line->lru)
                    {
                        set->lines[j].lru++;
                    }
                }
            }
            cache->flags |= CACHE_HIT;
            return;
        }
    }

    cache->flags |= CACHE_MISS;
    cache->cycles += cache->cfg.cost;

    for (int i = 0; i < cache->num_lines; i++)
    {
        line = &set->lines[i];
        if (!line->valid)
        {
            line->valid = 1;
            line->tag = cache->cache_addr.tag;
            line->lru = 0;
            return;
        }
    }
    cache->flags |= CACHE_EVICT;
    line = &set->lines[0];
    for (int i = 0; i < cache->num_lines; i++) {
        if (set->lines[i].lru > line->lru) {
            line = &set->lines[i];
        }
    }
    line->valid = 1;
    line->tag = cache->cache_addr.tag;
    line->lru = 0;
}
