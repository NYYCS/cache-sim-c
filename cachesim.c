#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cache.h"

struct args
{
    char cfg[64];
    char trace[64];
    char output[64];
    int out;
};

struct cache_stats
{
    uint64_t accesses;
    uint64_t hits;
    uint64_t misses;
    uint64_t evicts;
    uint64_t cycles;
};

struct args args;
struct cache cache;

struct cache_stats cache_stats;
struct cache_stats load_stats;
struct cache_stats store_stats;

void init_stats(struct cache_stats *cache_stats)
{
    cache_stats->accesses = 0;
    cache_stats->hits = 0;
    cache_stats->misses = 0;
    cache_stats->evicts = 0;
    cache_stats->cycles = 0;
}

void parse_args(int argc, char *argv[])
{
    args.out = 0;
    int opt;
    while ((opt = getopt(argc, argv, "c:t:o:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            strncpy(args.cfg, optarg, 64);
            break;
        case 't':
            strncpy(args.trace, optarg, 64);
            break;
        case 'o':
            strncpy(args.output, optarg, 64);
            args.out = 1;
            break;
        default:
            printf("Usage: ./cachesim -c <file> -t <file> [-o <file>]>\n");
            exit(1);
        }
    }
}

void sim_cache()
{
    FILE *trace_file = fopen(args.trace, "r");
    FILE *output_file;

    if (args.out)
    {
        output_file = fopen(args.output, "w");
    }
    else
    {
        char filename[128];
        strncpy(filename, args.trace, 64);
        strncat(filename, ".out", 8);
        output_file = fopen(filename, "w");
    }

    char buf[64];
    mem_addr_t inst_addr, mem_addr;
    char op = 'I';


    while (fgets(buf, 64, trace_file) != NULL)
    {   
        if (buf[0] == '#')
        {
            break;
        }

        sscanf(buf, "%lx: %c %lx", &inst_addr, &op, &mem_addr);

        cache_stats.accesses++;
        if (op == 'R')
            load_stats.accesses++;
        if (op == 'W')
            store_stats.accesses++;

        access_cache(mem_addr, &cache);
        cache_stats.hits += (cache.flags & CACHE_HIT) != 0;
        cache_stats.misses += (cache.flags & CACHE_MISS) != 0;
        cache_stats.evicts += (cache.flags & CACHE_EVICT) != 0;
        cache_stats.cycles += cache.cycles;

        if (op == 'R')
        {
            load_stats.hits += (cache.flags & CACHE_HIT) != 0;
            load_stats.misses += (cache.flags & CACHE_MISS) != 0;
            load_stats.evicts += (cache.flags & CACHE_EVICT) != 0;
            load_stats.cycles += cache.cycles;
        }
        if (op == 'W')
        {
            store_stats.hits += (cache.flags & CACHE_HIT) != 0;
            store_stats.misses += (cache.flags & CACHE_MISS) != 0;
            store_stats.evicts += (cache.flags & CACHE_EVICT) != 0;
            store_stats.cycles += cache.cycles;
        }
    }

    fprintf(output_file, "Total Hit Rate: %.2lf\n", (double)(cache_stats.hits) / (double)(cache_stats.accesses) * 100.0);
    fprintf(output_file, "Load Hit Rate: %.2lf\n", (double)load_stats.hits / (double)load_stats.accesses * 100.0);
    fprintf(output_file, "Store Hit Rate: %.2lf\n", (double)store_stats.hits / (double)store_stats.accesses * 100.0);
    fprintf(output_file, "Total Run Time: %llu\n", cache_stats.cycles);
    fprintf(output_file, "Average MA Latency: %2lf",   (double)cache_stats.cycles /  (double)cache_stats.accesses);

    fclose(trace_file);
    fclose(output_file);
}


int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    init_cache(args.cfg, &cache);
    sim_cache();
}