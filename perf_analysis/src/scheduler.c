#include <stdio.h>
#include "utlist.h"
#include "utils.h"
#include <stdlib.h>
#include "memory_controller.h"
#include "scheduler.h"
#include <time.h>
#include "params.h"
#include "cache.h"

//Common Variable
extern long long int CYCLE_VAL;
extern int NUMCORES;
unsigned long long int delay_stall[MAX_NUM_CHANNELS][MAX_NUM_RANKS];

long int count_col_hits[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
/* A data structure to see if a bank is a candidate for precharge. */
int recent_colacc[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];

unsigned long long int bank_count[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
int curr_bank;
// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHANNELS];
int wait_time[MAX_NUM_CHANNELS];
unsigned int issue_req[MAX_NUM_CHANNELS];
unsigned long long int sbm1;
unsigned long long int tem1;

unsigned long long int prefetch_counter = 0;

unsigned long long int time_diff[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS][9]; //These many commands exist
unsigned long long int channel_time_diff[MAX_NUM_CHANNELS];
int prev_core[MAX_NUM_CHANNELS];
unsigned long long int current_count[MAX_NUM_CHANNELS];
unsigned long long int current_core[MAX_NUM_CHANNELS];
unsigned long long int perthread_timer_start[MAX_NUM_CHANNELS];
unsigned long long int perthread_timer_end[MAX_NUM_CHANNELS];
unsigned long long int bin_active[MAX_NUM_CHANNELS][10];
unsigned long long int bin_fake[MAX_NUM_CHANNELS][10];
unsigned long long int epoch_start_timer[MAX_NUM_CHANNELS];
int prev_bank[4][MAX_NUM_CHANNELS][MAX_NUM_RANKS];
unsigned long long int b_counter[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
unsigned long long int prob_tracker[MAX_NUM_CHANNELS];
unsigned long long int prob_epoch_tracker[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
unsigned long long int refreshdone[MAX_NUM_CHANNELS][MAX_NUM_RANKS];


unsigned long long int * ract_real[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
unsigned long long int * ract_temp[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
unsigned long long int * stall[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
unsigned int *actcnt;
unsigned long long int ccaccess;
unsigned long long int cchit;
unsigned int *cra_filter;
unsigned long long int filteraccesses;
unsigned long long int filterhits;

unsigned long long int spareoccupancy;

unsigned long long int risi;
unsigned long long int risv;
unsigned long long int rvsi;
unsigned long long int rvsv;
unsigned int mitigationdone[MAX_NUM_CHANNELS];


unsigned long long int totalresetcc;

#define max(a, b) (((a) > (b)) ? (a) : (b))

//Scheduler Variables
void init_scheduler_vars ()
{
    int i, j, k;
    unsigned long long int rth_actual = TRH;
    rth_actual = rth_actual/SWITCH_RATE; // Divide this by Switching Rate --> Swap Rate or Move Rate
    mg_entries = 1360000/rth_actual;

    ccaccess = 0;
    cchit = 0;
    filteraccesses = 0;
    filterhits = 0;
    totalresetcc = 0;
    spareoccupancy = 0;
    risi = 0;
    risv = 0;
    rvsi = 0;
    rvsv = 0;

    // initialize all scheduler variables here
    long long int cache_size = CCSIZE * 1024; //KB
    uns assoc = CCASSOC;
    uns block_size = CCBLOCK;
    uns sets = cache_size / (assoc * block_size);
    uns repl = CCREPL;
    // initialize all scheduler variables here
    
    //Initialize the Counter Cache Per Channel
    CCache = (MCache *)calloc(1, sizeof(MCache));
    init_cache(CCache, sets, assoc, repl, block_size);
    CCache->index_policy = CRA_ENABLE_RAI;

    if(CRA_ENABLE_RAI){
        uns cc_lines_in_rbuf = (NUM_COLUMNS*64)/CCBLOCK;
        uns rai_shift_bits = log_base2(cc_lines_in_rbuf);
        CCache->index_policy = rai_shift_bits; 
    }
    unsigned long long int totalrows = NUM_CHANNELS*NUM_RANKS*NUM_BANKS*NUM_ROWS;
    actcnt = (unsigned int*)calloc(totalrows, sizeof(unsigned int));
    
    if(CRA_FILTER_ENTRIES){
        cra_filter = (unsigned int*)calloc(CRA_FILTER_ENTRIES, sizeof(unsigned int));
    }

    // initialize all scheduler variables here
    uniquecount = 0;
    for (int i = 0; i < MAX_NUM_CHANNELS; i++){
        prob_tracker[i] = 0;
        mitigationapply[i] = 0;
        cachequeue[i] = 0;
        mitigationdone[i] = 0;
 
        for (int j = 0; j < MAX_NUM_RANKS; j++){
            prob_epoch_tracker[i][j] = 0;
            prob_epoch_tracker_temp[i][j] = 0;
            refreshdone[i][j] = 0;
            delay_stall[i][j] = 0;
            for (int k = 0; k < MAX_NUM_BANKS; k++){
                count_col_hits[i][j][k] = 0;
                bank_count[i][j][k] = 0;
                b_counter[i][j][k] = 0;
                unique[i][j][k] = (int*)calloc(NUM_ROWS, sizeof(int));
                ract_real[i][j][k] = (unsigned long long int*)calloc(NUM_ROWS, sizeof(unsigned long long int));
                ract_temp[i][j][k] = (unsigned long long int*)calloc(NUM_ROWS, sizeof(unsigned long long int));
                stall[i][j][k] = (unsigned long long int*)calloc(NUM_ROWS, sizeof(unsigned long long int)); 
            }
        }
        new_sandbox_mode1[i] = (sandbox_t *)malloc(sizeof(sandbox_t));
        new_sandbox_mode1[i]->mode = 0;
        new_sandbox_mode1[i]->close_page = 0;
        drain_writes[i] = 0;
        wait_time[i] = 0;
        issue_req[i] = 0;
        channel_time_diff[i] = 0;
        prev_core[i] = 0;
        current_core[i] = 0;
        perthread_timer_start[i] = 0;
        perthread_timer_end[i] = 0;
        epoch_start_timer[i] = 0;
    }
    for (i=0; i<MAX_NUM_CHANNELS; i++) {
        for (j=0; j<MAX_NUM_RANKS; j++) {
            for (k=0; k<MAX_NUM_BANKS; k++) {
                recent_colacc[i][j][k] = 0;
                for(int command_type = 0; command_type < 9; command_type++){
                    time_diff[i][j][k][command_type] = 0;
                }
            }
        }
    }
    for (i=0; i<MAX_NUM_CHANNELS; i++) {
        for(int k = 0; k < 10; k++)
        {
            bin_active[i][k] = 0;
            bin_fake[i][k] = 0;
        }
    }

    sbm1 = 0;
    tem1 = 0;
    if(MITIGATION_MODE == 3){
        assert(TRH > BLACKLIST);
    }
    skipit = 0;
    if(MITIGATION_MODE == 3){
        T_STALL = 20480000/(TRH-BLACKLIST);
    }

    // ------ Randomizer Starts -----//
    unsigned long long int hash = 0;
    int *tracker= (int *) calloc(totalrows, sizeof(int));
    hashaddr = (unsigned long long int  *) calloc(totalrows, sizeof(unsigned long long int));

    for(unsigned long long int rowc = 0; rowc < totalrows; rowc++){
        tracker[rowc] = -1;
    }

    for(unsigned long long int rowc = 0; rowc < totalrows; rowc++){
        do{
          hash = rand() % totalrows;
      }while(tracker[hash] != -1);
      tracker[hash] = 0;
      hashaddr[rowc] = hash;
    }
    // ------ Randomizer Ends -----//

    mapping = (long long int*)calloc(totalrows, sizeof(long long int));
    headpointer = 0;

    for (unsigned long long l=0; l<(totalrows); l++) {
        mapping[l] = -1;
    }

    //MISRA-GRIES
    for (i=0; i<MAX_NUM_CHANNELS; i++) {
        for (j=0; j<MAX_NUM_RANKS; j++) {
            for (k=0; k<MAX_NUM_BANKS; k++) {
                row_address_track[i][j][k] = (unsigned long long int*)calloc(mg_entries, sizeof(unsigned long long int));
                row_address_count[i][j][k] = (unsigned long long int*)calloc(mg_entries, sizeof(unsigned long long int));
                spillcounter[i][j][k] = 0; 
                spillcounter_tracker[i][j][k] = 0; 
            }
        }
    }
    return;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 95

// end write queue drain once write queue has this many writes in it
#define LO_WM 20

/* Each cycle it is possible to issue a valid command from the read or write queues
   OR
   a valid precharge command to any bank (issue_precharge_command())
   OR
   a valid precharge_all bank command to a rank (issue_all_bank_precharge_command())
   OR
   a power_down command (issue_powerdown_command()), programmed either for fast or slow exit mode
   OR
   a refresh command (issue_refresh_command())
   OR
   a power_up command (issue_powerup_command())
   OR
   an activate to a specific row (issue_activate_command()).
   If a COL-RD or COL-WR is picked for issue, the scheduler also has the
   option to issue an auto-precharge in this cycle (issue_autoprecharge()).
   Before issuing a command it is important to check if it is issuable. For the RD/WR queue resident commands, checking the "command_issuable" flag is necessary. To check if the other commands (mentioned above) can be issued, it is important to check one of the following functions: is_precharge_allowed, is_all_bank_precharge_allowed, is_powerdown_fast_allowed, is_powerdown_slow_allowed, is_powerup_allowed, is_refresh_allowed, is_autoprecharge_allowed, is_activate_allowed.
   */

void resetCC()
{
    unsigned long long int totalrows = NUM_CHANNELS*NUM_RANKS*NUM_BANKS*NUM_ROWS;
    for(unsigned int index = 0; index < CRA_FILTER_ENTRIES; index++)
    {
        cra_filter[index] = 0;
    }
    for(unsigned int ctraddr = 0; ctraddr < totalrows; ctraddr++)
    {
        actcnt[ctraddr] = 0;
    }
    totalresetcc++;
    mcache_reset(CCache);
}


int rqidealRIT(unsigned long long int phyaddr)
{
    unsigned long long int rowaddress = phyaddr/(NUM_COLUMNS*64);
    unsigned long long int totalrows = NUM_CHANNELS*NUM_BANKS*NUM_RANKS*NUM_ROWS;

    long long int spareaddress = headpointer + ((TOTALOSPAGES*OS_PAGESIZE)/(NUM_COLUMNS*64));
    spareoccupancy++;

    if(mapping[rowaddress] == -1){
        if(mapping[spareaddress] == -1){
            mapping[rowaddress] = spareaddress;
            mapping[spareaddress] = rowaddress;
            headpointer = (headpointer + 1)%SPAREROWS;
            risi++;
            return 1;
        }
        else{
            mapping[mapping[spareaddress]] = -1;
            mapping[rowaddress] = spareaddress;
            mapping[spareaddress] = rowaddress;
            headpointer = (headpointer + 1)%SPAREROWS;
            risv++;
            return 2;
        }

    }
    else{
        mapping[mapping[rowaddress]] = -1;
        mapping[rowaddress] = -1;
        if(mapping[spareaddress] == -1){
            mapping[rowaddress] = spareaddress;
            mapping[spareaddress] = rowaddress;
            headpointer = (headpointer + 1)%SPAREROWS;
            rvsi++;
            return 1;
        }
        else{
            mapping[mapping[spareaddress]] = -1;
            mapping[rowaddress] = spareaddress;
            mapping[spareaddress] = rowaddress;
            headpointer = (headpointer + 1)%SPAREROWS;
            rvsv++;
            return 2;
        }
    }
    return 1;
}

int hydra(int channel, unsigned long int rank, unsigned long int bank, unsigned long int row, unsigned long int col, unsigned long long int phyaddr)
{
    unsigned long long int rth_actual = TRH;
    rth_actual = rth_actual/SWITCH_RATE;


    unsigned long long int ccblockbits = log_base2(CCBLOCK);
    unsigned long long int bitmove = 0;
    unsigned long long int ctraddr = 0;
    unsigned long long int ctraddrfinal = 0;

    unsigned int fake = 0;
    if(phyaddr > ((TOTALOSPAGES*OS_PAGESIZE)+(SPAREROWS*NUM_COLUMNS*64))){
        fake = 1;
    }

    //Counter Offset
    ctraddr = phyaddr/(NUM_COLUMNS*64);

    //Make sure spare area isnt indexed in hydra -- spare area only helps track
    assert((ctraddr < (TOTALOSPAGES*OS_PAGESIZE)) || (ctraddr >= (TOTALOSPAGES*OS_PAGESIZE)+(SPAREROWS*NUM_COLUMNS*64)));

    //Check if this location is remapped
    if(mapping[ctraddr] != -1){
        ctraddr = mapping[ctraddr];
    }

    //Distributing Counters
    ctraddr = hashaddr[ctraddr];

    // Verify how row numbers need to be calculated
    ctraddrfinal = ctraddr+((TOTALOSPAGES*OS_PAGESIZE)+(SPAREROWS*NUM_COLUMNS*64)); // Counter Byte Address
    ctraddrfinal = ctraddrfinal >> ccblockbits; // Counter Block Address
    
    if(CRA_FILTER_ENTRIES){
        filteraccesses++;
        unsigned int index = (ctraddr/64)%CRA_FILTER_ENTRIES;
        if(cra_filter[index] < (CRA_FILTER_THRESHOLD_PERC*rth_actual*0.01)){
            filterhits++;
            cra_filter[index]++;
            return 0;
        }
    }
    
    MCache_Entry victim;

    if(!fake){
        ccaccess++;
        int CCHit = isHit(CCache, ctraddrfinal, 0);
        if (!CCHit)
        {    
            if(!CRA_ENABLE_RAI){
                insert_read(ctraddrfinal << ccblockbits, CYCLE_VAL, 0, 0 , 0, 0, 0, 1, 0);
            }
            else{
                insert_read(ctraddrfinal << ccblockbits, CYCLE_VAL, 0, 0 , 0, 0, 0, 4, 0);
            }

            victim = install(CCache, ctraddrfinal, 0, 1);
            if(victim.valid && !CRA_ENABLE_RAI){
                insert_write(victim.tag << ccblockbits, CYCLE_VAL, 0, 0, 1);
            }
        }
        else{
            cchit++;
        }  
    }
    if(CRA_FILTER_ENTRIES){
        if(actcnt[ctraddr] == 0){
            actcnt[ctraddr] = CRA_FILTER_THRESHOLD_PERC*rth_actual*0.01;
        }
    }

    actcnt[ctraddr]++;
    if(actcnt[ctraddr] >= rth_actual){
        actcnt[ctraddr] = 1;
        return 1;
    }
    else{
        return 0;
    }
    return 0;
}


int misraGries(int channel, int rank, int bank, unsigned long long int rowaddr)
{
    unsigned long long int rth_actual = TRH;
    rth_actual = rth_actual/SWITCH_RATE;

    for(unsigned long long int entry = 0; entry < mg_entries; entry++)
    {
        if(row_address_track[channel][rank][bank][entry] == rowaddr)
        {
            row_address_count[channel][rank][bank][entry]++;
            if(row_address_count[channel][rank][bank][entry]%rth_actual == 0)
                return 1;
            else
                return 0;
        }
    }
    
    for(unsigned long long int entry = 0; entry < mg_entries; entry++)
    {
        if(row_address_count[channel][rank][bank][entry] == spillcounter[channel][rank][bank])
        {
            row_address_track[channel][rank][bank][entry] = rowaddr;
            row_address_count[channel][rank][bank][entry]++;
            if(row_address_count[channel][rank][bank][entry]%rth_actual == 0)
                return 1;
            else
                return 0;
        }
    }
    spillcounter[channel][rank][bank]++;
    return 0;

}


void schedule_frfcfs (int channel)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;

    if(CYCLE_VAL%204800000 == 0){ // Refresh period ends
        for (int j = 0; j < MAX_NUM_RANKS; j++){
            for (int k = 0; k < MAX_NUM_BANKS; k++){
                for (unsigned long long int r = 0; r < NUM_ROWS; r++){
                    ract_real[channel][j][k][r] = ract_real[channel][j][k][r] + ract_temp[channel][j][k][r];
                    ract_temp[channel][j][k][r] = 0; 
                }        
            }        
        }        
    }
    
    for(int k=0; k<MAX_NUM_RANKS; k++)
    {
        if(delay_stall[channel][k] == 1)
        {
            if(is_refresh_allowed(channel,k))
            {
                if(MITIGATION_MODE == 1){
                    //issue_swap(channel,k);
                    delay_stall[channel][k] = 0;
                    return;
                }
                if(MITIGATION_MODE == 2){
                    //issue_move(channel,k);
                    delay_stall[channel][k] = 0;
                    return;
                }
            }
        }
    }
    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1;    // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0;    // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {
        LL_FOREACH (write_queue_head[channel], wr_ptr)
        {
            // if no open rows, just issue any other available commands
            if (wr_ptr->command_issuable)
            {
                if(wr_ptr->next_command == ACT_CMD)
                {
                    if(MITIGATION_MODE == 1){
                        if(misraGries(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row) == 1)
                        {
                            delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                            prob_tracker[channel]++;
                        }
                    }
                    if(MITIGATION_MODE == 2){
                        if(hydra(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row, wr_ptr->dram_addr.column, wr_ptr->physical_address) == 1)
                        {
                            delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                            prob_tracker[channel]++;
                        }
                    }

                }
                issue_request_command (wr_ptr);
                return;
            }
        }
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel])
    {
        LL_FOREACH (read_queue_head[channel], rd_ptr)
        {
            // if COL_WRITE_CMD is the next command, then that means the appropriate row must already be open
            if (rd_ptr->command_issuable && (rd_ptr->next_command == COL_READ_CMD))
            {
                issue_request_command (rd_ptr);
                return;
            }
        }

    }
    if(!drain_writes[channel])
    {
        LL_FOREACH (read_queue_head[channel], rd_ptr)
        {
            // no hits, so just issue other available commands
            if (rd_ptr->command_issuable)
            {
                if(rd_ptr->next_command == ACT_CMD)
                {
                    if(MITIGATION_MODE == 1){
                        if(misraGries(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row) == 1)
                        {
                            delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                            prob_tracker[channel]++;
                        }
                    }
                    if(MITIGATION_MODE == 2){
                        if(hydra(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row, rd_ptr->dram_addr.column, rd_ptr->physical_address) == 1)
                        {
                            delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                            prob_tracker[channel]++;
                        }
                    }
                }
                issue_request_command (rd_ptr);
                return;
            }
        }
    }
    return;
}


void schedule_fcfs(int channel)
{
    //Mode 0 = Baseline
    //Mode 1 = RRS
    //Mode 2 = Hydra
    //Mode 3 = Blockhammer

    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;
    int returnvaldelay = 0;

    if(CYCLE_VAL%204800000 == 0){ // Refresh period ends
        for (int j = 0; j < MAX_NUM_RANKS; j++){
            for (int k = 0; k < MAX_NUM_BANKS; k++){
                for (unsigned long long int r = 0; r < NUM_ROWS; r++){
                    ract_real[channel][j][k][r] = ract_real[channel][j][k][r] + ract_temp[channel][j][k][r];
                    ract_temp[channel][j][k][r] = 0; 
                    stall[channel][j][k][r] = 0;
                }        
            }        
        }
        //Spill Counter Reset
        for (int j = 0; j < NUM_RANKS; j++){
            for (int k = 0; k < NUM_BANKS; k++){
                spillcounter[channel][j][k] = 0;
                // Row and Count Tracker Reset
                for(unsigned long long int entry = 0; entry < mg_entries; entry++)
                {
                    row_address_track[channel][j][k][entry] = 0;
                    row_address_count[channel][j][k][entry] = 0;
                }
            }
            if((MITIGATION_MODE == 4) || (MITIGATION_MODE == 5)){
                refreshdone[channel][j] = 1;
            }
            else{
                refreshdone[channel][j] = 0;
            }
        }

        if(CRA_FILTER_ENTRIES){
            resetCC();
        }

        for (int k = 0; k < NUM_RANKS; k++){
            prob_epoch_tracker_temp[channel][k] = prob_epoch_tracker[channel][k];
            prob_epoch_tracker[channel][k] = 0;
        }
    }

    if((MITIGATION_MODE == 4) || (MITIGATION_MODE == 5))
    {
        for (int k = 0; k < NUM_RANKS; k++){
            if(refreshdone[channel][k] == 1){
                if(prob_epoch_tracker_temp[channel][k] == 0){
                    refreshdone[channel][k] = 0;
                }
                else if(is_refresh_allowed(channel,k))
                {
                    mitigationapply[channel] = issue_move(channel, k, 1);
                    skipit = max(mitigationapply[channel], skipit);
                    mitigationapply[channel] = T_MOVE;
                    if(prob_epoch_tracker_temp[channel][k] > 0){
                        prob_epoch_tracker_temp[channel][k]--;
                    }
                    if(prob_epoch_tracker_temp[channel][k] == 0){
                        refreshdone[channel][k] = 0;
                    }
                    return;
                }
                else{
                    return;
                }
            }
        }
    }

    if(CYCLE_VAL < skipit)
    {
        return;
    }

    for(int k=0; k<NUM_RANKS; k++)
    {
        if(delay_stall[channel][k] > 0)
        {
            if(is_refresh_allowed(channel,k))
            {
                if(MITIGATION_MODE == 1){
                    if(RQENABLE){
                        issue_move(channel, k, delay_stall[channel][k]);
                        mitigationapply[channel] = T_MOVE*delay_stall[channel][k];
                    }
                    else{
                        issue_swap(channel, k, delay_stall[channel][k]);
                        mitigationapply[channel] = T_SWAP*delay_stall[channel][k];
                    }
                    //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                    delay_stall[channel][k] = 0;
                    return;
                }
                if(MITIGATION_MODE == 2){
                    if(RQENABLE){
                        issue_move(channel, k, delay_stall[channel][k]);
                        mitigationapply[channel] = T_MOVE*delay_stall[channel][k];
                    }
                    else{
                        issue_swap(channel, k, delay_stall[channel][k]);
                        mitigationapply[channel] = T_SWAP*delay_stall[channel][k];
                    }
                    //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                    delay_stall[channel][k] = 0;
                    return;
                }
                if(MITIGATION_MODE == 3){
                    issue_stall(channel, k, delay_stall[channel][k]);
                    mitigationapply[channel] = T_STALL*delay_stall[channel][k];
                    delay_stall[channel][k] = 0;
                    return;
                }
                if(MITIGATION_MODE == 4){
                    mitigationapply[channel] = issue_move(channel, k, delay_stall[channel][k]);
                    skipit = max(mitigationapply[channel], skipit);
                    mitigationapply[channel] = T_MOVE*delay_stall[channel][k];
                    delay_stall[channel][k] = 0;
                    return;
                }
                if(MITIGATION_MODE == 5){
                    mitigationapply[channel] = issue_move(channel, k, delay_stall[channel][k]);
                    skipit = max(mitigationapply[channel], skipit);
                    mitigationapply[channel] = T_MOVE*delay_stall[channel][k];
                    delay_stall[channel][k] = 0;
                    return;
                }
                if(MITIGATION_MODE == 6){
                    mitigationapply[channel] = issue_move(channel, k, 1);
                    skipit = max(mitigationapply[channel], skipit);
                    mitigationapply[channel] = T_MOVE*delay_stall[channel][k];
                    if(delay_stall[channel][k] > 0){
                        delay_stall[channel][k]--;
                    }
                    return;
                }
                if(MITIGATION_MODE == 7){
                    mitigationapply[channel] = issue_move(channel, k, 1);
                    skipit = max(mitigationapply[channel], skipit);
                    mitigationapply[channel] = T_MOVE*delay_stall[channel][k];
                    if(delay_stall[channel][k] > 0){
                        delay_stall[channel][k]--;
                    }
                    return;
                }
            }
            return;
        }
    }

    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    /*else {
        if (!read_queue_length[channel]){
            drain_writes[channel] = 1;
        }
    }*/


    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready

    /*if(drain_writes[channel])
    {        
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                if (wr_ptr->next_command == COL_WRITE_CMD) {
                    wr_ptr->tracker = 1;
                    mitigationapply[channel] = 0;
                    issue_request_command(wr_ptr);
                    return;
                }
            }
        }
    }*/


    if(drain_writes[channel])
    {
        unsigned long long int timesince = (204800000 - (CYCLE_VAL%204800000));
    
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                if (wr_ptr->next_command == ACT_CMD) {
                    if(wr_ptr->tracker == 0){
                        wr_ptr->tracker = 1;
                        ract_temp[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row]++;
                        unique[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row] = 1;

                        if(MITIGATION_MODE == 1){
                            if(misraGries(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row) == 1)
                            {
                                if(RQENABLE){
                                    delay_stall[channel][wr_ptr->dram_addr.rank] = rqidealRIT(wr_ptr->physical_address);
                                    mitigationapply[channel] = T_MOVE*delay_stall[channel][wr_ptr->dram_addr.rank];
                                }
                                else{
                                    delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                                    mitigationapply[channel] = T_SWAP*delay_stall[channel][wr_ptr->dram_addr.rank];
                                }
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][wr_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 2){
                            if(hydra(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row, wr_ptr->dram_addr.column, wr_ptr->physical_address) == 1)
                            {
                                if(RQENABLE){
                                    delay_stall[channel][wr_ptr->dram_addr.rank] = rqidealRIT(wr_ptr->physical_address);
                                    mitigationapply[channel] = T_MOVE*delay_stall[channel][wr_ptr->dram_addr.rank];
                                }
                                else{
                                    delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                                    mitigationapply[channel] = T_SWAP*delay_stall[channel][wr_ptr->dram_addr.rank];
                                }
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][wr_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 3){
                            if(ract_temp[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row] == BLACKLIST){
                                stall[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row] = T_STALL;
                                delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                            }
                            if(ract_temp[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row] > BLACKLIST){
                                if((timesince/ract_temp[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row]) < stall[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank][wr_ptr->dram_addr.row]){
                                    delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                                }
                            }
                        }
                        if(MITIGATION_MODE == 4){
                            if(misraGries(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row) == 1)
                            {
                                delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][wr_ptr->dram_addr.rank];
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][wr_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 5){
                            if(hydra(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row, wr_ptr->dram_addr.column, wr_ptr->physical_address) == 1)
                            {
                                delay_stall[channel][wr_ptr->dram_addr.rank] = 1;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][wr_ptr->dram_addr.rank];
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][wr_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 6){
                            if(misraGries(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row) == 1)
                            {
                                delay_stall[channel][wr_ptr->dram_addr.rank] = 2;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][wr_ptr->dram_addr.rank];
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][wr_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 7){
                            if(hydra(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank, wr_ptr->dram_addr.row, wr_ptr->dram_addr.column, wr_ptr->physical_address) == 1)
                            {
                                delay_stall[channel][wr_ptr->dram_addr.rank] = 2;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][wr_ptr->dram_addr.rank];
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][wr_ptr->dram_addr.rank]++;
                            }
                        }
                    }
                }
                issue_request_command(wr_ptr);
                return;
            }
            return;
        }
        return;
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS

    /*if(!drain_writes[channel])
    {        
        LL_FOREACH(read_queue_head[channel], rd_ptr)
        {
            if(rd_ptr->command_issuable)
            {
                if (rd_ptr->next_command == COL_READ_CMD){
                    rd_ptr->tracker = 1;
                    mitigationapply[channel] = 0;
                    issue_request_command(rd_ptr);
                    return;
                }
            }
        }
    }*/

    if(!drain_writes[channel])
    {
        unsigned long long int timesince = (204800000 - (CYCLE_VAL%204800000));
        LL_FOREACH(read_queue_head[channel],rd_ptr)
        {
            if(rd_ptr->command_issuable)
            {
                if ((rd_ptr->next_command == ACT_CMD)) {
                    if(rd_ptr->tracker == 0){
                        rd_ptr->tracker = 1;
                        ract_temp[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row]++;
                        unique[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row] = 1;
                        if(MITIGATION_MODE == 1){
                            if(misraGries(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row) == 1)
                            {
                                if(RQENABLE){
                                    delay_stall[channel][rd_ptr->dram_addr.rank] = rqidealRIT(rd_ptr->physical_address);
                                    mitigationapply[channel] = T_MOVE*delay_stall[channel][rd_ptr->dram_addr.rank];
                                }
                                else{
                                    delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                                    mitigationapply[channel] = T_SWAP*delay_stall[channel][rd_ptr->dram_addr.rank];
                                }
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][rd_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 2){
                            if(hydra(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row, rd_ptr->dram_addr.column, rd_ptr->physical_address) == 1)
                            {
                                if(RQENABLE){
                                    delay_stall[channel][rd_ptr->dram_addr.rank] = rqidealRIT(rd_ptr->physical_address);
                                    mitigationapply[channel] = T_MOVE*delay_stall[channel][rd_ptr->dram_addr.rank];
                                }
                                else{
                                    delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                                    mitigationapply[channel] = T_SWAP*delay_stall[channel][rd_ptr->dram_addr.rank];
                                }
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][rd_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 3){
                            if(ract_temp[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row] == BLACKLIST){
                                stall[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row] = T_STALL;
                                delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                            }
                            if(ract_temp[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row] > BLACKLIST){
                                if((timesince/ract_temp[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row]) < stall[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->dram_addr.row]){
                                    delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                                }
                            }
                        }
                        if(MITIGATION_MODE == 4){
                            if(misraGries(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row) == 1)
                            {
                                delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][rd_ptr->dram_addr.rank];
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][rd_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 5){
                            if(hydra(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row, rd_ptr->dram_addr.column, rd_ptr->physical_address) == 1)
                            {
                                delay_stall[channel][rd_ptr->dram_addr.rank] = 1;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][rd_ptr->dram_addr.rank];
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][rd_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 6){
                            if(misraGries(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row) == 1)
                            {
                                delay_stall[channel][rd_ptr->dram_addr.rank] = 2;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][rd_ptr->dram_addr.rank];
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][rd_ptr->dram_addr.rank]++;
                            }
                        }
                        if(MITIGATION_MODE == 7){
                            if(hydra(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank, rd_ptr->dram_addr.row, rd_ptr->dram_addr.column, rd_ptr->physical_address) == 1)
                            {
                                delay_stall[channel][rd_ptr->dram_addr.rank] = 2;
                                mitigationapply[channel] = T_MOVE*delay_stall[channel][rd_ptr->dram_addr.rank];
                                //skipit = max(CYCLE_VAL + mitigationapply[channel], skipit);
                                prob_tracker[channel]++;
                                prob_epoch_tracker[channel][rd_ptr->dram_addr.rank]++;
                            }
                        }
                    }
                }
                issue_request_command(rd_ptr);
                return;
            }
            return;
        }
        return;
    }
}


void schedule_camou(int channel)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;
    long long int inter_arrival_time = 0;

    if(CYCLE_VAL%7920 == 0){
        epoch_start_timer[channel] = CYCLE_VAL;
        for(int i = 0; i< 10; i++)
        {
            bin_fake[channel][i] = bin_active[channel][i];
        }
        for(int i = 0; i< 10; i++)
        {
            bin_active[channel][i] = (10-i) - bin_fake[channel][i];
        }
    }


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }


    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {

        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
        return;
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if(!drain_writes[channel])
    {
        LL_FOREACH(read_queue_head[channel],rd_ptr)
        {
            if(rd_ptr->command_issuable)
            {
                inter_arrival_time = 0;
                inter_arrival_time = inter_arrival_time/20;
                if(inter_arrival_time > 9){
                    inter_arrival_time = 9;
                }
                if(rd_ptr->next_command == COL_READ_CMD){
                    if(bin_active[channel][inter_arrival_time] < (10-inter_arrival_time)){
                        issue_request_command(rd_ptr);
                        bin_active[channel][inter_arrival_time]++;
                        return;
                    }
                }
                else{
                    issue_request_command(rd_ptr);
                    return;
                }
            }
        }
    }
}


void schedule_closepage(int channel)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;
    int i, j;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel] && !wait_time[channel])
            drain_writes[channel] = 1;
    }


    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {

        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if(!drain_writes[channel] && !wait_time[channel])
    {
        LL_FOREACH(read_queue_head[channel],rd_ptr)
        {
            if(rd_ptr->command_issuable)
            {
                /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                   If the bank just did an activate or precharge, it is not a candidate for closure. */
                if (rd_ptr->next_command == COL_READ_CMD) {
                    recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 1;
                    wait_time[channel] = 1;
                }
                if (rd_ptr->next_command == ACT_CMD) {
                    recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
                }
                if (rd_ptr->next_command == PRE_CMD) {
                    recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
                }
                issue_request_command(rd_ptr);
                return;
            }
        }
    }

    // Check if you need to close the page (individually)
    if(wait_time[channel] == 1){
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if (recent_colacc[channel][i][j]){
                    if (is_precharge_allowed(channel, i, j)){
                        if (issue_precharge_command(channel, i, j))
                        {
                            recent_colacc[channel][i][j] = 0;
                        }
                    }
                }
            }
        }
        wait_time[channel] = 0;
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if(recent_colacc[channel][i][j]){
                    wait_time[channel] = 1;
                }
            }
        }
    }
}


void schedule_lps(int channel)
{

    //2 requests per turn : 44*4*2 = 352 cycles

    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }


    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }


    // Draining Reads
    if(!drain_writes[channel])
    {
        if(CYCLE_VAL%(3*4*44*NUMCORES) == 0){
            //current_core[channel] = 0;
            perthread_timer_end[channel] = CYCLE_VAL+(3*4*44);
            perthread_timer_start[channel] = CYCLE_VAL;
        }

        for(int dd = current_core[channel]; dd >=0 ; dd--){
            //printf("Current core is %d\n", current_core[channel]);
            LL_FOREACH(read_queue_head[channel], rd_ptr){
                if(rd_ptr->command_issuable){
                    if(rd_ptr->thread_id == dd)
                    {
                        if((dd == current_core[channel]) && (CYCLE_VAL >= perthread_timer_start[channel])){
                            if(CYCLE_VAL > perthread_timer_end[channel]){
                                current_core[channel]++;
                                perthread_timer_start[channel] = CYCLE_VAL;
                                perthread_timer_end[channel] = CYCLE_VAL+(3*4*44);
                                if(current_core[channel] == NUMCORES){
                                    current_core[channel] = NUMCORES-1;
                                }
                                return;
                            }
                            issue_request_command(rd_ptr);
                            return;
                        }
                        else if((dd != current_core[channel]) && (CYCLE_VAL >= perthread_timer_start[channel])){
                            if(CYCLE_VAL < perthread_timer_end[channel]){
                                issue_request_command(rd_ptr);
                            }
                            return;
                        }
                    }

                }
            }
        }
    }
    return;

}

void schedule_tp(int channel, unsigned int *runcore)
{

    //2 requests per turn : 44*4*2 = 352 cycles

    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }


    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }


    // Draining Reads
    if(!drain_writes[channel])
    {
        if(CYCLE_VAL%(10*4*44*NUMCORES) == 0){
            runcore[channel] = (runcore[channel]+1)%NUMCORES;
            perthread_timer_end[channel] = CYCLE_VAL+(10*4*44);
            perthread_timer_start[channel] = CYCLE_VAL;
        }
        LL_FOREACH(read_queue_head[channel], rd_ptr){
            if(rd_ptr->command_issuable){
                if(rd_ptr->thread_id == runcore[channel])
                {
                    if(CYCLE_VAL >= perthread_timer_start[channel]){
                        if(CYCLE_VAL > perthread_timer_end[channel]){
                            perthread_timer_start[channel] = CYCLE_VAL;
                            perthread_timer_end[channel] = CYCLE_VAL+(10*4*44);
                            runcore[channel] = (runcore[channel]+1)%NUMCORES;
                            return;
                        }
                        issue_request_command(rd_ptr);
                        return;
                    }
                }

            }
        }
    }
    return;

}

void schedule_bta(int channel)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }


    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }

    // Draining Reads
    if(!drain_writes[channel])
    {
        LL_FOREACH(read_queue_head[channel], rd_ptr){
            if(rd_ptr->command_issuable){
                if((CYCLE_VAL - time_diff[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->next_command]) > 244) // 43*4 = 172
                {
                    issue_request_command(rd_ptr);
                    time_diff[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank][rd_ptr->next_command] = CYCLE_VAL;
                    return;
                }
            }
        }
    }
    return;

}


void schedule_fs(int channel, unsigned int *runcore)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;

    int i, j;

    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel] && !wait_time[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if (wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel] && !wait_time[channel])
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
            temp_ptr = rd_ptr->reqptr;
            assert (temp_ptr != NULL);
            if (temp_ptr->command_issuable)
            {
                /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                   If the bank just did an activate or precharge, it is not a candidate for closure. */
                if (temp_ptr->next_command == COL_READ_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                    wait_time[channel] = 1;
                    runcore[channel] = (runcore[channel]+1)%NUMCORES;
                }
                if (temp_ptr->next_command == ACT_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                if (temp_ptr->next_command == PRE_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                issue_request_command(temp_ptr);
                return;
            }
        }
    }

    /* If a command hasn't yet been issued to this channel in this cycle, issue a precharge. */
    // Check if you need to close the page (individually)
    if(wait_time[channel] == 1){
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if (recent_colacc[channel][i][j]){
                    if (is_precharge_allowed(channel, i, j)){
                        if (issue_precharge_command(channel, i, j))
                        {
                            recent_colacc[channel][i][j] = 0;
                        }
                    }
                }
            }
        }
        wait_time[channel] = 0;
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if(recent_colacc[channel][i][j]){
                    wait_time[channel] = 1;
                }
            }
        }
    }
    return;
}

void schedule_fsopen(int channel, unsigned int *runcore)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {   
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {   
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {   
        drain_writes[channel] = 1;
    }
    else
    {   
        if (!read_queue_length[channel] && !wait_time[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {   
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {   
            if (wr_ptr->command_issuable)
            {   
                issue_request_command(wr_ptr);
                break;
            }
        }
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel] && !wait_time[channel])
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
            temp_ptr = rd_ptr->reqptr;
            assert (temp_ptr != NULL);
            if (temp_ptr->command_issuable)
            {
                /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                   If the bank just did an activate or precharge, it is not a candidate for closure. */
                if (temp_ptr->next_command == COL_READ_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                    wait_time[channel] = 1;
                    runcore[channel] = (runcore[channel]+1)%NUMCORES;
                }
                if (temp_ptr->next_command == ACT_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                if (temp_ptr->next_command == PRE_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                issue_request_command(temp_ptr);
                return;
            }
        }
    }
    wait_time[channel] = 0;
    return;
}

void schedule_bl(int channel, unsigned int *runcore, unsigned long long int **timer_bank, long long int *change_core)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;

    int i, j;

    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel] && !wait_time[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if (wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
    }

    if(timer_bank[runcore[channel]][channel] >= 4){
        timer_bank[runcore[channel]][channel] = timer_bank[runcore[channel]][channel] - 4;
    }

   //printf("timer is %d\n", timer_bank[runcore[channel]][channel]);
    if(timer_bank[runcore[channel]][channel] > 0){
            wait_time[channel] = 0;
     }else{
           wait_time[channel] = 1;
           runcore[channel] = (runcore[channel]+1)%NUMCORES;
           change_core[runcore[channel]] = 1;
     }
     
     // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel] && !wait_time[channel])
    {
        //printf("come here");
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
            temp_ptr = rd_ptr->reqptr;
            assert (temp_ptr != NULL);
            if (temp_ptr->command_issuable)
            {
                /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                   If the bank just did an activate or precharge, it is not a candidate for closure. */
                if (temp_ptr->next_command == COL_READ_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                }
                if (temp_ptr->next_command == PRE_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                if (temp_ptr->next_command == ACT_CMD && temp_ptr->dram_addr.bank != prev_bank[runcore[channel]][channel][temp_ptr->dram_addr.rank])
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                    prev_bank[runcore[channel]][channel][temp_ptr->dram_addr.rank] = temp_ptr->dram_addr.bank;
                }
                issue_request_command(temp_ptr);
                return;
            }
        }

    }

    /* If a command hasn't yet been issued to this channel in this cycle, issue a precharge. */
    // Check if you need to close the page (individually)
    if(wait_time[channel] == 1){
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if (recent_colacc[channel][i][j]){
                    if (is_precharge_allowed(channel, i, j)){
                        if (issue_precharge_command(channel, i, j))
                        {
                            recent_colacc[channel][i][j] = 0;
                        }
                    }
                }
            }
        }
    }
    return;
}

void schedule_blopen(int channel, unsigned int *runcore, unsigned long long int **timer_bank, long long int *change_core)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel] && !wait_time[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if (wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
    }

    if(timer_bank[runcore[channel]][channel] >= 4){
        timer_bank[runcore[channel]][channel] = timer_bank[runcore[channel]][channel] - 4;
    }

   //printf("timer is %d\n", timer_bank[runcore[channel]][channel]);
    if(timer_bank[runcore[channel]][channel] > 0){
            wait_time[channel] = 0;
     }else{
           wait_time[channel] = 1;
           runcore[channel] = (runcore[channel]+1)%NUMCORES;
           change_core[runcore[channel]] = 1;
     }

     // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel] && !wait_time[channel])
    {
        //printf("come here");
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
            temp_ptr = rd_ptr->reqptr;
            assert (temp_ptr != NULL);
            if (temp_ptr->command_issuable)
            {
                /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                   If the bank just did an activate or precharge, it is not a candidate for closure. */
                if (temp_ptr->next_command == COL_READ_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                }
                if (temp_ptr->next_command == PRE_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                if (temp_ptr->next_command == ACT_CMD && temp_ptr->dram_addr.bank != prev_bank[runcore[channel]][channel][temp_ptr->dram_addr.rank])
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                    prev_bank[runcore[channel]][channel][temp_ptr->dram_addr.rank] = temp_ptr->dram_addr.bank;
                }
                issue_request_command(temp_ptr);
                return;
            }
        }

    }

    /* If a command hasn't yet been issued to this channel in this cycle, issue a precharge. */
    // Check if you need to close the page (individually)
    return;
}

void schedule_pr(int channel)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    int i,j;
    int k = 0;
    request_t *temp_ptr = NULL;

    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {

        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if (wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }

    if ((new_sandbox_mode1[channel]->mode == 0) && (read_queue_length[channel] > 0) && (new_sandbox_mode1[channel]->close_page == 0)){
        LL_FOREACH(read_queue_head[channel], temp_ptr){
            k++;
            if(k == read_queue_length[channel])
            {
                new_sandbox_mode1[channel]->bottom_request = temp_ptr;
                break;
            }
        }
        new_sandbox_mode1[channel]->mode = 1;
        new_sandbox_mode1[channel]->length = k;
        sbm1 += k;
        tem1++;
    }
    k = 0;
    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if((!drain_writes[channel]) && (new_sandbox_mode1[channel]->close_page == 0))
    {
        LL_FOREACH(read_queue_head[channel], rd_ptr)
        {
            if(new_sandbox_mode1[channel]->close_page > 0)
            {
                new_sandbox_mode1[channel]->mode = 0;
                break;
            }
            else if(new_sandbox_mode1[channel]->length == 0)
            {
                new_sandbox_mode1[channel]->mode = 0;
                new_sandbox_mode1[channel]->close_page = NUM_RANKS;
                break;
            }
            else{
                if (rd_ptr->command_issuable && (rd_ptr->request_id <= new_sandbox_mode1[channel]->bottom_request->request_id))
                {
                    if(rd_ptr->next_command == COL_READ_CMD)
                    {
                        new_sandbox_mode1[channel]->length--;
                        if(new_sandbox_mode1[channel]->length == 0){
                            new_sandbox_mode1[channel]->bottom_request = NULL;
                            new_sandbox_mode1[channel]->mode = 0;
                            new_sandbox_mode1[channel]->close_page = NUM_RANKS;
                        }
                        else{
                            if(rd_ptr->request_id == new_sandbox_mode1[channel]->bottom_request->request_id){
                                LL_FOREACH(read_queue_head[channel], temp_ptr){
                                    k++;
                                    if(k == new_sandbox_mode1[channel]->length)
                                    {
                                        new_sandbox_mode1[channel]->bottom_request = temp_ptr;
                                        break;
                                    }
                                }
                            }
                        }
                        recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 1;
                        issue_request_command(rd_ptr);
                        return;
                    }
                }
            }
        }
        LL_FOREACH(read_queue_head[channel], rd_ptr)
        {
            if(new_sandbox_mode1[channel]->close_page > 0)
            {
                new_sandbox_mode1[channel]->mode = 0;
                break;
            }
            else if(new_sandbox_mode1[channel]->length == 0)
            {
                new_sandbox_mode1[channel]->mode = 0;
                new_sandbox_mode1[channel]->close_page = NUM_RANKS;
                break;
            }
            else{
                if (rd_ptr->command_issuable && (rd_ptr->request_id <= new_sandbox_mode1[channel]->bottom_request->request_id))
                {
                    if(rd_ptr->next_command == COL_READ_CMD)
                    {
                        new_sandbox_mode1[channel]->length--;
                        if(new_sandbox_mode1[channel]->length == 0){
                            new_sandbox_mode1[channel]->bottom_request = NULL;
                            new_sandbox_mode1[channel]->mode = 0;
                            new_sandbox_mode1[channel]->close_page = NUM_RANKS;
                        }
                        else{
                            if(rd_ptr->request_id == new_sandbox_mode1[channel]->bottom_request->request_id){
                                LL_FOREACH(read_queue_head[channel], temp_ptr){
                                    k++;
                                    if(k == new_sandbox_mode1[channel]->length)
                                    {
                                        new_sandbox_mode1[channel]->bottom_request = temp_ptr;
                                        break;
                                    }
                                }
                            }
                        }
                        recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 1;
                    }
                    if (rd_ptr->next_command == ACT_CMD)
                    {
                        recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
                    }
                    if (rd_ptr->next_command == PRE_CMD)
                    {
                        recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
                    }
                    issue_request_command(rd_ptr);
                    return;
                }
            }
        }
    }
    // Check if you need to close the page (individually)
    if(new_sandbox_mode1[channel]->close_page > 0){
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if (recent_colacc[channel][i][j]){
                    if (is_precharge_allowed(channel, i, j)){
                        if (issue_precharge_command(channel, i, j))
                        {
                            recent_colacc[channel][i][j] = 0;
                        }
                    }
                }
            }
        }
        new_sandbox_mode1[channel]->close_page = 0;
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if(recent_colacc[channel][i][j]){
                    new_sandbox_mode1[channel]->close_page = NUM_RANKS;
                }
            }
        }
    }
}

void schedule_pq(int channel, unsigned int *runcore, long long int **num_issue, unsigned long long int *timer, unsigned long long int ***prefetch_buffer, int buffer_size, unsigned long long int *fakereadreqfunc)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;

    int i, j;

    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel] && !wait_time[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if (wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }
    
    if(timer[channel] >= 4){
        timer[channel] = timer[channel] - 4;
    }

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel] && (num_issue[runcore[channel]][channel] > 0))
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        { 
            temp_ptr = rd_ptr->reqptr;
            assert (temp_ptr != NULL);
            if (temp_ptr->command_issuable)
            {
                /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                   If the bank just did an activate or precharge, it is not a candidate for closure. */
                if (temp_ptr->next_command == COL_READ_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                    if(num_issue[runcore[channel]][channel] > 0){
                        num_issue[runcore[channel]][channel]--;
                        if(num_issue[runcore[channel]][channel] == 0){
                            wait_time[channel] = 1;
                        }
                    }
                    if(temp_ptr->fake)
                    {
                        *fakereadreqfunc = *fakereadreqfunc + 1;
                        for(int rr = (buffer_size-1); rr > 0; rr--){
                            prefetch_buffer[runcore[channel]][channel][rr] = prefetch_buffer[runcore[channel]][channel][rr-1];
                        }
                        prefetch_buffer[runcore[channel]][channel][0] = (temp_ptr->physical_address) >> 6;
                        prefetch_counter++;
                    }
                }
                issue_request_command(temp_ptr);
                return;
            }
        }
    }
    if (!drain_writes[channel] && (num_issue[runcore[channel]][channel] > 0))
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        { 
            temp_ptr = rd_ptr->reqptr;
            assert (temp_ptr != NULL);
            if (temp_ptr->command_issuable)
            {
                if (temp_ptr->next_command == ACT_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                if (temp_ptr->next_command == PRE_CMD)
                {
                    recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                }
                issue_request_command(temp_ptr);
                return;
            }
        }
    }

    /* If a command hasn't yet been issued to this channel in this cycle, issue a precharge. */
    // Check if you need to close the page (individually)
    if(num_issue[runcore[channel]][channel] == 0){
        wait_time[channel] = 0;
        for (i = 0; i < NUM_RANKS; i++)
        {
            for (j = 0; j < NUM_BANKS; j++){
                if (recent_colacc[channel][i][j]){
                    if (is_precharge_allowed(channel, i, j)){
                        if (issue_precharge_command(channel, i, j))
                        {
                            recent_colacc[channel][i][j] = 0;
                        }
                    }
                    wait_time[channel] = 1;
                }
            }
        }

        if(timer[channel] > 0){
            wait_time[channel] = 1;
        }

        if(wait_time[channel] == 0)
        {
            LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
            {    
                temp_ptr = rd_ptr->reqptr;
                assert (temp_ptr != NULL);
                if (temp_ptr->fake)
                {
                    temp_ptr->request_served = 1;
                }
            }
            num_issue[runcore[channel]][channel] = -1;
            runcore[channel] = (runcore[channel]+1)%NUMCORES;
        }
    }
    return;
}

void schedule_pb(int channel, unsigned int *runcore, tracker_t ***per_thread_tracker)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;
    request_t *issue_ptr = NULL;
    int rand_num;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {   
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {   
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {   
        drain_writes[channel] = 1;
    }
    else
    {   
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {   
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {   
            if (wr_ptr->command_issuable)
            {   
                issue_request_command(wr_ptr);
                break;
            }
        }
    }

    
    if(per_thread_tracker[runcore[channel]][channel]->length == 0)
    {
        return;
    }else{
        while(true)
        {
            rand_num = rand() % 7;
            // printf("random bank generated %d\n", rand_num);
            // printf("tracker length is %d\n", per_thread_tracker[runcore[channel]][channel]->length);
            // printf("tracker item bank is %d\n", per_thread_tracker[runcore[channel]][channel]->tracker_i[rand_num]->fake);
             printf("tracker item valiability is %d\n", per_thread_tracker[runcore[channel]][channel]->tracker_i[rand_num]->valid);
            if(per_thread_tracker[runcore[channel]][channel]->tracker_i[rand_num]->valid == 1)
            {
                issue_ptr = per_thread_tracker[runcore[channel]][channel]->tracker_i[rand_num];
                //per_thread_tracker[runcore[channel]][channel]->tracker_i[rand_num]->valid = 0;
                break;
            }
            //printf("Slecting request to issue\n");
        }
    }
    
    

    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if (!drain_writes[channel])
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
            if(rd_ptr->request_id == issue_ptr->request_id)
            {
                temp_ptr = rd_ptr->reqptr;
                assert (temp_ptr != NULL);
                if (temp_ptr->command_issuable)
                {
                    /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                    If the bank just did an activate or precharge, it is not a candidate for closure. */
                    if (temp_ptr->next_command == COL_READ_CMD)
                    {
                        recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                        per_thread_tracker[runcore[channel]][channel]->length--;
                        per_thread_tracker[runcore[channel]][channel]->tracker_i[rand_num]->valid = 0;
                        runcore[channel] = (runcore[channel]+1)%NUMCORES;
                    }
                    if (temp_ptr->next_command == ACT_CMD)
                    {
                        recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                    }
                    if (temp_ptr->next_command == PRE_CMD)
                    {
                        recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                    }
                    issue_request_command(temp_ptr);
                    return;
                }
            }
        }
    }
    return;
}

void schedule_bp(int channel, unsigned int *runcore, unsigned int **bank, unsigned int ***issued, unsigned int ***bank_busy, unsigned int **bank_group, unsigned long long int req_id)
{
    request_t *rd_ptr = NULL;
    request_t *wr_ptr = NULL;
    request_t *temp_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM))
    {
        drain_writes[channel] = 1; // Keep draining.
    }
    else
    {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if (write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else
    {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }

    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if (drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if (wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
    }


    // Draining Reads
    // look through the queue and find the first request whose
    // command can be issued in this cycle and issue it
    // Simple FCFS
    if(!drain_writes[channel])
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
                
                temp_ptr = rd_ptr->reqptr;
                if(temp_ptr->dram_addr.bank == bank[runcore[channel]][channel] && temp_ptr->request_id == req_id && temp_ptr->bank_group == bank_group[runcore[channel]][channel])
                {

                    assert (temp_ptr != NULL);
                    // if(temp_ptr->request_served == 1)
                    // {
                    //     bank_busy[runcore[channel]][channel][bank[runcore[channel]][channel]] = 0;
                    //     bank[runcore[channel]][channel] = (bank[runcore[channel]][channel]+1)%NUM_BANKS;
                    // }

                    if (temp_ptr->command_issuable)
                    {
                        printf("runcore %d || bank %d || fake %d || bank group %d || Command %d || req id %lld || cycle %lld || command_issuable %d || bank busy %d\n" , runcore[channel], temp_ptr->dram_addr.bank, temp_ptr->fake, bank_group[runcore[channel]][channel], temp_ptr->next_command, temp_ptr->request_id, CYCLE_VAL, temp_ptr->command_issuable, bank_busy[runcore[channel]][channel][bank[runcore[channel]][channel]]);
                        /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                        If the bank just did an activate or precharge, it is not a candidate for closure. */
                        if (temp_ptr->next_command == COL_READ_CMD)
                        {
                            recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                            bank_busy[runcore[channel]][channel][bank[runcore[channel]][channel]] = 1;
                            // bank[runcore[channel]][channel] = (bank[runcore[channel]][channel]+1)%NUM_BANKS; 
                            bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%2; 

                        }
                        if (temp_ptr->next_command == ACT_CMD)
                        {

                            recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                            bank_busy[runcore[channel]][channel][bank[runcore[channel]][channel]] = 0;
                            // bank[runcore[channel]][channel] = (bank[runcore[channel]][channel]+1)%NUM_BANKS; 
                            bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%2;

                        }
                        if (temp_ptr->next_command == PRE_CMD)
                        {
                            recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                            bank_busy[runcore[channel]][channel][bank[runcore[channel]][channel]] = 1;
                            // bank[runcore[channel]][channel] = (bank[runcore[channel]][channel]+1)%NUM_BANKS; 
                            bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%2;

                        }

                        // if(bank_group[runcore[channel]][channel] == 0)
                        // {
                        //     bank[runcore[channel]][channel] = rand() % 4;
                        //     // printf("Change bank G0\n");
                        //     // printf("Current bank %d\n", bank[runcore[channel]][channel]);
                        // }else if(bank_group[runcore[channel]][channel] == 1)
                        // {
                        //     bank[runcore[channel]][channel] = random_num(4, 7);
                        //     // printf("Change bank G1\n");
                        //     // printf("Current bank %d\n", bank[runcore[channel]][channel]);
                        // }

                        if(temp_ptr->fake)
                        {
                            prefetch_counter++;
                        }
                     
                        issue_request_command(temp_ptr); 
                        return;
                    }
                }
        }
    }

    /* If a command hasn't yet been issued to this channel in this cycle, issue a precharge. */
    // Check if you need to close the page (individually)
    return;
}

void schedule_secmc(int channel, unsigned int *runcore, unsigned int **bank, unsigned int ***issued, unsigned int ***bank_busy, unsigned int **switch_core, unsigned int **bank_group, int skip, unsigned long long int req_id)
{
    request_t * rd_ptr = NULL;
    request_t * wr_ptr = NULL;
    request_t *temp_ptr = NULL;


    // if in write drain mode, keep draining writes until the
    // write queue occupancy drops to LO_WM
    if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
    }
    else {
        drain_writes[channel] = 0; // No need to drain.
    }

    // initiate write drain if either the write queue occupancy
    // has reached the HI_WM , OR, if there are no pending read
    // requests
    if(write_queue_length[channel] > HI_WM)
    {
        drain_writes[channel] = 1;
    }
    else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
    }

    if(switch_core[runcore[channel]][channel] == (NUM_BANKS - 1))
    {
        runcore[channel] = (runcore[channel]+1)%NUMCORES;
        switch_core[runcore[channel]][channel] = 0;
        // printf("core %d\n", runcore[channel]);
    }    
    
    // If in write drain mode, look through all the write queue
    // elements (already arranged in the order of arrival), and
    // issue the command for the first request that is ready
    if(drain_writes[channel])
    {
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                return;
            }
        }
    }

    // Draining Reads
    if(!drain_writes[channel])
    {
        LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
        {
                
                temp_ptr = rd_ptr->reqptr;
                if(temp_ptr->dram_addr.bank == bank[runcore[channel]][channel] && temp_ptr->bank_group == bank_group[runcore[channel]][channel] && temp_ptr->request_id == req_id)
                {
                    skip = 0;
                    assert (temp_ptr != NULL);
                    if (temp_ptr->command_issuable)
                    {
                        printf("runcore %d || bank %d || fake %d || bank group %d || Command %d || req id %lld || cycle %lld || command_issuable %d\n" , runcore[channel], temp_ptr->dram_addr.bank, temp_ptr->fake, bank_group[runcore[channel]][channel], temp_ptr->next_command, temp_ptr->request_id, CYCLE_VAL, temp_ptr->command_issuable);
                        /* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
                        If the bank just did an activate or precharge, it is not a candidate for closure. */
                        if (temp_ptr->next_command == COL_READ_CMD)
                        {
                            recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 1;
                            bank_busy[runcore[channel]][channel][bank_group[runcore[channel]][channel]] = 1;
                            bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%8;

                        }
                        if (temp_ptr->next_command == ACT_CMD)
                        {

                            recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                            bank_busy[runcore[channel]][channel][bank_group[runcore[channel]][channel]] = 0;
                            bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%8;

                        }
                        if (temp_ptr->next_command == PRE_CMD)
                        {
                            recent_colacc[channel][temp_ptr->dram_addr.rank][temp_ptr->dram_addr.bank] = 0;
                            bank_busy[runcore[channel]][channel][bank_group[runcore[channel]][channel]] = 1;
                            bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%8;

                        }
                        if(temp_ptr->fake)
                        {
                            prefetch_counter++;
                        }
                        // bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%2;

                        // printf("runcore %d || bank %d || fake %d || bank group %d || Command %d || req id %lld || cycle %lld\n" , runcore[channel], temp_ptr->dram_addr.bank, temp_ptr->fake, bank_group[runcore[channel]][channel], temp_ptr->next_command, temp_ptr->request_id, CYCLE_VAL);
                     
                        issue_request_command(temp_ptr); 
                        return;
                    }
                }
        }
        skip = 1;
        // bank_busy[runcore[channel]][channel][bank_group[runcore[channel]][channel]] = 1;
        // bank_group[runcore[channel]][channel] = (bank_group[runcore[channel]][channel]+1)%2;
    }
    return;

}

unsigned long long int get_demreq_addr(int channel, unsigned int *runcore)
{
    request_t *rd_ptr = NULL;
    LL_FOREACH(read_queue_head[channel], rd_ptr)
    {
        if(rd_ptr->request_id == per_thread_queue[runcore[channel]][channel]->request_id)
        {
            return rd_ptr->physical_address;
        }
    }
    return 0;
}

unsigned long long int get_demreq_next_addr(int channel, unsigned int *runcore)
{
    request_t *rd_ptr = NULL;
    LL_FOREACH(read_queue_head[channel], rd_ptr)
    {
        if(rd_ptr->request_id == per_thread_queue[runcore[channel]][channel]->next->request_id)
        {
            return rd_ptr->physical_address;
        }
    }
    return 0;
}

void fill_in_tracker(int channel, unsigned int *runcore, tracker_t ***per_thread_tracker)
{
    request_t *rd_ptr = NULL;
    int bank0_done =0;
    int bank1_done =0;
    int bank2_done =0;
    int bank3_done =0;
    int bank4_done =0;
    int bank5_done =0;
    int bank6_done =0;
    int bank7_done =0;
    LL_FOREACH(per_thread_queue[runcore[channel]][channel], rd_ptr)
    {
        if(bank0_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 0)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[0] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[0]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank0_done = 1;
            }
        }
        if(bank1_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 1)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[1] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[1]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank1_done = 1;
            }
        }
        if(bank2_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 2)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[2] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[2]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank2_done = 2;
            }
        }
        if(bank3_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 3)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[3] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[3]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank3_done = 1;
            }
        }
        if(bank4_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 4)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[4] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[4]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank4_done = 1;
            }
        }
        if(bank5_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 5)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[5] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[5]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank5_done = 1;
            }
        }
        if(bank6_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 6)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[6] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[6]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank6_done = 1;
            }
        }
        if(bank7_done == 0)
        {
            if(rd_ptr->dram_addr.bank == 7)
            {
                per_thread_tracker[runcore[channel]][channel]->tracker_i[7] = rd_ptr;
                per_thread_tracker[runcore[channel]][channel]->tracker_i[7]->valid = 1;
                per_thread_tracker[runcore[channel]][channel]->length++;
                bank7_done = 1;
            }
        }
    }
  
}

int random_num(int min_num, int max_num)
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

void scheduler_stats()
{
    unsigned long long int tempvar = 0;
    unsigned long long int tempvar2 = 0;
    unsigned long long int tempvar3 = 0;
    unsigned long long int uniquebankcount = 0;
    unsigned long long int ref_int = CYCLE_VAL/204800000;
    double ref_float = (((double)CYCLE_VAL)/204800000) > 0 ? (((double)CYCLE_VAL)/204800000) : 1;


    printf("M1SB\t : %f\n",((double)sbm1)/tem1);
    printf("Total_fake_number : %llu\n", prefetch_counter);
    
    for (int i = 0; i < NUM_CHANNELS; i++){
        for (int j = 0; j < NUM_RANKS; j++){
            tempvar2 = ref_counter[i][j] + tempvar2;
            for (int k = 0; k < NUM_BANKS; k++){
                tempvar3 += spillcounter_tracker[i][j][k];
                for(unsigned long long int u = 0; u <NUM_ROWS; u++){
                    if(unique[i][j][k][u] == 1){
                        uniquecount++;
                        uniquebankcount++;
                    }
                    ract_real[i][j][k][u] = ract_real[i][j][k][u] + ract_temp[i][j][k][u];
                }
                if(MITIGATION_MODE == 0){
                    printf("Unique_RowsC%dR%dB%d : %lf\n", i, j, k, ((double)uniquebankcount));
                }
                uniquebankcount = 0;
            }

        }
        free(new_sandbox_mode1[i]);
        tempvar = prob_tracker[i] + tempvar;
    }
    tempvar3 = tempvar3/NUM_BANKS;
     
    //printf("Total_Unique_Rows : %lf\n", ((double)uniquecount)/NUM_BANKS);
    printf("Bank_MG_Spill : %lf\n", ((double)tempvar3)/ref_float);
    printf("MG_ENTRIES : %llu\n", mg_entries);
    printf("TRH : %d\n", TRH);
    printf("Total_Mitigations : %lf\n", ((double)tempvar)/ref_float);
    
    /*for (int i = 0; i < NUM_CHANNELS; i++){
        for (int j = 0; j < NUM_RANKS; j++){
            for (int k = 0; k < NUM_BANKS; k++){
                for (unsigned long long int r = 0; r < NUM_ROWS; r++){
                    ract_real[i][j][k][r] = ((double)ract_real[i][j][k][r])/ref_float;
                }
            }
        }
    }*/

    unsigned long long int maxactstat = 0;
    for(int z = 0; z < 40; z++){
        maxactstat = 0;
        for (int i = 0; i < NUM_CHANNELS; i++){
            for (int j = 0; j < NUM_RANKS; j++){
                for (int k = 0; k < NUM_BANKS; k++){
                    for (unsigned long long int r = 0; r < NUM_ROWS; r++){
                        if(ract_real[i][j][k][r] > (100*(z+1))){
                            maxactstat++;    
                        }
                    }
                }
            }
        }
        printf("Above%d : %lf\n", 100*(z+1), ((double)maxactstat)/ref_float);
    }
    printf("\n\n");
    printf("CRAFILTER_ACCESSES  : %lld\n", filteraccesses);
    printf("CRAFILTER_HITS : %lld\n", filterhits);
    printf("CRAFILTER_HITRATE : %lf\n", (double)filterhits/filteraccesses);
    printf("\n\n");
    printf("CCACHE_LATENCY  : %d\n", CCLOOKUPLATENCY);
    printf("CCACHE_ACCESSES : %lld\n", ccaccess);
    printf("CCACHE_HIT      : %lld\n", cchit);
    printf("CCACHE_HITRATE  : %lf\n", ((double)cchit)/ccaccess);
    printf("RESETCC         : %lf\n", ((double)totalresetcc)/ref_float);
    printf("\n\n");
    printf("REFRESH_INTV    : %lf\n", ref_float);
    printf("SPARE_OCC       : %lf\n", ((double)spareoccupancy)/ref_float);
    printf("RISI            : %lf\n", ((double)risi)/ref_float);
    printf("RISV            : %lf\n", ((double)risv)/ref_float);
    printf("RVSI            : %lf\n", ((double)rvsi)/ref_float);
    printf("RVSV            : %lf\n", ((double)rvsv)/ref_float);
    printf("\n\n");

    /* Nothing to print for now. */
}
