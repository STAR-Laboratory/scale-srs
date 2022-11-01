//
// Created by Seokin Hong on 3/29/18.
//

#ifndef SRC_CACHE_HPP
#define SRC_CACHE_HPP

#include <stdbool.h>
#include <stdint.h>
#include "global_types.h"

#define FALSE 0
#define TRUE  1

#define HIT   1
#define MISS  0

#define MCACHE_SRRIP_MAX  7
#define MCACHE_SRRIP_INIT 1
#define MCACHE_PSEL_MAX    1023
#define MCACHE_LEADER_SETS  1

//typedef unsigned uns;
//typedef unsigned char uns8;
//typedef unsigned short uns16;
//typedef unsigned uns32;
//typedef unsigned long long uns64;
//typedef short int16;
//typedef int int32;
//typedef int long long int64;
//typedef int Generic_Enum;

/* Conventions */
//typedef uns32 Binary;
//typedef uns8 Flag;
//typedef uns64		    Addr;

//typedef uns64 Counter;
//typedef int64 SCounter;

typedef struct MCache_Entry {
    Flag valid;
    Flag dirty;
    Addr tag;
    Addr pc;
    uns ripctr;
    uns64 last_access;
} MCache_Entry;

typedef enum MCache_ReplPolicy_Enum {
    REPL_LRU = 0,
    REPL_RND = 1,
    REPL_SRRIP = 2,
    REPL_DRRIP = 3,
    REPL_FIFO = 4,
    REPL_DIP = 5,
    NUM_REPL_POLICY = 6
} MCache_ReplPolicy;

typedef struct MCache {
    uns sets;
    uns assocs;
    uns linesize;
    uns64 lineoffset;
    MCache_ReplPolicy repl_policy; //0:LRU  1:RND 2:SRRIP
    uns index_policy; // how to index cache
    
    Flag *is_leader_p0; // leader SET for D(RR)IP
    Flag *is_leader_p1; // leader SET for D(RR)IP
    uns psel;
    
    MCache_Entry *entries;
    uns *fifo_ptr; // for fifo replacement (per set)
    
    uns64 s_count; // number of accesses
    uns64 s_miss; // number of misses
    uns64 s_evict; // number of evictions
    uns64 s_writeback; // number of writeback
    
    uns64 s_read;
    uns64 s_write;
    uns64 s_read_hit;
    uns64 s_write_hit;
    
    int touched_wayid;
    int touched_setid;
    int touched_lineid;
} MCache;

uns64 logtwo(uns x);

void init_cache(MCache* c, uns sets, uns assocs, uns repl, uns block_size);

int isHit(MCache* cache, Addr addr, Flag dirty);

MCache_Entry install(MCache* cache, Addr addr, Addr pc, Flag dirty);

MCache_Entry mcache_install(MCache *c, Addr addr, Addr pc, Flag dirty);

bool mcache_access(MCache *c, Addr addr, Flag dirty);  //true: hit, false: miss

Flag mcache_probe(MCache *c, Addr addr);

Flag mcache_invalidate(MCache *c, Addr addr);

Flag mcache_mark_dirty(MCache *c, Addr addr);

Flag mcache_mark_valid(MCache *c, Addr addr);

uns mcache_get_index(MCache *c, Addr addr);

uns mcache_find_victim(MCache *c, uns set);

uns mcache_find_victim_lru(MCache *c, uns set);

uns mcache_find_victim_rnd(MCache *c, uns set);

uns mcache_find_victim_srrip(MCache *c, uns set);

uns mcache_find_victim_fifo(MCache *c, uns set);

void mcache_swap_lines(MCache *c, uns set, uns way_i, uns way_j);

void mcache_select_leader_sets(MCache *c, uns sets);

uns mcache_drrip_get_ripctrval(MCache *c, uns set);

Flag mcache_dip_check_lru_update(MCache *c, uns set);

void mcache_reset(MCache *c);

void print_cache_stats(MCache *c, unsigned long long int instructions);

#endif //SRC_CACHE_HPP
