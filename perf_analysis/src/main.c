#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
#include <math.h>
#include "processor.h"
#include "configfile.h"
#include "memory_controller.h"
#include "scheduler.h"
#include "params.h"
#include "randomize.h"
#include "cache.h"
#include "global_types.h"
#include "os.h"
#include "malloc_lib.h"
#include "utlist.h"
#include "utils.h"
#include <time.h>

#define L3_LATENCY 24

#define MAXTRACELINESIZE 64
#define EPOCH_LENGTH 100
#define EPOCH_LENGTH 100
#define PREFETCH_BUFFER_SIZE 128
#define ACCESS_ADDR_BUFFER_SIZE 64
#define SLOT_TIME 304
#define MAX_NUM_REQS 3
#define MAX_BLOCK_SIZE 64
#define SCORE_MAX 31
#define ROUND_MAX 100
#define EPOCH 124
#define FIX_TIME 256 
#define STALL_TIME  0
#define STALL_TIME_MC 148
#define MSHRSIZE 24
#define CACHEDELAY 1

// #define p 419
// #define q 541
// #define e 2737
// #define p 17
// #define q 11
// #define e 7
#define p 3529
#define q 7703
#define e 13

/* ======================
   FR-FCFS = 1
   FCFS = 2
   Close Page = 3
   BTA = 4
   Mode1 = 5
   FS = 6
   Mode2 =7
   SECMEM = 4
   Plumber-R = 5
   FS = 6
   Plumber-Q-Stream =7
   Plumber-Q-BO = 8
   LPS = 9
   TP = 10
   Camouflage = 11
   ======================== */


// Trace file .gz reading definitions
void init_trace(char *, gzFile *);

// CYCLE_VAL keeps a global counter for all elements to tick.
long long int CYCLE_VAL = 0;

// Row Indirection
long int *rowindirection;

//Stats for MCPI
unsigned long long int queuing_delay_total = 0;
unsigned long long int readreq_total = 0;
unsigned long long int *queuing_delay = NULL;
unsigned long long int *readreq = NULL;

//Prefetchers for Plumber-Q
void stream_prefetcher(unsigned long long int reference_addr, unsigned long long int* prefetch_addr);
void bo_prefetcher(unsigned long long int reference_addr, unsigned long long int* prefetch_addr, unsigned long long int ***prefetch_buffer, int numcores, int num_channels, int prefetch_buffer_size);
int request_in_prefetch_buffer(long long int addr, unsigned long long int ***prefetch_buffer, int numcores, int num_channels, int prefetch_buffer_size);

long long int randomizer(long long int physical_addr);

long long int get_current_cycle()
{
    return CYCLE_VAL;
}

typedef struct {
 long long int d;
 long long int x;
 long long int y;
} EE;

//Struct of offset buffer
struct off_buffer
{
    int offset;
    int score;
}; 

struct off_buffer  ***offset_buffer;

tracker_t ***per_thread_tracker;

//The L3 Cache model and its latency
MCache *L3Cache;
MCache *Cache;
int LAT_6EC7ED;

//The ROB to simulate OOO behaviour
struct robstructure *ROB;

//LSQ size; 72 Loads and 44 entry store queues per core
int *loadq = NULL;
int *storeq = NULL;

//A big number that is used to avoid long stalls and let the ROB proceed ahead
long long int BIGNUM = 10000000;

//Stall counters for Statistics
long long int robf_stalls = 0;
long long int wrqf_stalls = 0;
long long int robn_stalls = 0;
long long int wrqn_stalls = 0;

int num_round = 0;
int init_offset_buffer = 0;
//Logging counters for Statistics of time done and core power
long long int *time_done;
long long int total_time_done;
float core_power = 0;

int check_sandbox_first[2] = {0, 0};

//Number of Pages mapped
long long int page_counter = 0;

//Set this variable to notify if the sims are done
int expt_done = 0;

/*Indicates how many config params exist and their types*/
int config_param = 0;
gzFile *tif = NULL; /* The handles to the trace input files. */
FILE *config_file = NULL;
FILE *vi_file = NULL;
int *prefixtable; /* For (multi-threaded) MT workloads only */

// Bo-Prefetcher Global Values
unsigned long long int local_best_score_fetch = 1;
unsigned long long int local_second_score_fetch = 1;
unsigned long long int local_third_score_fetch = 1;


int main(int argc, char *argv[])
{

    printf("---------------------------------------------\n");
    printf("-- USIMM: the Utah SImulated Memory Module --\n");
    printf("--              Version: 1.3               --\n");
    printf("---------------------------------------------\n");
    srand(13);

    int numc = 0;
    int num_ret = 0;
    int num_fetch = 0;
    int num_done = 0;
    int numch = 0;
    long long int *committed;
    long long int *fetched;

    int writeqfull = 0;
    int readqfull = 0;
    int fnstart;
    int currMTapp;
    long long int maxtd;
    int maxcr;
    long long int temp_inst_cntr = 0;
    long long int temp_inst_cntr2 = 0;
    char newstr[MAXTRACELINESIZE];
    int *nonmemops;
    char *opertype;
    long long int *addr;
    long long int fake_num = 0;

    long long int *instrpc;
    unsigned long long int request_id_counter = 0;
    int *compressedSize1Line;
    int *compressedSize2Line;

    long long int ran_addr;
    long long int ran_bank_addr;
    unsigned long long int prefetch_hit = 0;

    unsigned long long int *fetch_addr = NULL;
    //struct off_buffer  *offset_buffer;

    // Prefetch Buffers
    unsigned long long int ***prefetch_buffer = NULL;
    unsigned long long int **prev_addr = NULL;
    // Access addr Buffer
    unsigned long long int ***addr_buffer = NULL;

    // Number to Issue
    long long int **num_issue = NULL;

    long long int *change_core = NULL;
    // Time to Issue
    unsigned long long int *timer = NULL;
    unsigned long long int **timer_bank = NULL;

    int **qlength;
    request_t *rd_ptr = NULL;
    unsigned int *runcore = NULL;
    unsigned int *prev_runcore = NULL;     
    unsigned long long int sbavg= 0;
    unsigned long long int te = 0;
    unsigned int ***issued = NULL;
    unsigned long long int ***Stall_bank = NULL;
    unsigned int ***bank_busy = NULL;
    unsigned int ***issue_fake = NULL; 
    unsigned int ***issue_r = NULL; 
    unsigned int ***no_real = NULL;
    unsigned int *chosen_core = NULL;
    unsigned int *rand_core = NULL;
    unsigned int **current_bank = NULL;
    unsigned int **selected_bank = NULL;
    unsigned int **bank_group = 0;
    unsigned int **prev_bank = NULL;
    //  unsigned int current_bank = 0;
    unsigned int **switch_core = NULL;
    unsigned long long int ***Stall_fake = NULL;
    unsigned int ***core_tracker = NULL; 
    unsigned int *core_instflight = NULL; 

    long long int *inter_arrival_time = NULL;
    int chips_per_rank = -1;
    long long int total_inst_fetched = 0;

    //Variable to check if Physical Addresses are being used as input
    unsigned long long int os_pages = 0;
    OS *os;

    //To keep track of how much is done
    unsigned long long int inst_comp = 0;

    unsigned long long int ref_addr = 0;
    // Performance Logging Variables
    unsigned long long int MIN_USIMM_CYCLES = 0;
    unsigned long long int MIN_FETCH_INST = 0;
    double ADDED_IPC = 0;
    double *ipccore;
    unsigned long long int fakereadreq = 0;

    unsigned long long int *L3MSHR;
    unsigned long long int stallnow = 0;
    unsigned long long int totalinst= 0;

    /* Initialization code. */
    printf("Initializing.\n");

    // add an argument for refreshing selection
    if (argc < 4)
    {
        fprintf(stderr, "Need at least one input configuration file and one trace file as argument.  Quitting.\n");
        return -3;
    }

    config_param = atoi(argv[1]);

    config_file = fopen(argv[2], "r");
    printf("%s\n", argv[2]);

    if (!config_file)
    {
        fprintf(stderr, "Missing system configuration file.  Quitting. \n");
        return -4;
    }
    read_config_file(config_file);

    NUMCORES = argc - 3;

    ROB = (struct robstructure *)calloc(NUMCORES, sizeof(struct robstructure));
    tif = (gzFile *)calloc(NUMCORES, sizeof(gzFile));
    committed = (long long int *)calloc(NUMCORES, sizeof(long long int));
    fetched = (long long int *)calloc(NUMCORES, sizeof(long long int));
    time_done = (long long int *)calloc(NUMCORES, sizeof(long long int));
    nonmemops = (int *)calloc(NUMCORES, sizeof(int));
    opertype = (char *)calloc(NUMCORES, sizeof(char));
    addr = (long long int *)calloc(NUMCORES, sizeof(long long int));
    instrpc = (long long int *)calloc(NUMCORES, sizeof(long long int));
    compressedSize1Line = (int *)calloc(NUMCORES, sizeof(int));
    compressedSize2Line = (int *)calloc(NUMCORES, sizeof(int));
    offset_buffer = (struct off_buffer ***)calloc(NUMCORES, sizeof(struct off_buffer **));
    for (int j = 0; j < NUMCORES; j++)
    {
        offset_buffer[j]=(struct off_buffer **)calloc(NUM_CHANNELS, sizeof(struct off_buffer *));
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            offset_buffer[j][k]=(struct off_buffer *)calloc(64, sizeof(struct off_buffer));
        }
    }

    loadq = (int *)calloc(NUMCORES, sizeof(int));
    storeq = (int *)calloc(NUMCORES, sizeof(int));
    runcore = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int)); 
    prev_runcore = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int)); 
    // issue_fake = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int));
    // current_bank = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int));  
    chosen_core = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int)); 
    rand_core = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int)); 
    timer = (unsigned long long int*)calloc(NUM_CHANNELS, sizeof(unsigned long long int));
    // switch_core = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int)); 

    issued = (unsigned int***)calloc(NUMCORES, sizeof(unsigned int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        issued[i]=(unsigned int**)calloc(NUM_CHANNELS, sizeof(unsigned int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            issued[i][j]=(unsigned int*)calloc(NUM_BANKS, sizeof(unsigned int));
        }
    }

    Stall_bank = (unsigned long long int***)calloc(NUMCORES, sizeof(unsigned long long int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        Stall_bank[i]=(unsigned long long int**)calloc(NUM_CHANNELS, sizeof(unsigned long long int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            Stall_bank[i][j]=(unsigned long long int*)calloc(NUM_BANKS, sizeof(unsigned long long int));
        }
    }

    Stall_fake = (unsigned long long int***)calloc(NUMCORES, sizeof(unsigned long long int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        Stall_fake[i]=(unsigned long long int**)calloc(NUM_CHANNELS, sizeof(unsigned long long int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            Stall_fake[i][j]=(unsigned long long int*)calloc(NUM_BANKS, sizeof(unsigned long long int));
        }
    }

    bank_busy = (unsigned int***)calloc(NUMCORES, sizeof(unsigned int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        bank_busy[i]=(unsigned int**)calloc(NUM_CHANNELS, sizeof(unsigned int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            bank_busy[i][j]=(unsigned int*)calloc(NUM_BANKS, sizeof(unsigned int));
        }
    }
    for (int i = 0; i < NUMCORES; i++)
    {
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
           for(int k = 0; k < NUM_BANKS; k++)
           {
               bank_busy[i][j][k] = 1;
           }
        }
    }

    timer_bank = (unsigned long long int**)calloc(NUMCORES, sizeof(unsigned long long int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        timer_bank[j]=(unsigned long long int*)calloc(NUM_CHANNELS, sizeof(unsigned long long int));
    }
    prev_addr=(unsigned long long int**)calloc(NUMCORES, sizeof(unsigned long long int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        prev_addr[j]=(unsigned long long int*)calloc(NUM_CHANNELS, sizeof(unsigned long long int));
    }

    current_bank=(unsigned int**)calloc(NUMCORES, sizeof(unsigned int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        current_bank[j]=(unsigned int*)calloc(NUM_CHANNELS, sizeof(unsigned int));
    }

    selected_bank=(unsigned int**)calloc(NUMCORES, sizeof(unsigned int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        selected_bank[j]=(unsigned int*)calloc(NUM_CHANNELS, sizeof(unsigned int));
    }

    bank_group=(unsigned int**)calloc(NUMCORES, sizeof(unsigned int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        bank_group[j]=(unsigned int*)calloc(NUM_CHANNELS, sizeof(unsigned int));
    }

    core_tracker=(unsigned int***)calloc(NUM_CHANNELS, sizeof(unsigned int**));
    for (int i = 0; i < NUM_CHANNELS; i++)
    {
        core_tracker[i]=(unsigned int**)calloc(NUM_BANKS, sizeof(unsigned int*));
        for (int j = 0; j < NUM_BANKS; j++)
        {
            core_tracker[i][j]=(unsigned int*)calloc(NUMCORES, sizeof(unsigned int));
        }
    }

    prev_bank=(unsigned int**)calloc(NUMCORES, sizeof(unsigned int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        prev_bank[j]=(unsigned int*)calloc(NUM_CHANNELS, sizeof(unsigned int));
    }

    issue_fake = (unsigned int***)calloc(NUMCORES, sizeof(unsigned int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        issue_fake[i]=(unsigned int**)calloc(NUM_CHANNELS, sizeof(unsigned int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            issue_fake[i][j]=(unsigned int*)calloc(NUM_BANKS, sizeof(unsigned int));
        }
    }

    issue_r = (unsigned int***)calloc(NUMCORES, sizeof(unsigned int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        issue_r[i]=(unsigned int**)calloc(NUM_CHANNELS, sizeof(unsigned int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            issue_r[i][j]=(unsigned int*)calloc(NUM_BANKS, sizeof(unsigned int));
        }
    }

    no_real = (unsigned int***)calloc(NUMCORES, sizeof(unsigned int **));
    for (int i = 0; i < NUMCORES; i++)
    {
        no_real[i]=(unsigned int**)calloc(NUM_CHANNELS, sizeof(unsigned int*));
        for (int j = 0; j < NUM_CHANNELS; j++)
        {
            no_real[i][j]=(unsigned int*)calloc(NUM_BANKS, sizeof(unsigned int));
        }
    }

    switch_core=(unsigned int**)calloc(NUMCORES, sizeof(unsigned int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        switch_core[j]=(unsigned int*)calloc(NUM_CHANNELS, sizeof(unsigned int));
    }         

    qlength=(int**)calloc(NUMCORES, sizeof(int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        qlength[j]=(int*)calloc(NUM_CHANNELS, sizeof(int));
    }

    prefetch_buffer = (unsigned long long int***)calloc(NUMCORES, sizeof(unsigned long long int**));
    for (int j = 0; j < NUMCORES; j++)
    {
        prefetch_buffer[j]=(unsigned long long int**)calloc(NUM_CHANNELS, sizeof(unsigned long long int*));
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            prefetch_buffer[j][k]=(unsigned long long int*)calloc(PREFETCH_BUFFER_SIZE, sizeof(unsigned long long int));
        }
    }

    addr_buffer = (unsigned long long int***)calloc(NUMCORES, sizeof(unsigned long long int**));
    for (int j = 0; j < NUMCORES; j++)
    {
        addr_buffer[j]=(unsigned long long int**)calloc(NUM_CHANNELS, sizeof(unsigned long long int*));
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            addr_buffer[j][k]=(unsigned long long int*)calloc(ACCESS_ADDR_BUFFER_SIZE, sizeof(unsigned long long int));
        }
    }

    num_issue = (long long int**)calloc(NUMCORES, sizeof(long long int*));
    for (int j = 0; j < NUMCORES; j++)
    {
        num_issue[j]=(long long int*)calloc(NUM_CHANNELS, sizeof(long long int));
    }
    change_core = (long long int*)calloc(NUMCORES, sizeof(long long int));

    fetch_addr = (unsigned long long int*)calloc(MAX_NUM_REQS, sizeof(unsigned long long int));
    prefixtable = (int *)calloc(NUMCORES, sizeof(int));
    ipccore = (double*)calloc(NUMCORES, sizeof(double));

    queuing_delay = (unsigned long long int *)calloc(NUMCORES, sizeof(unsigned long long int));
    readreq = (unsigned long long int*)calloc(NUMCORES, sizeof(unsigned long long int));
    
    per_thread_tracker = (tracker_t ***)calloc(NUMCORES, sizeof(tracker_t **));
    for (int j = 0; j < NUMCORES; j++)
    {
        per_thread_tracker[j] = (tracker_t **)calloc(NUM_CHANNELS, sizeof(tracker_t*));
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            per_thread_tracker[j][k] = (tracker_t *)calloc(NUM_BANKS, sizeof(tracker_t));
            for (int i = 0; i < NUM_BANKS; i++)
            {
                per_thread_tracker[j][k]->tracker_i[i] = (request_t *)calloc(1, sizeof(request_t));
            }
        }
    }

    for (int j = 0; j < NUMCORES; j++)
    {
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            per_thread_tracker[j][k]->length = 0;
        }
    }

    currMTapp = -1;

    // add an argument for refreshing selection
    for (numc = 0; numc < NUMCORES; numc++)
    {
        gzFile *temp = &tif[numc];
        init_trace(argv[numc + 3], temp);

        /* The addresses in each trace are given a prefix that equals
           their core ID.  If the input trace starts with "MT", it is
           assumed to be part of a multi-threaded app.  The addresses
           from this trace file are given a prefix that equals that of
           the last seen input trace file that starts with "MT0".  For
           example, the following is an acceptable set of inputs for
           multi-threaded apps CG (4 threads) and LU (2 threads):
           usimm 1channel.cfg MT0CG MT1CG MT2CG MT3CG MT0LU MT1LU */
        prefixtable[numc] = numc;

        /* Find the start of the filename.  It's after the last "/". */
        for (fnstart = strlen(argv[numc + 3]); fnstart >= 0; fnstart--)
        {
            if (argv[numc + 3][fnstart] == '/')
            {
                break;
            }
        }
        fnstart++; /* fnstart is either the letter after the last / or the 0th letter. */

        if ((strlen(argv[numc + 3]) - fnstart) > 2)
        {
            if ((argv[numc + 3][fnstart + 0] == 'M') && (argv[numc + 3][fnstart + 1] == 'T'))
            {
                if (argv[numc + 3][fnstart + 2] == '0')
                {
                    currMTapp = numc;
                }
                else
                {
                    if (currMTapp < 0)
                    {
                        fprintf(stderr, "Poor set of input parameters.  Input file %s starts with \"MT\", but there is no preceding input file starting with \"MT0\".  Quitting.\n", argv[numc + 3]);
                        return -6;
                    }
                    else
                        prefixtable[numc] = currMTapp;
                }
            }
        }
        printf("Core %d: Input trace file %s : Addresses will have prefix %d\n", numc, argv[numc + 3], prefixtable[numc]);

        committed[numc] = 0;
        fetched[numc] = 0;
        time_done[numc] = 0;
        ROB[numc].head = 0;
        ROB[numc].tail = 0;
        ROB[numc].inflight = 0;
        ROB[numc].tracedone = 0;
    }

    vi_file = fopen("../input/8Gb_x8.vi", "r");
    chips_per_rank = 8;
    printf("Reading vi file: 8Gb_x8.vi\t\n%d Chips per Rank\n", chips_per_rank);

    if (!vi_file)
    {
        fprintf(stderr, "Missing DRAM chip parameter file.  Quitting. \n");
        return -5;
    }

    assert((log_base2(NUM_CHANNELS) + log_base2(NUM_RANKS) + log_base2(NUM_BANKS) + log_base2(NUM_ROWS) + log_base2(NUM_COLUMNS) + log_base2(CACHE_LINE_SIZE)) == ADDRESS_BITS);

    read_config_file(vi_file);
    print_params();

    for (int i = 0; i < NUMCORES; i++)
    {
        ROB[i].entry = (struct node *)calloc(ROBSIZE, sizeof(struct node));
        for(int j = 0; j < ROBSIZE; j++){
            ROB[i].entry[j].next = NULL;
            ROB[i].entry[j].comptime = 0;
        }
        ROB[i].mem_address = (long long int *)calloc(ROBSIZE, sizeof(long long int));
        ROB[i].instrpc = (long long int *)calloc(ROBSIZE, sizeof(long long int));
        ROB[i].optype = (int *)calloc(ROBSIZE, sizeof(int));
        ROB[i].cachemiss = (int *)calloc(ROBSIZE, sizeof(int));
        ROB[i].queue = (int *)calloc(ROBSIZE, sizeof(int));
    }
    // Initialize the inter_arrival_times to 0;
    inter_arrival_time = (long long int *)calloc(NUM_CHANNELS, sizeof(long long int));
    long long int cache_size = CACHE_SIZE * 1024 * 1024; //4MB LLC (SHARED)
    uns assoc = 16;
    uns block_size = 64;
    uns sets = cache_size / (assoc * block_size);
    uns repl = 0;

    L3Cache = (MCache *)calloc(1, sizeof(MCache));
    init_cache(L3Cache, sets, assoc, repl, block_size);
    L3MSHR = (unsigned long long int*)calloc(MSHRSIZE, sizeof(unsigned long long int));
    core_instflight = (unsigned int*)calloc(NUMCORES, sizeof(unsigned int));
    /*rowindirection = (long int *)calloc(NUM_CHANNELS*NUM_RANKS*NUM_BANKS*NUM_ROWS, sizeof(long int));
    
    for(long int rowint = 0; rowint < (NUM_CHANNELS*NUM_RANKS*NUM_BANKS*NUM_ROWS); rowint++){
        rowindirection[rowint] = -1;
    }*/

    os_pages = TOTALOSPAGES;
    init_memory_controller_vars();
    init_scheduler_vars();
    os = os_new(os_pages, OS_PAGESIZE, OS_NUM_RND_TRIES, NUMCORES);
    /* Done initializing. */
    int val = rand()%NUMCORES;

    /* Must start by reading one line of each trace file. */
    for (int corecount = 0; corecount < NUMCORES; corecount++)
    {
        numc = (val + corecount)%NUMCORES;
        if (gzgets(tif[numc], newstr, sizeof(newstr)) != NULL)
        {
            inst_comp++;
            if (sscanf(newstr, "%d %c", &nonmemops[numc], &opertype[numc]) > 0)
            {
                if (opertype[numc] == 'R')
                {
                    if (sscanf(newstr, "%d %c %llx %d %d", &nonmemops[numc], &opertype[numc], &addr[numc], &compressedSize1Line[numc], &compressedSize2Line[numc]) < 1)
                    {
                        fprintf(stderr, "Panic.  Poor trace format 1.\n");
                        return -4;
                    }
                }
                else
                {
                    if (opertype[numc] == 'W')
                    {
                        if (sscanf(newstr, "%d %c %llx %d %d", &nonmemops[numc], &opertype[numc], &addr[numc], &compressedSize1Line[numc], &compressedSize2Line[numc]) < 1)
                        {
                            fprintf(stderr, "Panic.  Poor trace format 2.\n");
                            return -3;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Panic.  Poor trace format 3.\n");
                        return -2;
                    }
                }
            }
            else
            {
                fprintf(stderr, "Panic.  Poor trace format 4.\n");
                return -1;
            }

            if(BM_MODE == 1){
                //Insert the OS here to do a Virtual to Physical Translation since we are using a virtual address trace
                Addr phy_lineaddr = os_v2p_lineaddr(os, addr[numc], numc);
                addr[numc] = phy_lineaddr;
            }
        }
        else
        {
            gzFile *temp = &tif[numc];
            gzclose(tif[numc]);
            init_trace(argv[numc + 3], temp);
        }
    }
    printf("CONFIG PARAM FOR SCHEDULER: %d\n", config_param);    
    printf("Starting simulation.\n");
    while (!expt_done)
    {
        val = rand()%NUMCORES;
        
        /* For each core, retire instructions if they have finished. */
        for(int corecount = 0; corecount < NUMCORES; corecount++)
        {
            numc = (val+corecount)%NUMCORES;
            num_ret = 0;

            while ((num_ret < MAX_RETIRE) && ROB[numc].inflight)
            {
                /* Keep retiring until retire width is consumed or ROB is empty. */
                if (ROB[numc].entry[ROB[numc].head].comptime < CYCLE_VAL)
                {
                    if(CYCLE_VAL >= skipit){
                        if (ROB[numc].optype[ROB[numc].head] == 'R')
                        {
                            loadq[numc]--;
                            assert(loadq[numc] >= 0);
                        }
                        else if (ROB[numc].optype[ROB[numc].head] == 'W')
                        {
                            storeq[numc]--;
                            assert(storeq[numc] >= 0);
                        }
                        /* Keep retiring instructions if they are done. */
                        if(ROB[numc].cachemiss[ROB[numc].head] == 1){
                            ROB[numc].cachemiss[ROB[numc].head] = 0;
                            //mcache_mark_valid(L3Cache, ROB[numc].mem_address[ROB[numc].head]);
                        }
                        if(ROB[numc].entry[ROB[numc].head].next != NULL){
                            ROB[numc].entry[ROB[numc].head].next->comptime = ROB[numc].entry[ROB[numc].head].comptime;
                        }

                        ROB[numc].head = (ROB[numc].head + 1) % ROBSIZE;
                        if(ROB[numc].inflight > 0){
                            ROB[numc].inflight--;
                        }
                        committed[numc]++;
                        num_ret++;
                        core_instflight[numc]++;
                    }
                    else{
                        for(int l = 0; l < ROBSIZE; l++){
                            if(ROB[numc].entry[(ROB[numc].head + l)%ROBSIZE].comptime < CYCLE_VAL){
                                ROB[numc].entry[(ROB[numc].head + l)%ROBSIZE].comptime = skipit + PIPELINEDEPTH;
                            }
                        }
                    }
                }
                else /* Instruction not complete.  Stop retirement for this core. */
                    break;
            } /* End of while loop that is retiring instruction for one core. */
        }     /* End of for loop that is retiring instructions for all cores. */


        if (CYCLE_VAL % PROCESSOR_CLK_MULTIPLIER == 0)
        {
            unsigned long long int stallvalue = 0;
            /* Execute function to find ready instructions. */
            update_memory(NUMCORES);

            /* Execute user-provided function to select ready instructions for issue. */
            /* Based on this selection, update DRAM data structures and set
               instruction completion times. */
            if(config_param == 1){ // fr-fcfs 
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_frfcfs(c);
                    gather_stats(c);
                }
            }
            else if(config_param == 2){ //fcfs
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_fcfs(c);
                    gather_stats(c);
                }
                /*if((MITIGATION_MODE == 4) || (MITIGATION_MODE == 5)){
                    if(CYCLE_VAL%204800000 == 0){
                        stallvalue = 0;
                        for (int c = 0; c < NUM_CHANNELS; c++)
                        {
                            for (int r = 0; r < NUM_RANKS; r++)
                            {
                                if(prob_epoch_tracker_temp[c][r] > stallvalue)
                                {
                                    stallvalue = prob_epoch_tracker_temp[c][r];
                                }
                            }
                        }
                        for (int c = 0; c < NUM_CHANNELS; c++)
                        {
                            for (int r = 0; r < NUM_RANKS; r++)
                            {
                                prob_epoch_tracker_temp[c][r] = stallvalue;
                                refreshdone[c][r] = 1;
                            }
                        }
                        stallvalue = 0;
                        for (int c = 0; c < NUM_CHANNELS; c++)
                        {
                            if(mitigationapply[c] > stallvalue)
                            {
                                stallvalue = mitigationapply[c];
                            }
                        }
                        for (int c = 0; c < NUM_CHANNELS; c++)
                        {
                            mitigationapply[c] = stallvalue;
                        }
                    }

                }*/
                /*if((MITIGATION_MODE == 4) || (MITIGATION_MODE == 5)){
                    stallvalue = 0;
                    for (int c = 0; c < NUM_CHANNELS; c++)
                    {
                        if(mitigationapply[c] > stallvalue)
                        {
                            stallvalue = mitigationapply[c];
                        }
                    }
                    for (int c = 0; c < NUM_CHANNELS; c++)
                    {
                        mitigationapply[c] = stallvalue;
                    }
                }*/
            }
            else if(config_param == 3){ //closepage
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_closepage(c);
                    gather_stats(c);
                }
            }
            else if(config_param == 4){ //bta -- not working
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_bta(c);
                    gather_stats(c);
                }
            }
            else if(config_param == 5){ //plumber-r
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_pr(c);
                    gather_stats(c);
                }
            }
            else if(config_param == 6){ //fixed-service
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    if ((per_thread_queue_length[runcore[c]][c] == 0)){
                        ran_addr = generate_random_addr(c);
                        assert(check_dram_addr(ran_addr, c) == 0);
                        fakereadreq++;
                        rd_ptr = insert_read(ran_addr, CYCLE_VAL, runcore[c], 0 , 0, request_id_counter, 0, 1, 0);
                        insert_requestID(ran_addr, runcore[c], request_id_counter, 1, rd_ptr, 0);
                        request_id_counter++;
                    }
                    schedule_fs(c, runcore);
                    gather_stats(c);
                }
            }
           else if(config_param == 9){ //lps
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_lps(c);
                    gather_stats(c);
                }
            }
            else if(config_param == 10){ //TP
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_tp(c, runcore);
                    gather_stats(c);
                }
            }
            else if(config_param == 11){ //Camouflage
                for (int c = 0; c < NUM_CHANNELS; c++)
                {
                    schedule_camou(c);
                    gather_stats(c);
                }
            }
            else{
                printf("Incorrect parameter. Should be less than 8!\n");
                return -3;
            }
        }

        /* For each core, bring in new instructions from the trace file to
           fill up the ROB. */
        num_done = 0;
        writeqfull = 0;
        for (int c = 0; c < NUM_CHANNELS; c++)
        {
            if(write_queue_length[c] >= WQ_CAPACITY)
            {
                writeqfull = 1;
            }
        }
        readqfull = 0;
        for (int c = 0; c < NUM_CHANNELS; c++)
        {
            if(read_queue_length[c] >= RQ_CAPACITY)
            {
                readqfull = 1;
            }
        }
        stallnow = 0;
        for (int corecount = 0; corecount < NUMCORES; corecount++)
        {
            numc = (val+corecount)%NUMCORES;
            if (!ROB[numc].tracedone)
            { /* Try to fetch if EOF has not been encountered. */
                num_fetch = 0;
                while ((num_fetch < MAX_FETCH) && (ROB[numc].inflight < ROBSIZE) && (!writeqfull) && (!readqfull))// && (CYCLE_VAL >= skipit))// && ((CYCLE_VAL - stallnow) >=CACHEDELAY))
                {
                    /* Keep fetching until fetch width or ROB capacity or WriteQ are fully consumed. */
                    /* Read the corresponding trace file and populate the tail of the ROB data structure. */
                    /* If Memop, then populate read/write queue.  Set up completion time. */
                    if (nonmemops[numc] > 0)
                    { /* Have some non-memory-ops to consume. */
                        ROB[numc].optype[ROB[numc].tail] = 'N';
                        ROB[numc].cachemiss[ROB[numc].tail] = 0;
                        ROB[numc].queue[ROB[numc].tail] = 0;
                        ROB[numc].entry[ROB[numc].tail].comptime = CYCLE_VAL + PIPELINEDEPTH;
                        ROB[numc].entry[ROB[numc].tail].next = NULL;
                        nonmemops[numc]--;
                        ROB[numc].tail = (ROB[numc].tail + 1) % ROBSIZE;
                        ROB[numc].inflight++;
                        fetched[numc]++;
                        temp_inst_cntr++;
                        num_fetch++;
                        totalinst++;
                    }
                    else
                    {
                        /* Done consuming non-memory-ops.  Must now consume the memory rd or wr. */
                        if (opertype[numc] == 'R')
                        {
                            loadq[numc]++;
                            int L3Hit = 0; 
                            ROB[numc].mem_address[ROB[numc].tail] = addr[numc];
                            ROB[numc].optype[ROB[numc].tail] = opertype[numc];
                            ROB[numc].entry[ROB[numc].tail].comptime = CYCLE_VAL + BIGNUM;
                            ROB[numc].entry[ROB[numc].tail].next = NULL;
                            ROB[numc].instrpc[ROB[numc].tail] = instrpc[numc];
                            ROB[numc].cachemiss[ROB[numc].tail] = 0;
                            ROB[numc].queue[ROB[numc].tail] = 0;
                            long long int wb_addr = 0;
                            MCache_Entry victim;
                            //if((addr[numc] << (L3Cache->lineoffset)) < ((TOTALOSPAGES*OS_PAGESIZE) - ((TOTALOSPAGES*OS_PAGESIZE)>>5))){
                                L3Hit = isHit(L3Cache, addr[numc], false); //addr[numc] is cl address
                            //}
                            //else{
                                //L3Hit =  2;
                            //}


                            if (L3Hit == 0)
                            {
                                victim = install(L3Cache, addr[numc], instrpc[numc], false);
                                ROB[numc].cachemiss[ROB[numc].tail] = 1;
                            }

                            // Check to see if the read is for buffered data in write queue -
                            // return constant latency if match in WQ
                            // add in read queue otherwise
                            int lat = read_matches_write_or_read_queue(addr[numc] << (L3Cache->lineoffset));

                            if (L3Hit == 1)
                            {
                                ROB[numc].entry[ROB[numc].tail].comptime = CYCLE_VAL + L3_LATENCY + PIPELINEDEPTH;
                                stallnow++;
                            }
                            else if(L3Hit == 0)
                            {

                                if (victim.dirty && victim.valid)
                                {
                                    wb_addr = victim.tag << (L3Cache->lineoffset);
                                    if (!write_exists_in_write_queue(wb_addr))
                                    {
                                        insert_write(wb_addr, CYCLE_VAL, numc, ROB[numc].tail, 0);
                                    }

                                    for (int c = 0; c < NUM_CHANNELS; c++)
                                    {
                                        if(write_queue_length[c] >= WQ_CAPACITY)
                                        {
                                            writeqfull = 1;
                                        }
                                    }
                                }

                                // Check to see if the read is for buffered data in write queue -
                                // return constant latency if match in WQ
                                // add in read queue otherwise

                                if (lat == WQ_LOOKUP_LATENCY)
                                {
                                    ROB[numc].entry[ROB[numc].tail].comptime = CYCLE_VAL + lat + PIPELINEDEPTH;
                                }
                                else if(lat == 1){
                                    stallnow++;
                                    for(int jj = 0; jj < ROBSIZE; jj++){
                                        if(ROB[numc].optype[jj] == 'R'){
                                            if(ROB[numc].tail != jj){
                                                if(ROB[numc].mem_address[ROB[numc].tail] == ROB[numc].mem_address[jj]){
                                                    if(ROB[numc].entry[jj].next == NULL){
                                                        ROB[numc].entry[jj].next = &(ROB[numc].entry[ROB[numc].tail]);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    int temp_channel = get_dram_channel_addr(addr[numc] << (L3Cache->lineoffset));
                                    rd_ptr = insert_read(addr[numc] << (L3Cache->lineoffset), CYCLE_VAL, numc, ROB[numc].tail, instrpc[numc], request_id_counter, CYCLE_VAL - inter_arrival_time[temp_channel], 0, 0);
                                    inter_arrival_time[temp_channel] = CYCLE_VAL;
                                    if(config_param > 5){
                                        insert_requestID(addr[numc] << (L3Cache->lineoffset), numc, request_id_counter, 0, rd_ptr, 0);
                                    }
                                    request_id_counter++;
                                }
                            }
                            else{
                                int temp_channel = get_dram_channel_addr(addr[numc] << (L3Cache->lineoffset));
                                rd_ptr = insert_read(addr[numc] << (L3Cache->lineoffset), CYCLE_VAL, numc, ROB[numc].tail, instrpc[numc], request_id_counter, CYCLE_VAL - inter_arrival_time[temp_channel], 0, 0);
                                inter_arrival_time[temp_channel] = CYCLE_VAL;
                                if(config_param > 5){
                                    insert_requestID(addr[numc] << (L3Cache->lineoffset), numc, request_id_counter, 0, rd_ptr, 0);
                                }
                                request_id_counter++;
 
                            }
                            for (int c = 0; c < NUM_CHANNELS; c++)
                            {
                                if (read_queue_length[c] >= RQ_CAPACITY)
                                {
                                    readqfull = 1;
                                }
                            }
                        }
                        else
                        { /* This must be a 'W'.  We are confirming that while reading the trace. */
                            if (opertype[numc] == 'W')
                            {
                                int L3Hit = 0;
                                storeq[numc]++;
                                ROB[numc].mem_address[ROB[numc].tail] = addr[numc];
                                ROB[numc].optype[ROB[numc].tail] = opertype[numc];
                                ROB[numc].entry[ROB[numc].tail].comptime = CYCLE_VAL + PIPELINEDEPTH;
                                ROB[numc].entry[ROB[numc].tail].next = NULL;
                                ROB[numc].cachemiss[ROB[numc].tail] = 0;
                                ROB[numc].queue[ROB[numc].tail] = 0;
                                /* Also, add this to the write queue. */
                                long long int wb_addr = 0;
                                MCache_Entry victim;

                                //if((addr[numc] << (L3Cache->lineoffset)) < ((TOTALOSPAGES*OS_PAGESIZE) - ((TOTALOSPAGES*OS_PAGESIZE)>>5))){
                                    L3Hit = isHit(L3Cache, addr[numc], true); //addr[numc] is cl address
                                //}
                                //else{
                                    //L3Hit = 2;
                                //}

                                if (L3Hit == 0)
                                {
                                    victim = install(L3Cache, addr[numc], instrpc[numc], true);
                                    if (victim.dirty && victim.valid)
                                    {
                                        wb_addr = victim.tag << (L3Cache->lineoffset);
                                        if (!write_exists_in_write_queue(wb_addr))
                                        {
                                            insert_write(wb_addr, CYCLE_VAL, numc, ROB[numc].tail, 0);
                                        }
                                    }
                                }
                                else if(L3Hit == 1) {
                                    stallnow++;
                                }
                                else{
                                    wb_addr = addr[numc] << (L3Cache->lineoffset);
                                    if (!write_exists_in_write_queue(wb_addr))
                                    {
                                        insert_write(wb_addr, CYCLE_VAL, numc, ROB[numc].tail, 0);
                                    } 
                                }

                                for (int c = 0; c < NUM_CHANNELS; c++)
                                {
                                    if(write_queue_length[c] >= WQ_CAPACITY)
                                    {
                                        writeqfull = 1;
                                    }
                                }
                            }
                            else
                            {
                                fprintf(stderr, "Panic.  Poor trace format 5. \n");
                                return -1;
                            }
                        }
                        ROB[numc].tail = (ROB[numc].tail + 1) % ROBSIZE;
                        ROB[numc].inflight++;
                        fetched[numc]++;
                        temp_inst_cntr++;
                        num_fetch++;
                        totalinst++;

                        /* Done consuming one line of the trace file.  Read in the next. */
                        if (gzgets(tif[numc], newstr, sizeof(newstr)) != NULL)
                        {
                            inst_comp++;
                            if (sscanf(newstr, "%d %c", &nonmemops[numc], &opertype[numc]) > 0)
                            {
                                if (opertype[numc] == 'R')
                                {
                                    if (sscanf(newstr, "%d %c %llx %d %d", &nonmemops[numc], &opertype[numc], &addr[numc], &compressedSize1Line[numc], &compressedSize2Line[numc]) < 1)
                                    {
                                        fprintf(stderr, "Panic.  Poor trace format 6.\n");
                                        return -4;
                                    }
                                }
                                else
                                {
                                    if (opertype[numc] == 'W')
                                    {
                                        if (sscanf(newstr, "%d %c %llx %d %d", &nonmemops[numc], &opertype[numc], &addr[numc], &compressedSize1Line[numc], &compressedSize2Line[numc]) < 1)
                                        {
                                            fprintf(stderr, "Panic.  Poor trace format 7.\n");
                                            return -3;
                                        }
                                    }
                                    else
                                    {
                                        fprintf(stderr, "Panic.  Poor trace format 8.\n");
                                        return -2;
                                    }
                                }
                            }
                            else
                            {
                                fprintf(stderr, "Panic.  Poor trace format 9.\n");
                                return -1;
                            }
                            if(BM_MODE == 1){
                                Addr phy_lineaddr = os_v2p_lineaddr(os, addr[numc], numc);
                                addr[numc] = phy_lineaddr;
                            }

                        }
                        else
                        {
                            gzFile *temp = &tif[numc];
                            gzclose(tif[numc]);
                            init_trace(argv[numc + 3], temp);
                        }

                    }
                    /* Done consuming the next rd or wr. */
                } /* One iteration of the fetch while loop done. */
                if (committed[numc] >= (INST_MILLION * 1000000))
                {
                    ROB[numc].tracedone = 1;
                }
            } /* Closing brace for if(trace not done). */
            else
            { /* Input trace is done.  Check to see if all inflight instrs have finished. */
                if (ROB[numc].inflight == 0)
                {
                    num_done++;
                    if (!time_done[numc])
                        time_done[numc] = CYCLE_VAL;
                }
            }
        } /* End of for loop that goes through all cores. */

        if (num_done == NUMCORES)
        {
            /* Traces have been consumed and in-flight windows are empty.  Must confirm that write queues have been drained. */
            numch = NUM_CHANNELS;
            if (numch == NUM_CHANNELS)
            {
                fflush( stdout );
                printf("expt_done!!\n");
                expt_done = 1; /* All traces have been consumed and the write queues are drained. */
            }
        }

        CYCLE_VAL++; /* Advance the simulation cycle. */
        if (temp_inst_cntr > 10000000)
        {
            temp_inst_cntr2++;
            temp_inst_cntr = 0;
            fflush(stdout);
            printf(".");

            if (temp_inst_cntr2 % 10 == 0)
            {
                fflush( stdout );
                printf(" - %lld00 Million Instructions -- %lf\n", temp_inst_cntr2 / 10, (double)totalinst/CYCLE_VAL);
            }
        }
    }
    fflush(stdout);
    printf("sim exit!\n");

    /* Code to make sure that the write queue drain time is included in
       the execution time of the thread that finishes last. */
    maxtd = time_done[0];
    maxcr = 0;
    for (numc = 1; numc < NUMCORES; numc++)
    {
        if (time_done[numc] > maxtd)
        {
            maxtd = time_done[numc];
            maxcr = numc;
        }
    }
    time_done[maxcr] = CYCLE_VAL;

    core_power = 0;
    for (numc = 0; numc < NUMCORES; numc++)
    {
        /* A core has peak power of 10 W in a 4-channel config.  Peak power is consumed while the thread is running, else the core is perfectly power gated. */
        core_power = core_power + (10 * ((float)time_done[numc] / (float)CYCLE_VAL));
    }
    if (NUM_CHANNELS == 1)
    {
        /* The core is more energy-efficient in our single-channel configuration. */
        core_power = core_power / 2.0;
    }
    fflush(stdout);
    fflush(stdout);
    fflush(stdout);
    printf("Done with loop. Printing stats.\n");
    printf("Cycles %lld\n", CYCLE_VAL);
    total_time_done = 0;

    for (numc = 0; numc < NUMCORES; numc++)
    {
        printf("Done: Core %d: Fetched %lld : Committed %lld : At time : %lld\n", numc, fetched[numc], committed[numc], time_done[numc]);
        total_time_done += time_done[numc];
        total_inst_fetched = total_inst_fetched + fetched[numc];
        if(time_done[numc] > MIN_USIMM_CYCLES)
            MIN_USIMM_CYCLES = time_done[numc];
        if(fetched[numc] > MIN_FETCH_INST)
            MIN_FETCH_INST = fetched[numc];

    }
    printf("\n\n---------------------------------\n");
    printf("ADDED_USIMM_CYCLES \t : %lld\n", total_time_done);
    printf("ADDED_USIMM_INST   \t : %lld\n", total_inst_fetched);
    ADDED_IPC = (double)(total_inst_fetched)/(double)total_time_done;
    printf("ADDED_IPC	       \t : %f\n", ADDED_IPC);
    printf("---------------------------------\n\n");

    printf("PREFETCH_HIT %llu\n", prefetch_hit);

    for(numc = 0; numc < NUMCORES; numc++){
        printf("*****************************\n");
        printf("CYCLE_CORE%d        \t : %lld\n", numc, time_done[numc]);
        printf("INST_CORE%d         \t : %lld\n", numc, fetched[numc]);
        ipccore[numc] = (double)fetched[numc]/(double)(time_done[numc]);
        printf("IPC_CORE%d	    \t : %f\n", numc, ipccore[numc]);
        printf("*****************************\n\n");
    }

    printf("Fake number is %lld\n", fake_num);

    printf("\n\n+++++++++++++++++++++++++++++++++\n");
    printf("MIN_USIMM_CYCLES \t : %llu\n", MIN_USIMM_CYCLES);
    printf("MIN_FETCH_INST   \t : %llu\n", MIN_FETCH_INST);
    printf("MIN_IPI          \t : %f\n", (double)(MIN_FETCH_INST)/(double)MIN_USIMM_CYCLES);
    printf("+++++++++++++++++++++++++++++++++\n\n");

    printf("TOTAL_MCPI	   \t : %f\n", (double)(queuing_delay_total)/(double)(readreq_total));

    for(numc = 0; numc < NUMCORES; numc++){
        printf("################################\n");
        printf("READREQ_CORE%d	      \t : %llu\n", numc, readreq[numc]);
        printf("MCPI_CORE%d	      \t : %f\n", numc, (double)(queuing_delay[numc])/(double)(readreq[numc]));
        printf("################################\n\n");
    }

    printf("TOTAL_FAKE_REQ	      	\t : %llu\n", fakereadreq);
    printf("TOTAL_READ_REQ	      	\t : %llu\n", readreq_total);
    printf("TOTAL_FAKE_READREQ_RATIO    \t : %f\n", (double)(fakereadreq)/(double)readreq_total);
    printf("SANDBOX \t : %f\n", ((double)sbavg)/te); 
    printf("\nUSIMM_ROBN_STALLS     \t : %lld\n", robn_stalls);
    printf("\nUSIMM_WRQF_STALLS     \t : %lld\n", wrqf_stalls);
    printf("\nUSIMM_WRQN_STALLS     \t : %lld\n", wrqn_stalls);
    printf("Num_Reads_Merged: %lld\n", num_read_merge);
    printf("Num_Writes_Merged: %lld\n", num_write_merge);
    fflush(stdout);
    /* Print all other memory system stats. */
    scheduler_stats();
    print_cache_stats(L3Cache,total_inst_fetched);
    print_stats();
    os_print_stats(os, OS_PAGESIZE);
    fflush(stdout);
    float total_system_power = 0;
    for (int stats = 0; stats < 2; stats++){
        total_system_power += calculate_power(stats, chips_per_rank);
    }

    printf("\n#-------------------------------------------------------------------------------------------------\n");
    if (NUM_CHANNELS == 4)
    { /* Assuming that this is 4channel.cfg  */
        printf("Total_memory_system_power : %f W\n", total_system_power / 1000);
        printf("Miscellaneous_system_power : 40 W  # Processor uncore power, disk, I/O, cooling, etc.\n");
        printf("Processor_core_power : %f W  # Assuming that each core consumes 10 W when running\n", core_power);
        printf("Total_system_power : %f W # Sum of the previous three lines\n", 40 + core_power + total_system_power / 1000);
        printf("EDP : %2.9f J.s\n", (40 + core_power + total_system_power / 1000) * (float)((double)CYCLE_VAL / (double)3200000000) * (float)((double)CYCLE_VAL / (double)3200000000));
    }
    else
    { /* Assuming that this is 1channel.cfg  */
        printf("Total_memory_system_power : %f W\n", total_system_power / 1000);
        printf("Miscellaneous_system_power : 10 W  # Processor uncore power, disk, I/O, cooling, etc.\n"); /* The total 40 W misc power will be split across 4 channels, only 1 of which is being considered in the 1-channel experiment. */
        printf("Processor_core_power : %f W  # Assuming that each core consumes 5 W\n", core_power);       /* Assuming that the cores are more lightweight. */
        printf("Total_system_power : %f W # Sum of the previous three lines\n", 10 + core_power + total_system_power / 1000);
        printf("EDP : %2.9f J.s\n", (10 + core_power + total_system_power / 1000) * (float)((double)CYCLE_VAL / (double)3200000000) * (float)((double)CYCLE_VAL / (double)3200000000));
    }
    fflush(stdout);
    for (int i = 0; i < NUMCORES; i++)
    {
        gzclose(tif[i]);
    }
    for (int j = 0; j < NUMCORES; j++)
    {
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            free(addr_buffer[j][k]);
        }
        free(addr_buffer[j]);
    }
    for (int j = 0; j < NUMCORES; j++)
    {
        for (int k = 0; k < NUM_CHANNELS; k++)
        {
            free(prefetch_buffer[j][k]);
        }
        free(prefetch_buffer[j]);
    }
    for (int g = 0; g < NUMCORES; g++)
    {
        free(ROB[g].entry);
        free(ROB[g].mem_address);
        free(ROB[g].instrpc);
        free(ROB[g].optype);
    }
    for (int j = 0; j < NUMCORES; j++)
    {
        free(num_issue[j]);
    }
    for (int j = 0; j < NUMCORES; j++)
    {
        free(prev_addr[j]);
    }
    for (int j = 0; j < NUMCORES; j++)
    {
        free(qlength[j]);
    }
    free(runcore);
    free(compressedSize1Line);
    free(compressedSize2Line);
    free(tif);
    free(committed);
    free(fetched);
    free(time_done);
    free(nonmemops);
    free(opertype);
    free(addr);
    free(instrpc);
    free(prefixtable);
    free(ROB);
    free(L3Cache->entries);
    free(L3Cache->fifo_ptr);
    free(L3Cache->is_leader_p1);
    free(L3Cache->is_leader_p0);
    free(addr_buffer);
    free(prefetch_buffer);
    free(offset_buffer);
    free(num_issue);
    free(prev_addr);
    free(L3Cache);
    free(inter_arrival_time);
    free(qlength);
    free(ipccore);
    free(timer);
    free(readreq);
    free(queuing_delay);
    //free(os->pt->entries);
    free(loadq);
    free(storeq);
    free(fetch_addr);
    return 0;
}

void stream_prefetcher(unsigned long long int reference_addr, unsigned long long int *prefetch_addr)
{
    //printf("Reference address is %llu\n", reference_addr);
    unsigned long long int temp_addr_top = reference_addr >> 7;
    unsigned long long int temp_addr_bottom = reference_addr & 0x7F;
    temp_addr_top = temp_addr_top << 7;

    for(int k=0; k < 3; k++){
        temp_addr_bottom = (temp_addr_bottom + 1) % 64;
        prefetch_addr[k] = temp_addr_top | temp_addr_bottom;
        //        printf("prefetched address is %llu k is %d\n", prefetch_addr[k], k);
    }
}

void bo_prefetcher(unsigned long long int reference_addr, unsigned long long int *prefetch_addr, unsigned long long int ***addr_buffer, int numcores, int num_channels, int addr_buffer_size)
{
    if(init_offset_buffer == 0){
     for (int i = 0; i < NUMCORES; i++)
    {  
      for (int j = 0; j < NUM_CHANNELS; j++)
      {
        for(int num_block=1; num_block<64; num_block++)
        {
            offset_buffer[i][j][num_block-1].offset = num_block;
            offset_buffer[i][j][num_block-1].score = 0;
        }
        init_offset_buffer = 1;
      }
    }
    }

    unsigned long long int temp_top = reference_addr >> 6;
    unsigned long long int temp_addr = (reference_addr & 0x3F);
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < addr_buffer_size; j++){
            if(((temp_top << 6) | (((temp_addr)-(offset_buffer[numcores][num_channels][i].offset)) % 64)) == addr_buffer[numcores][num_channels][j])
            {
                offset_buffer[numcores][num_channels][i].score++;
                //printf("Score incremented! score is %d i is %d\n", offset_buffer[i].score, i);
            }
        }
    }

    num_round++;

    unsigned int best_score = offset_buffer[numcores][num_channels][0].score;
    unsigned int second_score = offset_buffer[numcores][num_channels][0].score;
    unsigned int third_score = offset_buffer[numcores][num_channels][0].score;
    unsigned int best_score_fetch = offset_buffer[numcores][num_channels][0].offset;
    unsigned int second_score_fetch = offset_buffer[numcores][num_channels][0].offset;
    unsigned int third_score_fetch = offset_buffer[numcores][num_channels][0].offset;
    for(int k=1;k<64;k++){
        if(offset_buffer[numcores][num_channels][k-1].score > best_score){
            second_score = best_score;
            best_score = offset_buffer[numcores][num_channels][k-1].score;
            best_score_fetch = offset_buffer[numcores][num_channels][k-1].offset;
        } else if(offset_buffer[numcores][num_channels][k-1].score > second_score && offset_buffer[numcores][num_channels][k-1].score != best_score){
            third_score = second_score;
            best_score = offset_buffer[numcores][num_channels][k-1].score;
            second_score_fetch = offset_buffer[numcores][num_channels][k-1].offset;
        } else if(offset_buffer[numcores][num_channels][k-1].score > third_score && offset_buffer[numcores][num_channels][k-1].score != best_score && offset_buffer[numcores][num_channels][k-1].score != second_score){
            third_score = offset_buffer[numcores][num_channels][k-1].score;
            third_score_fetch = offset_buffer[numcores][num_channels][k-1].offset;
        }

    }


    if(num_round == ROUND_MAX)
    {
        num_round = 0;
        init_offset_buffer = 0;

        local_best_score_fetch = best_score_fetch;
        local_second_score_fetch = second_score_fetch;
        local_third_score_fetch = third_score_fetch;

        /*  for(int num=1; num<64; num++)
            {
            printf("offset %d score is %d\n", num, offset_buffer[num-1].score);
            }*/
    }

    //printf("best offset is %d\n", local_best_score_fetch);
    //printf("second offset is %d\n", local_second_score_fetch);
    //printf("third offset is %d\n", local_third_score_fetch);

    unsigned long long int temp_addr_top = reference_addr >> 6;
    unsigned long long int temp_addr_bottom = reference_addr & 0x3F;
    temp_addr_top = temp_addr_top << 6; 

    temp_addr_bottom = (temp_addr_bottom + local_best_score_fetch) % 64;
    prefetch_addr[0] = temp_addr_top | temp_addr_bottom;
    //printf("first prefetched address is %llu\n", prefetch_addr[0]);

    temp_addr_bottom = (temp_addr_bottom + local_second_score_fetch) % 64;
    prefetch_addr[1] = temp_addr_top | temp_addr_bottom; 
    //printf("second prefetched address is %llu\n", prefetch_addr[1]);

    temp_addr_bottom = (temp_addr_bottom + local_third_score_fetch) % 64;
    prefetch_addr[2] = temp_addr_top | temp_addr_bottom;
    //printf("third prefetched address is %llu\n", prefetch_addr[2]);

}


int request_in_prefetch_buffer(long long int addr, unsigned long long int ***prefetch_buffer, int numcores, int num_channels, int prefetch_buffer_size)
{
    for(int i = 0; i < numcores; i++){
        for(int j = 0; j < num_channels; j++){
            for( int k = 0; k < prefetch_buffer_size; k++){
                if (addr == prefetch_buffer[i][j][k])
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void init_trace(char *addr_trace_fname, gzFile *addr_trace)
{
    if ((*addr_trace = gzopen(addr_trace_fname, "r")) == NULL)
    {
        printf("Unable to open the file with gzip option \n");
    }
}

EE extended_euclid(long long int a, long long int b) {
    EE ee1, ee2, ee3;
    if (b == 0) {
        ee1.d = a;
        ee1.x = 1;
        ee1.y = 0;
        return ee1;
    } else {
        ee2 = extended_euclid(b, a % b);
        ee3.d = ee2.d;
        ee3.x = ee2.y;
        ee3.y = ee2.x - floor(a / b) * ee2.y;
        return ee3;
    }
}

long long int modulo(long long int x, long long int N){
    return (x % N + N) % N;
}

void decimal_to_binary(long long int op1, long long int aOp[]){
    long long int result, i = 0;
    do{
        result = op1 % 2;
        op1 /= 2;
        aOp[i] = result;
        i++;
    }while(op1 > 0);
}

long long int modular_exponentiation(long long int a, long long int b, long long int n){
    long long int *bb;
    long long int count = 0, c = 0, d = 1, i;
    
    // find out the size of binary b
    count = (long long int) (log(b)/log(2)) + 1;
    
    bb = (long long int *) malloc(sizeof(long long int*) * count);
    decimal_to_binary(b, bb);
    
    for (i = count - 1; i >= 0; i--) {
        c = 2 * c;
        d = (d*d) % n;
        if (bb[i] == 1) {
            c = c + 1;
            d = (d*a) % n;
        }
    }
    return d;
}

long long int randomizer(long long int physical_addr)
{
    long long int n, rand_addr;
    n = p*q;
    rand_addr = modular_exponentiation(physical_addr, e, n);

    return rand_addr;
}

int random_number(int min_num, int max_num)
    {
        int result = 0, low_num = 0, hi_num = 0;

        if (min_num < max_num)
        {
            low_num = min_num;
            hi_num = max_num + 1; // include max_num in output
        } else {
            low_num = max_num + 1; // include max_num in output
            hi_num = min_num;
        }

        result = (rand() % (hi_num - low_num)) + low_num;
        return result;
    }

    
