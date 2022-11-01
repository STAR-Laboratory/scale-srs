#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "utlist.h"

#include "utils.h"

#include "params.h"
#include "memory_controller.h"
#include "processor.h"

// ROB Structure, used to release stall on instructions
// when the read request completes
extern struct robstructure *ROB;

// Current Processor Cycle
extern long long int CYCLE_VAL;

extern unsigned long long int queuing_delay_total; 
extern unsigned long long int readreq_total; 
extern unsigned long long int *queuing_delay; 
extern unsigned long long int *readreq; 


#define max(a, b) (((a) > (b)) ? (a) : (b))

#define BIG_ACTIVATION_WINDOW 1

// moving window that captures each activate issued in the past
int activation_record[MAX_NUM_CHANNELS][MAX_NUM_RANKS][BIG_ACTIVATION_WINDOW];

// record an activate in the activation record
void record_activate(int channel, int rank, long long int cycle)
{
    assert(activation_record[channel][rank][(cycle % BIG_ACTIVATION_WINDOW)] == 0); //can't have two commands issued the same cycle - hence no two activations in the same cycle
    activation_record[channel][rank][(cycle % BIG_ACTIVATION_WINDOW)] = 1;
    act_counter[channel][rank]++;

    return;
}

// Have there been 3 or less activates in the last T_FAW period
int is_T_FAW_met(int channel, int rank, int cycle)
{
    return 1;
}

// shift the moving window, clear out the past
void flush_activate_record(int channel, int rank, long long int cycle)
{
    activation_record[channel][rank][cycle % BIG_ACTIVATION_WINDOW] = 0;
}

// initialize dram variables and statistics
void init_memory_controller_vars()
{
    num_read_merge = 0;
    num_write_merge = 0;
    for (int i = 0; i < NUM_CHANNELS; i++)
    {

        for (int j = 0; j < NUM_RANKS; j++)
        {
            act_counter[i][j] = 0;
            ref_counter[i][j] = 0;

            for (int w = 0; w < BIG_ACTIVATION_WINDOW; w++)
                activation_record[i][j][w] = 0;

            for (int k = 0; k < NUM_BANKS; k++)
            {
                dram_state[i][j][k].state = IDLE;
                dram_state[i][j][k].active_row = -1;
                dram_state[i][j][k].next_pre = -1;
                dram_state[i][j][k].next_pre = -1;
                dram_state[i][j][k].next_pre = -1;
                dram_state[i][j][k].next_pre = -1;

                cmd_precharge_issuable[i][j][k] = 0;

                stats_num_activate_read[i][j][k] = 0;
                stats_num_activate_write[i][j][k] = 0;
                stats_num_activate_spec[i][j][k] = 0;
                stats_num_precharge[i][j][k] = 0;
                stats_num_read[i][j][k] = 0;
                stats_num_write[i][j][k] = 0;
                cas_issued_current_cycle[i][j][k] = 0;
                //Stats to figure out the proximity of subsequent accesses to the same row
                resetflag[i][j][k] = 0;
            }

            cmd_all_bank_precharge_issuable[i][j] = 0;
            cmd_powerdown_fast_issuable[i][j] = 0;
            cmd_powerdown_slow_issuable[i][j] = 0;
            cmd_powerup_issuable[i][j] = 0;
            cmd_refresh_issuable[i][j] = 0;

            next_refresh_completion_deadline[i][j] = 8 * T_REFI;
            last_refresh_completion_deadline[i][j] = 0;
            forced_refresh_mode_on[i][j] = 0;
            refresh_issue_deadline[i][j] = next_refresh_completion_deadline[i][j] - T_RP - 8 * T_RFC;
            num_issued_refreshes[i][j] = 0;

            stats_time_spent_in_active_power_down[i][j] = 0;
            stats_time_spent_in_precharge_power_down_slow[i][j] = 0;
            stats_time_spent_in_precharge_power_down_fast[i][j] = 0;
            last_activate[i][j] = 0;
            //If average_gap_between_activates is 0 then we know that there have been no activates to [i][j]
            average_gap_between_activates[i][j] = 0;

            stats_num_powerdown_slow[i][j] = 0;
            stats_num_powerdown_fast[i][j] = 0;
            stats_num_powerup[i][j] = 0;

            stats_num_activate[i][j] = 0;

            command_issued_current_cycle[i] = 0;
        }

        read_queue_head[i] = NULL;
        write_queue_head[i] = NULL;

        read_queue_length[i] = 0;
        write_queue_length[i] = 0;

        command_issued_current_cycle[i] = 0;

        // Stats
        stats_reads_merged_per_channel[i] = 0;
        stats_writes_merged_per_channel[i] = 0;

        stats_reads_seen[i] = 0;
        stats_writes_seen[i] = 0;
        stats_reads_completed[i] = 0;
        stats_writes_completed[i] = 0;
        stats_average_read_latency[i] = 0;
        stats_average_read_queue_latency[i] = 0;
        stats_average_write_latency[i] = 0;
        stats_average_write_queue_latency[i] = 0;
        stats_page_hits[i] = 0;
        stats_read_row_hit_rate[i] = 0;
    }
    activates_for_reads_t = 0;
    activates_for_spec_t = 0;
    activates_for_writes_t = 0;
    read_cmds_t = 0;
    write_cmds_t = 0;

}

/********************************************************/
/*	Utility Functions				*/
/********************************************************/

unsigned int log_base2(unsigned int new_value)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        new_value >>= 1;
        if (new_value == 0)
            break;
    }
    return i;
}

// Function to decompose the incoming DRAM address into the
// constituent channel, rank, bank, row and column ids.
// Note : To prevent memory leaks, call free() on the pointer returned
// by this function after you have used the return value.
dram_address_t *calc_dram_addr(long long int physical_address)
{

    long long int input_a, temp_b, temp_a;

    int channelBitWidth = log_base2(NUM_CHANNELS);
    int rankBitWidth = log_base2(NUM_RANKS);
    int bankBitWidth = log_base2(NUM_BANKS);
    int rowBitWidth = log_base2(NUM_ROWS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);

    dram_address_t *this_a = (dram_address_t *)malloc(sizeof(dram_address_t));

    this_a->actual_address = physical_address;

    input_a = physical_address;

    input_a = input_a >> byteOffsetWidth; // strip out the cache_offset

    if (ADDRESS_MAPPING == 1)
    {
        temp_b = input_a;
        input_a = input_a >> colBitWidth;
        temp_a = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b; //strip out the column address

        temp_b = input_a;
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address

        temp_b = input_a;
        input_a = input_a >> bankBitWidth;
        temp_a = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b; // strip out the bank address

        temp_b = input_a;
        input_a = input_a >> rankBitWidth;
        temp_a = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b; // strip out the rank address

        temp_b = input_a;
        input_a = input_a >> rowBitWidth;
        temp_a = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b; // strip out the row number
    } else if (ADDRESS_MAPPING == 2) // Random address mapping
    {
        temp_b = input_a; 
        input_a = input_a >> bankBitWidth;
        temp_a = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b; // strip out the bank address

        temp_b = input_a; 
        input_a = input_a >> rankBitWidth;
        temp_a = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b; // strip out the rank address

        temp_b = input_a; 
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address

        temp_b = input_a; 
        input_a = input_a >> rowBitWidth;
        temp_a = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b; // strip out the row number

        temp_b = input_a; 
        input_a = input_a >> colBitWidth;
        temp_a = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b; //strip out the column address
    }
    else
    {
        temp_b = input_a;
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address

        temp_b = input_a;
        input_a = input_a >> bankBitWidth;
        temp_a = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b; // strip out the bank address

        temp_b = input_a;
        input_a = input_a >> rankBitWidth;
        temp_a = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b; // strip out the rank address

        temp_b = input_a;
        input_a = input_a >> colBitWidth;
        temp_a = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b; //strip out the column address

        temp_b = input_a;
        input_a = input_a >> rowBitWidth;
        temp_a = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b; // strip out the row number
    }
    return (this_a);
}

dram_address_t *calc_fake_dram_addr(long long int physical_address)
{
    long long int input_a, temp_b, temp_a;

    int channelBitWidth = log_base2(NUM_CHANNELS);
    int rankBitWidth = log_base2(NUM_RANKS);
    int bankBitWidth = log_base2(NUM_BANKS);
    int rowBitWidth = log_base2(NUM_ROWS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);

    dram_address_t *this_a = (dram_address_t *)malloc(sizeof(dram_address_t));

    this_a->actual_address = physical_address;

    input_a = physical_address;

    input_a = input_a >> byteOffsetWidth; // strip out the cache_offset

    temp_b = input_a;
    input_a = input_a >> colBitWidth;
    temp_a = input_a << colBitWidth;
    this_a->column = temp_a ^ temp_b; //strip out the column address

    temp_b = input_a;
    input_a = input_a >> channelBitWidth;
    temp_a = input_a << channelBitWidth;
    this_a->channel = temp_a ^ temp_b; // strip out the channel address

    temp_b = input_a;
    input_a = input_a >> bankBitWidth;
    temp_a = input_a << bankBitWidth;
    this_a->bank = temp_a ^ temp_b; // strip out the bank address

    temp_b = input_a;
    input_a = input_a >> rankBitWidth;
    temp_a = input_a << rankBitWidth;
    this_a->rank = temp_a ^ temp_b; // strip out the rank address

    temp_b = input_a;
    input_a = input_a >> rowBitWidth;
    temp_a = input_a << rowBitWidth;
    this_a->row = temp_a ^ temp_b; // strip out the row number

    return (this_a);

}

// Function to decompose the incoming DRAM address into the
// constituent channel, rank, bank, row and column ids.
// Note : To prevent memory leaks, call free() on the pointer returned
// by this function after you have used the return value.
int check_dram_addr(long long int physical_address, int channel)
{

    long long int input_a, temp_b, temp_a;

    int channelBitWidth = log_base2(NUM_CHANNELS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);

    dram_address_t *this_a = (dram_address_t *)malloc(sizeof(dram_address_t));

    this_a->actual_address = physical_address;

    input_a = physical_address;

    input_a = input_a >> byteOffsetWidth; // strip out the cache_offset

    if (ADDRESS_MAPPING == 1)
    {
        temp_b = input_a;
        input_a = input_a >> colBitWidth;
        temp_a = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b; //strip out the column address

        temp_b = input_a;
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address

        if(this_a->channel == channel)
            return 0;
        else
            return 1;
    }
    else
    {
        temp_b = input_a;
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address

        if(this_a->channel == channel)
            return 0;
        else
            return 1;
    }
}

// Get the channel address
int get_dram_channel_addr(long long int physical_address)
{

    long long int input_a, temp_b, temp_a;

    int channelBitWidth = log_base2(NUM_CHANNELS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);

    dram_address_t *this_a = (dram_address_t *)malloc(sizeof(dram_address_t));

    this_a->actual_address = physical_address;

    input_a = physical_address;

    input_a = input_a >> byteOffsetWidth; // strip out the cache_offset

    if (ADDRESS_MAPPING == 1)
    {
        temp_b = input_a;
        input_a = input_a >> colBitWidth;
        temp_a = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b; //strip out the column address

        temp_b = input_a;
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address
        return this_a->channel;
    }
    else
    {
        temp_b = input_a;
        input_a = input_a >> channelBitWidth;
        temp_a = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; // strip out the channel address

        return this_a->channel;
    }
}


// Function to create a new request node to be inserted into the read
// or write queue.
void *init_new_node(long long int physical_address, long long int arrival_time, optype_t type, int thread_id, int instruction_id, long long int instruction_pc, unsigned long long int request_id, long long int inter_arrival_time, int fake, int valid)
{
    request_t *new_node = NULL;

    new_node = (request_t *)malloc(sizeof(request_t));

    if (new_node == NULL)
    {
        fprintf(stderr, "FATAL : Malloc Error\n");

        exit(-1);
    }
    else
    {

        new_node->physical_address = physical_address;

        new_node->arrival_time = arrival_time;

        new_node->dispatch_time = -100;

        new_node->completion_time = -100;

        new_node->latency = -100;

        new_node->thread_id = thread_id;

        new_node->next_command = NOP;

        new_node->command_issuable = 0;

        new_node->operation_type = type;

        new_node->request_served = 0;

        new_node->instruction_id = instruction_id;

        new_node->instruction_pc = instruction_pc;

        new_node->request_id = request_id;

        new_node->next = NULL;

        new_node->fake = fake;

        new_node->valid = valid;

        new_node->inter_arrival_time = inter_arrival_time;

        new_node->delay = 0;

        new_node->tracker = 0;

        if(new_node->fake == 0) {
            dram_address_t *this_node_addr = calc_dram_addr(physical_address);

            new_node->dram_addr.actual_address = physical_address;
            new_node->dram_addr.channel = this_node_addr->channel;
            new_node->dram_addr.rank = this_node_addr->rank;
            new_node->dram_addr.bank = this_node_addr->bank;
            new_node->dram_addr.row = this_node_addr->row;
            new_node->dram_addr.column = this_node_addr->column;
            free(this_node_addr);
        }else{
            dram_address_t *fake_node_addr = calc_dram_addr(physical_address);

            new_node->dram_addr.actual_address = physical_address;
            new_node->dram_addr.channel = fake_node_addr->channel;
            new_node->dram_addr.rank = fake_node_addr->rank;
            new_node->dram_addr.bank = fake_node_addr->bank;
            new_node->dram_addr.row = fake_node_addr->row;
            new_node->dram_addr.column = fake_node_addr->column;
            free(fake_node_addr);
        }
        
        new_node->user_ptr = NULL;
        new_node->reqptr = NULL;

        return (new_node);
    }
}

void *init_new_id_node(long long int physical_address, int thread_id, unsigned long long int request_id, int fake, int valid)
{

    request_t *new_node = NULL;

    new_node = (request_t *)malloc(sizeof(request_t));

    if (new_node == NULL)
    {
        fprintf(stderr, "FATAL : Malloc Error\n");

        exit(-1);
    }
    else
    {
        new_node->physical_address = 0;

        new_node->arrival_time = -100;

        new_node->dispatch_time = -100;

        new_node->completion_time = -100;

        new_node->latency = -100;

        new_node->thread_id = thread_id;

        new_node->next_command = NOP;

        new_node->command_issuable = 0;

        new_node->operation_type = 0;

        new_node->request_served = 0;

        new_node->instruction_id = 0;

        new_node->instruction_pc = 0;

        new_node->request_id = request_id;

        new_node->next = NULL;

        new_node->fake = fake;

        new_node->valid = valid;

        new_node->user_ptr = NULL;

        new_node->reqptr = NULL;

        new_node->delay = 0;

        new_node->tracker = 0;
        
        dram_address_t *fake_node_addr = calc_fake_dram_addr(physical_address);

        new_node->dram_addr.actual_address = physical_address;
        new_node->dram_addr.channel = fake_node_addr->channel;
        new_node->dram_addr.rank = fake_node_addr->rank;
        new_node->dram_addr.bank = fake_node_addr->bank;
        new_node->dram_addr.row = fake_node_addr->row;
        new_node->dram_addr.column = fake_node_addr->column;
        free(fake_node_addr);

        // printf("new node bank %d\n", new_node->dram_addr.bank);
        return (new_node);
    }
}

// Function that checks to see if an incoming read can be served by a
// write request pending in the write queue and return
// WQ_LOOKUP_LATENCY if there is a match. Also the function goes over
// the read_queue to see if there is a pending read for the s// address and avoids duplication. The 2nd read is assumed to be
// serviced when the original request completes.

#define RQ_LOOKUP_LATENCY 1
int read_matches_write_or_read_queue(long long int physical_address)
{
    //get channel info
    dram_address_t *this_addr = calc_dram_addr(physical_address);
    int channel = this_addr->channel;
    free(this_addr);

    request_t *wr_ptr = NULL;
    request_t *rd_ptr = NULL;

    LL_FOREACH(write_queue_head[channel], wr_ptr)
    {
        if ((wr_ptr->dram_addr.actual_address == physical_address) && (wr_ptr->completion_time == -100)) 
        {
            num_read_merge++;
            stats_reads_merged_per_channel[channel]++;
            return WQ_LOOKUP_LATENCY;
        }
    }

    LL_FOREACH(read_queue_head[channel], rd_ptr)
    {
        if ((rd_ptr->dram_addr.actual_address == physical_address) && (rd_ptr->completion_time == -100))
        {
            num_read_merge++;
            stats_reads_merged_per_channel[channel]++;
            return RQ_LOOKUP_LATENCY;
        }
    }
    return 0;
}

// Function to merge writes to the same address
int write_exists_in_write_queue(long long int physical_address)
{
    //get channel info
    dram_address_t *this_addr = calc_dram_addr(physical_address);
    int channel = this_addr->channel;
    free(this_addr);

    request_t *wr_ptr = NULL;

    LL_FOREACH(write_queue_head[channel], wr_ptr)
    {
        if ((wr_ptr->dram_addr.actual_address == physical_address) && (wr_ptr->completion_time == -100))
        {
            num_write_merge++;
            stats_writes_merged_per_channel[channel]++;
            return 1;
        }
    }
    return 0;
}

// a new read to the read queue
request_t *insert_read(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, long long int instruction_pc, unsigned long long int request_id, long long int inter_arrival_time, int fake, int valid)
{

    optype_t this_op = READ;

    //get channel info
    dram_address_t *this_addr = calc_dram_addr(physical_address);
    int channel = this_addr->channel;
    free(this_addr);

    stats_reads_seen[channel]++;

    request_t *new_node = init_new_node(physical_address, arrival_time, this_op, thread_id, instruction_id, instruction_pc, request_id, inter_arrival_time, fake, valid);

    LL_APPEND(read_queue_head[channel], new_node);

    if(fake == 4){
        read_queue_length[channel] += 1;
        write_queue_length[channel] += 1;
    }
    else{
        read_queue_length[channel] += 1;
    }

    UT_MEM_DEBUG("\nCyc: %lld New READ:%lld Core:%d Chan:%d Rank:%d Bank:%d Row:%lld RD_Q_Length:%lld\n", CYCLE_VAL, new_node->id, new_node->thread_id, new_node->dram_addr.channel, new_node->dram_addr.rank, new_node->dram_addr.bank, new_node->dram_addr.row, read_queue_length[channel]);

    return new_node;
}

request_t *insert_requestID(long long int physical_address, int thread_id, unsigned long long int request_id, int fake, request_t *readreq, int valid)
{

    dram_address_t *this_addr = calc_dram_addr(physical_address);
    request_t *temp_ptr = NULL;

    int channel = this_addr->channel;
    free(this_addr);

    request_t *new_id_node = init_new_id_node(physical_address, thread_id, request_id, fake, valid);
    temp_ptr = readreq;
    new_id_node->reqptr = temp_ptr;

    LL_APPEND(per_thread_queue[thread_id][channel], new_id_node);

    per_thread_queue_length[thread_id][channel]++;
    assert(temp_ptr != NULL);

    //printf("\nCyc: %lld Chan:%d \n", CYCLE_VAL, temp_ptr->dram_addr.channel);
    return new_id_node;
}

request_t *insert_requestID_prefetch(int channel, int thread_id, unsigned long long int request_id, int fake, request_t *readreq)
{

    //dram_address_t *this_addr = calc_dram_addr(physical_address);
    request_t *temp_ptr = NULL;

    //int channel = this_addr->channel;
    //free(this_addr);

    request_t *new_id_node = init_new_id_node(0, thread_id, request_id, fake, 0);
    temp_ptr = readreq;
    new_id_node->reqptr = temp_ptr;

    LL_APPEND(per_thread_queue[thread_id][channel], new_id_node);

    per_thread_queue_length[thread_id][channel]++;
    assert(temp_ptr != NULL);

    //printf("\nCyc: %lld Chan:%d \n", CYCLE_VAL, temp_ptr->dram_addr.channel);
    return new_id_node;
}

// Insert a new write to the write queue
request_t *insert_write(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, int fake)
{
    optype_t this_op = WRITE;

    dram_address_t *this_addr = calc_dram_addr(physical_address);
    int channel = this_addr->channel;
    free(this_addr);

    stats_writes_seen[channel]++;

    request_t *new_node = init_new_node(physical_address, arrival_time, this_op, thread_id, instruction_id, 0, 0, 0, fake, 0);

    LL_APPEND(write_queue_head[channel], new_node);

    if(fake == 2){
        write_queue_length[channel] += 1;
        read_queue_length[channel] += 1;
    }
    else{
        write_queue_length[channel] += 1;
    }

    UT_MEM_DEBUG("\nCyc: %lld New WRITE:%lld Core:%d Chan:%d Rank:%d Bank:%d Row:%lld WR_Q_Length:%lld\n", CYCLE_VAL, new_node->id, new_node->thread_id, new_node->dram_addr.channel, new_node->dram_addr.rank, new_node->dram_addr.bank, new_node->dram_addr.row, write_queue_length[channel]);
    return new_node;
}

// Function to update the states of the read queue requests.
// Each DRAM cycle, this function iterates over the read queue and
// updates the next_command and command_issuable fields to mark which
// commands can be issued this cycle
void update_read_queue_commands(int channel)
{
    request_t *curr = NULL;

    LL_FOREACH(read_queue_head[channel], curr)
    {
        // ignore the requests whose completion time has been determined
        // these requests will be removed this very cycle
        if (curr->request_served == 1)
            continue;

        int bank = curr->dram_addr.bank;

        int rank = curr->dram_addr.rank;

        int row = curr->dram_addr.row;

        switch (dram_state[channel][rank][bank].state)
        {
            // if the DRAM bank has no rows open and the chip is
            // powered up, the next command for the request
            // should be ACT.
            case IDLE:
            case PRECHARGING:
            case REFRESHING:

                curr->next_command = ACT_CMD;

                if (CYCLE_VAL >= dram_state[channel][rank][bank].next_act && is_T_FAW_met(channel, rank, CYCLE_VAL))
                    curr->command_issuable = 1;
                else
                    curr->command_issuable = 0;

                // check if we are in OR too close to the forced refresh period
                if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_RAS) > refresh_issue_deadline[channel][rank]))
                    curr->command_issuable = 0;
                break;

            case ROW_ACTIVE:

                // if the bank is active then check if this is a row-hit or not
                // If the request is to the currently
                // opened row, the next command should
                // be a COL_RD, else it should be a
                // PRECHARGE
                if (row == dram_state[channel][rank][bank].active_row)
                {
                    curr->next_command = COL_READ_CMD;

                    if (CYCLE_VAL >= dram_state[channel][rank][bank].next_read)
                        curr->command_issuable = 1;
                    else
                        curr->command_issuable = 0;

                    if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_RTP) > refresh_issue_deadline[channel][rank]))
                        curr->command_issuable = 0;
                }
                else
                {
                    curr->next_command = PRE_CMD;

                    if (CYCLE_VAL >= dram_state[channel][rank][bank].next_pre)
                        curr->command_issuable = 1;
                    else
                        curr->command_issuable = 0;

                    if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_RP) > refresh_issue_deadline[channel][rank]))
                        curr->command_issuable = 0;
                }

                break;
                // if the chip was powered, down the
                // next command required is power_up

            case PRECHARGE_POWER_DOWN_SLOW:
            case PRECHARGE_POWER_DOWN_FAST:
            case ACTIVE_POWER_DOWN:

                curr->next_command = PWR_UP_CMD;

                if (CYCLE_VAL >= dram_state[channel][rank][bank].next_powerup)
                    curr->command_issuable = 1;
                else
                    curr->command_issuable = 0;

                if ((dram_state[channel][rank][bank].state == PRECHARGE_POWER_DOWN_SLOW) && ((CYCLE_VAL + T_XP_DLL) > refresh_issue_deadline[channel][rank]))
                    curr->command_issuable = 0;
                else if (((dram_state[channel][rank][bank].state == PRECHARGE_POWER_DOWN_FAST) || (dram_state[channel][rank][bank].state == ACTIVE_POWER_DOWN)) && ((CYCLE_VAL + T_XP) > refresh_issue_deadline[channel][rank]))
                    curr->command_issuable = 0;

                break;

            default:
                break;
        }
    }
}

// Similar to update_read_queue above, but for write queue
void update_write_queue_commands(int channel)
{
    request_t *curr = NULL;

    LL_FOREACH(write_queue_head[channel], curr)
    {

        if (curr->request_served == 1)
            continue;

        int bank = curr->dram_addr.bank;

        int rank = curr->dram_addr.rank;

        int row = curr->dram_addr.row;

        switch (dram_state[channel][rank][bank].state)
        {
            case IDLE:
            case PRECHARGING:
            case REFRESHING:
                curr->next_command = ACT_CMD;

                if (CYCLE_VAL >= dram_state[channel][rank][bank].next_act && is_T_FAW_met(channel, rank, CYCLE_VAL))
                    curr->command_issuable = 1;
                else
                    curr->command_issuable = 0;

                // check if we are in or too close to the forced refresh period
                if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_RAS) > refresh_issue_deadline[channel][rank]))
                    curr->command_issuable = 0;

                break;

            case ROW_ACTIVE:

                if (row == dram_state[channel][rank][bank].active_row)
                {
                    if(curr->fake < 2){
                        curr->next_command = COL_WRITE_CMD;

                        if (CYCLE_VAL >= dram_state[channel][rank][bank].next_write)
                            curr->command_issuable = 1;
                        else
                            curr->command_issuable = 0;

                        if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_CWD + T_DATA_TRANS + T_WR) > refresh_issue_deadline[channel][rank]))
                            curr->command_issuable = 0;
                    }
                    else{
                        curr->next_command = COL_READ_CMD;

                        if (CYCLE_VAL >= dram_state[channel][rank][bank].next_read)
                            curr->command_issuable = 1;
                        else
                            curr->command_issuable = 0;

                        if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_RTP) > refresh_issue_deadline[channel][rank]))
                            curr->command_issuable = 0;
                    }
                }
                else
                {
                    curr->next_command = PRE_CMD;

                    if (CYCLE_VAL >= dram_state[channel][rank][bank].next_pre)
                        curr->command_issuable = 1;
                    else
                        curr->command_issuable = 0;

                    if (forced_refresh_mode_on[channel][rank] || ((CYCLE_VAL + T_RP) > refresh_issue_deadline[channel][rank]))
                        curr->command_issuable = 0;
                }

                break;

            case PRECHARGE_POWER_DOWN_SLOW:
            case PRECHARGE_POWER_DOWN_FAST:
            case ACTIVE_POWER_DOWN:

                curr->next_command = PWR_UP_CMD;

                if (CYCLE_VAL >= dram_state[channel][rank][bank].next_powerup)
                    curr->command_issuable = 1;
                else
                    curr->command_issuable = 0;

                if (forced_refresh_mode_on[channel][rank])
                    curr->command_issuable = 0;

                if ((dram_state[channel][rank][bank].state == PRECHARGE_POWER_DOWN_SLOW) && ((CYCLE_VAL + T_XP_DLL) > refresh_issue_deadline[channel][rank]))
                    curr->command_issuable = 0;
                else if (((dram_state[channel][rank][bank].state == PRECHARGE_POWER_DOWN_FAST) || (dram_state[channel][rank][bank].state == ACTIVE_POWER_DOWN)) && ((CYCLE_VAL + T_XP) > refresh_issue_deadline[channel][rank]))
                    curr->command_issuable = 0;

                break;

            default:
                break;
        }
    }
}

// Remove finished requests from temp queues
void clean_temp_queues(int channel, int core)
{

    request_t *rd_ptr = NULL;
    request_t *rd_tmp = NULL;
    request_t *temp_ptr = NULL; 
    // Delete all READ requests whose completion time has been determined i.e. COL_RD has been issued
    LL_FOREACH_SAFE(per_thread_queue[core][channel], rd_ptr, rd_tmp)
    {
        temp_ptr = rd_ptr->reqptr;
        if (temp_ptr->request_served == 1)
        {
            //assert(temp_ptr->next_command == COL_READ_CMD);
            LL_DELETE(per_thread_queue[core][channel], rd_ptr);

            free(rd_ptr);

            per_thread_queue_length[core][channel]--;
            assert(core == rd_ptr->thread_id);

            assert(per_thread_queue_length[core][channel] >= 0);
        }
    }
}

// Remove finished requests from the queues.
void clean_queues(int channel)
{

    request_t *rd_ptr = NULL;
    request_t *rd_tmp = NULL;
    request_t *wrt_ptr = NULL;
    request_t *wrt_tmp = NULL;

    // Delete all READ requests whose completion time has been determined i.e. COL_RD has been issued
    LL_FOREACH_SAFE(read_queue_head[channel], rd_ptr, rd_tmp)
    {
        if (rd_ptr->request_served == 1)
        {
            //assert(rd_ptr->next_command == COL_READ_CMD);
            if (rd_ptr->fake == 0)
                assert(rd_ptr->completion_time != -100);

            LL_DELETE(read_queue_head[channel], rd_ptr);
            
            if(rd_ptr->fake == 4){
                read_queue_length[channel] -= 1;
                write_queue_length[channel] -= 1;
            }
            else{
                read_queue_length[channel] -= 1;
            }

            if (rd_ptr->user_ptr)
                free(rd_ptr->user_ptr);

            free(rd_ptr);

            // printf("read queue length %d\n", read_queue_length[channel]);
            assert(read_queue_length[channel] >= 0);
        }
    }

    // Delete all WRITE requests whose completion time has been determined i.e COL_WRITE has been issued
    LL_FOREACH_SAFE(write_queue_head[channel], wrt_ptr, wrt_tmp)
    {
        if (wrt_ptr->request_served == 1)
        {
            assert(wrt_ptr->next_command == COL_WRITE_CMD);

            LL_DELETE(write_queue_head[channel], wrt_ptr);

            if(wrt_ptr->fake == 2){
                write_queue_length[channel] -= 1;
                read_queue_length[channel] -= 1;
            }
            else{
                write_queue_length[channel] -= 1;
            }
            
            if (wrt_ptr->user_ptr)
                free(wrt_ptr->user_ptr);

            free(wrt_ptr);

           
            assert(write_queue_length[channel] >= 0);
        }
    }
}

// This affects state change
// Issue a valid command for a request in either the read or write
// queue.
// Upon issuing the request, the dram_state is changed and the
// next_"cmd" variables are updated to indicate when the next "cmd"
// can be issued to each bank
int issue_request_command(request_t *request)
{
    long long int cycle = CYCLE_VAL;
    if ((request->command_issuable != 1) || command_issued_current_cycle[request->dram_addr.channel]) 
    {
        printf("PANIC: SCHED_ERROR : Command for request selected can not be issued in  cycle:%lld.\n", CYCLE_VAL);
        return 0;
    }

    int channel = request->dram_addr.channel;
    int rank = request->dram_addr.rank;
    int bank = request->dram_addr.bank;
    long long int row = request->dram_addr.row;
    command_t cmd = request->next_command;

    switch (cmd)
    {
        case ACT_CMD:

            assert(dram_state[channel][rank][bank].state == PRECHARGING || dram_state[channel][rank][bank].state == IDLE || dram_state[channel][rank][bank].state == REFRESHING);

            //UT_MEM_DEBUG("\nCycle: %lld Cmd:ACT Req:%lld Chan:%d Rank:%d Bank:%d Row:%lld\n", CYCLE_VAL, request->id, channel, rank, bank, row);

            // open row
            dram_state[channel][rank][bank].state = ROW_ACTIVE;

            dram_state[channel][rank][bank].active_row = row;

            dram_state[channel][rank][bank].next_pre = max((cycle + T_RAS), dram_state[channel][rank][bank].next_pre);

            dram_state[channel][rank][bank].next_refresh = max((cycle + T_RAS), dram_state[channel][rank][bank].next_refresh);

            dram_state[channel][rank][bank].next_read = max(cycle + T_RCD, dram_state[channel][rank][bank].next_read);

            dram_state[channel][rank][bank].next_write = max(cycle + T_RCD, dram_state[channel][rank][bank].next_write);

            dram_state[channel][rank][bank].next_act = max(cycle + T_RC, dram_state[channel][rank][bank].next_act);

            dram_state[channel][rank][bank].next_powerdown = max(cycle + T_RCD, dram_state[channel][rank][bank].next_powerdown);

            for (int i = 0; i < NUM_BANKS; i++)
                if (i != bank)
                    dram_state[channel][rank][i].next_act = max(cycle + T_RRD, dram_state[channel][rank][i].next_act);

            record_activate(channel, rank, cycle);

            if (request->operation_type == READ)
                stats_num_activate_read[channel][rank][bank]++;
            else
                stats_num_activate_write[channel][rank][bank]++;

            stats_num_activate[channel][rank]++;

            average_gap_between_activates[channel][rank] = ((average_gap_between_activates[channel][rank] * (stats_num_activate[channel][rank] - 1)) + (CYCLE_VAL - last_activate[channel][rank])) / stats_num_activate[channel][rank];

            last_activate[channel][rank] = CYCLE_VAL;
            request->start_time = CYCLE_VAL;

            command_issued_current_cycle[channel] = 1;
            break;

        case COL_READ_CMD:

            assert(dram_state[channel][rank][bank].state == ROW_ACTIVE);

            dram_state[channel][rank][bank].next_pre = max(cycle + T_RTP, dram_state[channel][rank][bank].next_pre);

            dram_state[channel][rank][bank].next_refresh = max(cycle + T_RTP, dram_state[channel][rank][bank].next_refresh);

            dram_state[channel][rank][bank].next_powerdown = max(cycle + T_RTP, dram_state[channel][rank][bank].next_powerdown);

            for (int i = 0; i < NUM_RANKS; i++)
            {
                for (int j = 0; j < NUM_BANKS; j++)
                {
                    if (i != rank)
                        dram_state[channel][i][j].next_read = max(cycle + T_DATA_TRANS + T_RTRS, dram_state[channel][i][j].next_read);

                    else
                        dram_state[channel][i][j].next_read = max(cycle + max(T_CCD, T_DATA_TRANS), dram_state[channel][i][j].next_read);

                    dram_state[channel][i][j].next_write = max(cycle + T_CAS + T_DATA_TRANS + T_RTRS - T_CWD, dram_state[channel][i][j].next_write);
                }
            }

            // set the completion time of this read request
            // in the ROB and the controller queue.
            request->dispatch_time = CYCLE_VAL;

            if(request->fake == 4){
                request->completion_time = CYCLE_VAL + (T_CAS + T_WTR) + (2*T_DATA_TRANS);
            }
            else{
                request->completion_time = CYCLE_VAL + T_CAS + T_DATA_TRANS;
            }

            if(MITIGATION_MODE > 0){
                if((request->dispatch_time - request->arrival_time) <= CCLOOKUPLATENCY){
                    request->completion_time = request->completion_time + (CCLOOKUPLATENCY - (request->dispatch_time - request->arrival_time));
                }
            }

            request->latency = request->completion_time - request->arrival_time;
            request->request_served = 1;

            if(request->fake == 0){
                queuing_delay_total = (request->dispatch_time - request->arrival_time) + queuing_delay_total;
                queuing_delay[request->thread_id] = (request->dispatch_time - request->arrival_time) + queuing_delay[request->thread_id];
                readreq_total++;
                readreq[request->thread_id]++;
            }

            //printf("%llu\n",request->latency);

            // update the ROB with the completion time
            if (request->fake == 0){
                ROB[request->thread_id].entry[request->instruction_id].comptime = request->completion_time + PIPELINEDEPTH;
            }

            if(request->fake == 2){
                request->fake = 1;
                request->request_served = 0;
            }

            stats_reads_completed[channel]++;
            stats_average_read_latency[channel] = ((stats_reads_completed[channel] - 1) * stats_average_read_latency[channel] + request->latency) / stats_reads_completed[channel];
            stats_average_read_queue_latency[channel] = ((stats_reads_completed[channel] - 1) * stats_average_read_queue_latency[channel] + (request->dispatch_time - request->arrival_time)) / stats_reads_completed[channel];
            //UT_MEM_DEBUG("Req:%lld finishes at Cycle: %lld\n", request->id, request->completion_time);

            //printf("Cycle: %10lld, Reads  Completed = %5lld, this_latency= %5lld, latency = %f\n", CYCLE_VAL, stats_reads_completed[channel], request->latency, stats_average_read_latency[channel]);

            stats_num_read[channel][rank][bank]++;

            for (int i = 0; i < NUM_RANKS; i++)
            {
                if (i != rank)
                    stats_time_spent_terminating_reads_from_other_ranks[channel][i] += T_DATA_TRANS;
            }

            command_issued_current_cycle[channel] = 1;
            cas_issued_current_cycle[channel][rank][bank] = 1;
            break;

        case COL_WRITE_CMD:

            assert(dram_state[channel][rank][bank].state == ROW_ACTIVE);

            //UT_MEM_DEBUG("\nCycle: %lld Cmd: COL_WRITE Req:%lld Chan:%d Rank:%d Bank:%d \n", CYCLE_VAL, request->id, channel, rank, bank);

            dram_state[channel][rank][bank].next_pre = max(cycle + T_CWD + T_DATA_TRANS + T_WR, dram_state[channel][rank][bank].next_pre);

            dram_state[channel][rank][bank].next_refresh = max(cycle + T_CWD + T_DATA_TRANS + T_WR, dram_state[channel][rank][bank].next_refresh);

            dram_state[channel][rank][bank].next_powerdown = max(cycle + T_CWD + T_DATA_TRANS + T_WR, dram_state[channel][rank][bank].next_powerdown);

            for (int i = 0; i < NUM_RANKS; i++)
            {
                for (int j = 0; j < NUM_BANKS; j++)
                {
                    if (i != rank)
                    {
                        dram_state[channel][i][j].next_write = max(cycle + T_DATA_TRANS + T_RTRS, dram_state[channel][i][j].next_write);

                        dram_state[channel][i][j].next_read = max(cycle + T_CWD + T_DATA_TRANS + T_RTRS - T_CAS, dram_state[channel][i][j].next_read);
                    }
                    else
                    {
                        dram_state[channel][i][j].next_write = max(cycle + max(T_CCD, T_DATA_TRANS), dram_state[channel][i][j].next_write);

                        dram_state[channel][i][j].next_read = max(cycle + T_CWD + T_DATA_TRANS + T_WTR, dram_state[channel][i][j].next_read);
                    }
                }
            }

            request->dispatch_time = CYCLE_VAL;
            // set the completion time of this write request
            if(request->fake == 1){
                request->completion_time = CYCLE_VAL + (2*T_DATA_TRANS) + T_WR;
            }
            else{
                request->completion_time = CYCLE_VAL + T_DATA_TRANS + T_WR;
            }

            if(MITIGATION_MODE > 0){
                if((request->dispatch_time - request->arrival_time) <= CCLOOKUPLATENCY){
                    request->completion_time = request->completion_time + (CCLOOKUPLATENCY - (request->dispatch_time - request->arrival_time));
                }
            }

            request->latency = request->completion_time - request->arrival_time;
            request->request_served = 1;

            stats_writes_completed[channel]++;

            stats_num_write[channel][rank][bank]++;

            stats_average_write_latency[channel] = ((stats_writes_completed[channel] - 1) * stats_average_write_latency[channel] + request->latency) / stats_writes_completed[channel];
            stats_average_write_queue_latency[channel] = ((stats_writes_completed[channel] - 1) * stats_average_write_queue_latency[channel] + (request->dispatch_time - request->arrival_time)) / stats_writes_completed[channel];
            //UT_MEM_DEBUG("Req:%lld finishes at Cycle: %lld\n", request->id, request->completion_time);

            //printf("Cycle: %10lld, Writes Completed = %5lld, this_latency= %5lld, latency = %f\n", CYCLE_VAL, stats_writes_completed[channel], request->latency, stats_average_write_latency[channel]);

            for (int i = 0; i < NUM_RANKS; i++)
            {
                if (i != rank)
                    stats_time_spent_terminating_writes_to_other_ranks[channel][i] += T_DATA_TRANS;
            }

            command_issued_current_cycle[channel] = 1;
            cas_issued_current_cycle[channel][rank][bank] = 2;
            break;

        case PRE_CMD:

            assert(dram_state[channel][rank][bank].state == ROW_ACTIVE || dram_state[channel][rank][bank].state == PRECHARGING || dram_state[channel][rank][bank].state == IDLE || dram_state[channel][rank][bank].state == REFRESHING);

            //UT_MEM_DEBUG("\nCycle: %lld Cmd:PRE Req:%lld Chan:%d Rank:%d Bank:%d \n", CYCLE_VAL, request->id, channel, rank, bank);

            dram_state[channel][rank][bank].state = PRECHARGING;

            dram_state[channel][rank][bank].active_row = -1;

            dram_state[channel][rank][bank].next_act = max(cycle + T_RP, dram_state[channel][rank][bank].next_act);

            dram_state[channel][rank][bank].next_powerdown = max(cycle + T_RP, dram_state[channel][rank][bank].next_powerdown);

            dram_state[channel][rank][bank].next_pre = max(cycle + T_RP, dram_state[channel][rank][bank].next_pre);

            dram_state[channel][rank][bank].next_refresh = max(cycle + T_RP, dram_state[channel][rank][bank].next_refresh);

            stats_num_precharge[channel][rank][bank]++;

            command_issued_current_cycle[channel] = 1;

            break;

        case PWR_UP_CMD:

            assert(dram_state[channel][rank][bank].state == PRECHARGE_POWER_DOWN_SLOW || dram_state[channel][rank][bank].state == PRECHARGE_POWER_DOWN_FAST || dram_state[channel][rank][bank].state == ACTIVE_POWER_DOWN);

            //UT_MEM_DEBUG("\nCycle: %lld Cmd: PWR_UP_CMD Chan:%d Rank:%d \n", CYCLE_VAL, channel, rank);

            for (int i = 0; i < NUM_BANKS; i++)
            {

                if (dram_state[channel][rank][i].state == PRECHARGE_POWER_DOWN_SLOW || dram_state[channel][rank][i].state == PRECHARGE_POWER_DOWN_FAST)
                {
                    dram_state[channel][rank][i].state = IDLE;
                    dram_state[channel][rank][i].active_row = -1;
                }
                else
                {
                    dram_state[channel][rank][i].state = ROW_ACTIVE;
                }

                if (dram_state[channel][rank][i].state == PRECHARGE_POWER_DOWN_SLOW)
                {
                    dram_state[channel][rank][i].next_powerdown = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_powerdown);

                    dram_state[channel][rank][i].next_pre = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_pre);

                    dram_state[channel][rank][i].next_read = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_read);

                    dram_state[channel][rank][i].next_write = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_write);

                    dram_state[channel][rank][i].next_act = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_act);

                    dram_state[channel][rank][i].next_refresh = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_refresh);
                }
                else
                {

                    dram_state[channel][rank][i].next_powerdown = max(cycle + T_XP, dram_state[channel][rank][i].next_powerdown);

                    dram_state[channel][rank][i].next_pre = max(cycle + T_XP, dram_state[channel][rank][i].next_pre);

                    dram_state[channel][rank][i].next_read = max(cycle + T_XP, dram_state[channel][rank][i].next_read);

                    dram_state[channel][rank][i].next_write = max(cycle + T_XP, dram_state[channel][rank][i].next_write);

                    dram_state[channel][rank][i].next_act = max(cycle + T_XP, dram_state[channel][rank][i].next_act);

                    dram_state[channel][rank][i].next_refresh = max(cycle + T_XP, dram_state[channel][rank][i].next_refresh);
                }
            }

            stats_num_powerup[channel][rank]++;
            command_issued_current_cycle[channel] = 1;

            break;
        case NOP:

            //UT_MEM_DEBUG("\nCycle: %lld Cmd: NOP Chan:%d\n", CYCLE_VAL, channel);
            break;

        default:
            break;
    }
    //fclose(fptr);
    return 1;
}

// Function called to see if the rank can be transitioned into a fast low
// power state - ACT_PDN or PRE_PDN_FAST.
int is_powerdown_fast_allowed(int channel, int rank)
{
    int flag = 0;

    // if already a command has been issued this cycle, or if
    // forced refreshes are underway, or if issuing this command
    // will cause us to miss the refresh deadline, do not allow it
    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank] || (CYCLE_VAL + T_PD_MIN + T_XP > refresh_issue_deadline[channel][rank]))
        return 0;

    // command can be allowed if the next_powerdown is met for all banks in the rank
    for (int i = 0; i < NUM_BANKS; i++)
    {
        if ((dram_state[channel][rank][i].state == PRECHARGING || dram_state[channel][rank][i].state == ROW_ACTIVE || dram_state[channel][rank][i].state == IDLE || dram_state[channel][rank][i].state == REFRESHING) && CYCLE_VAL >= dram_state[channel][rank][i].next_powerdown)
            flag = 1;
        else
            return 0;
    }

    return flag;
}

// Function to see if the rank can be transitioned into a slow low
// power state - i.e. PRE_PDN_SLOW
int is_powerdown_slow_allowed(int channel, int rank)
{
    int flag = 0;

    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank] || (CYCLE_VAL + T_PD_MIN + T_XP_DLL > refresh_issue_deadline[channel][rank]))
        return 0;

    // Sleep command can be allowed if the next_powerdown is met for all banks in the rank
    // and if all the banks are precharged
    for (int i = 0; i < NUM_BANKS; i++)
    {
        if (dram_state[channel][rank][i].state == ROW_ACTIVE)
            return 0;
        else
        {
            if ((dram_state[channel][rank][i].state == PRECHARGING || dram_state[channel][rank][i].state == IDLE || dram_state[channel][rank][i].state == REFRESHING) && CYCLE_VAL >= dram_state[channel][rank][i].next_powerdown)
                flag = 1;
            else
                return 0;
        }
    }
    return flag;
}

// Function to see if the rank can be powered up
int is_powerup_allowed(int channel, int rank)
{
    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank])
        return 0;

    if (((dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_SLOW) || (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_FAST) || (dram_state[channel][rank][0].state == ACTIVE_POWER_DOWN)) && (CYCLE_VAL >= dram_state[channel][rank][0].next_powerup))
    {
        // check if issuing it will cause us to miss the refresh
        // deadline. If it does, don't allow it. The forced
        // refreshes will issue an implicit power up anyway
        if ((dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_SLOW) && ((CYCLE_VAL + T_XP_DLL) > refresh_issue_deadline[channel][0]))
            return 0;
        if (((dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_FAST) || (dram_state[channel][rank][0].state == ACTIVE_POWER_DOWN)) && ((CYCLE_VAL + T_XP) > refresh_issue_deadline[channel][0]))
            return 0;
        return 1;
    }
    else
        return 0;
}

// Function to see if the bank can be activated or not
int is_activate_allowed(int channel, int rank, int bank)
{
    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank] || (CYCLE_VAL + T_RAS > refresh_issue_deadline[channel][rank]))
        return 0;
    if ((dram_state[channel][rank][bank].state == IDLE || dram_state[channel][rank][bank].state == PRECHARGING || dram_state[channel][rank][bank].state == REFRESHING) && (CYCLE_VAL >= dram_state[channel][rank][bank].next_act) && (is_T_FAW_met(channel, rank, CYCLE_VAL)))
        return 1;
    else
        return 0;
}

// Function to see if the rank can be precharged or not
int is_autoprecharge_allowed(int channel, int rank, int bank)
{
    long long int start_precharge = 0;
    if (cas_issued_current_cycle[channel][rank][bank] == 1)
        start_precharge = max(CYCLE_VAL + T_RTP, dram_state[channel][rank][bank].next_pre);
    else
        start_precharge = max(CYCLE_VAL + T_CWD + T_DATA_TRANS + T_WR, dram_state[channel][rank][bank].next_pre);

    if (((cas_issued_current_cycle[channel][rank][bank] == 1) && ((start_precharge + T_RP) <= refresh_issue_deadline[channel][rank])) || ((cas_issued_current_cycle[channel][rank][bank] == 2) && ((start_precharge + T_RP) <= refresh_issue_deadline[channel][rank])))
        return 1;
    else
        return 0;
}

// Function to see if the rank can be precharged or not
int is_precharge_allowed(int channel, int rank, int bank)
{
    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank] || (CYCLE_VAL + T_RP > refresh_issue_deadline[channel][rank]))
        return 0;

    if ((dram_state[channel][rank][bank].state == ROW_ACTIVE || dram_state[channel][rank][bank].state == IDLE || dram_state[channel][rank][bank].state == PRECHARGING || dram_state[channel][rank][bank].state == REFRESHING) && (CYCLE_VAL >= dram_state[channel][rank][bank].next_pre))
        return 1;
    else
        return 0;
}

// function to see if all banks can be precharged this cycle
int is_all_bank_precharge_allowed(int channel, int rank)
{
    int flag = 0;
    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank] || (CYCLE_VAL + T_RP > refresh_issue_deadline[channel][rank]))
        return 0;

    for (int i = 0; i < NUM_BANKS; i++)
    {
        if ((dram_state[channel][rank][i].state == ROW_ACTIVE || dram_state[channel][rank][i].state == IDLE || dram_state[channel][rank][i].state == PRECHARGING || dram_state[channel][rank][i].state == REFRESHING) && (CYCLE_VAL >= dram_state[channel][rank][i].next_pre))
            flag = 1;
        else
            return 0;
    }
    return flag;
}

// function to see if refresh can be allowed this cycle

int is_refresh_allowed(int channel, int rank)
{
    if (command_issued_current_cycle[channel] || forced_refresh_mode_on[channel][rank])
        return 0;

    for (int b = 0; b < NUM_BANKS; b++)
    {
        if (CYCLE_VAL < dram_state[channel][rank][b].next_refresh)
            return 0;
    }
    return 1;
}

// Function to put a rank into the low power mode
int issue_powerdown_command(int channel, int rank, command_t cmd)
{
    if (command_issued_current_cycle[channel])
    {
        printf("PANIC : SCHED_ERROR: Got beat. POWER_DOWN command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }

    // if right CMD has been used
    if ((cmd != PWR_DN_FAST_CMD) && (cmd != PWR_DN_SLOW_CMD))
    {
        printf("PANIC: SCHED_ERROR : Only PWR_DN_SLOW_CMD or PWR_DN_FAST_CMD can be used to put DRAM rank to sleep\n");
        return 0;
    }
    // if the powerdown command can indeed be issued
    if (((cmd == PWR_DN_FAST_CMD) && !is_powerdown_fast_allowed(channel, rank)) || ((cmd == PWR_DN_SLOW_CMD) && !is_powerdown_slow_allowed(channel, rank)))
    {
        printf("PANIC : SCHED_ERROR: POWER_DOWN command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }

    for (int i = 0; i < NUM_BANKS; i++)
    {
        // next_powerup and refresh times
        dram_state[channel][rank][i].next_powerup = max(CYCLE_VAL + T_PD_MIN, dram_state[channel][rank][i].next_powerdown);
        dram_state[channel][rank][i].next_refresh = max(CYCLE_VAL + T_PD_MIN, dram_state[channel][rank][i].next_refresh);

        // state change
        if (dram_state[channel][rank][i].state == IDLE || dram_state[channel][rank][i].state == PRECHARGING || dram_state[channel][rank][i].state == REFRESHING)
        {
            if (cmd == PWR_DN_SLOW_CMD)
            {
                dram_state[channel][rank][i].state = PRECHARGE_POWER_DOWN_SLOW;
                stats_num_powerdown_slow[channel][rank]++;
            }
            else if (cmd == PWR_DN_FAST_CMD)
            {
                dram_state[channel][rank][i].state = PRECHARGE_POWER_DOWN_FAST;
                stats_num_powerdown_fast[channel][rank]++;
            }

            dram_state[channel][rank][i].active_row = -1;
        }
        else if (dram_state[channel][rank][i].state == ROW_ACTIVE)
        {
            dram_state[channel][rank][i].state = ACTIVE_POWER_DOWN;
        }
    }
    command_issued_current_cycle[channel] = 1;
    return 1;
}

// Function to power a rank up
int issue_powerup_command(int channel, int rank)
{
    if (!is_powerup_allowed(channel, rank))
    {
        printf("PANIC : SCHED_ERROR: POWER_UP command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        long long int cycle = CYCLE_VAL;
        for (int i = 0; i < NUM_BANKS; i++)
        {

            if (dram_state[channel][rank][i].state == PRECHARGE_POWER_DOWN_SLOW || dram_state[channel][rank][i].state == PRECHARGE_POWER_DOWN_FAST)
            {
                dram_state[channel][rank][i].state = IDLE;
                dram_state[channel][rank][i].active_row = -1;
            }
            else
            {
                dram_state[channel][rank][i].state = ROW_ACTIVE;
            }

            if (dram_state[channel][rank][i].state == PRECHARGE_POWER_DOWN_SLOW)
            {
                dram_state[channel][rank][i].next_powerdown = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_powerdown);

                dram_state[channel][rank][i].next_pre = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_pre);

                dram_state[channel][rank][i].next_read = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_read);

                dram_state[channel][rank][i].next_write = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_write);

                dram_state[channel][rank][i].next_act = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_act);

                dram_state[channel][rank][i].next_refresh = max(cycle + T_XP_DLL, dram_state[channel][rank][i].next_refresh);
            }
            else
            {

                dram_state[channel][rank][i].next_powerdown = max(cycle + T_XP, dram_state[channel][rank][i].next_powerdown);

                dram_state[channel][rank][i].next_pre = max(cycle + T_XP, dram_state[channel][rank][i].next_pre);

                dram_state[channel][rank][i].next_read = max(cycle + T_XP, dram_state[channel][rank][i].next_read);

                dram_state[channel][rank][i].next_write = max(cycle + T_XP, dram_state[channel][rank][i].next_write);

                dram_state[channel][rank][i].next_act = max(cycle + T_XP, dram_state[channel][rank][i].next_act);

                dram_state[channel][rank][i].next_refresh = max(cycle + T_XP, dram_state[channel][rank][i].next_refresh);
            }
        }

        command_issued_current_cycle[channel] = 1;
        return 1;
    }
}

// Function to issue a precharge command to a specific bank
int issue_autoprecharge(int channel, int rank, int bank)
{
    if (!is_autoprecharge_allowed(channel, rank, bank))
        return 0;
    else
    {
        long long int start_precharge = 0;

        dram_state[channel][rank][bank].active_row = -1;

        dram_state[channel][rank][bank].state = PRECHARGING;

        if (cas_issued_current_cycle[channel][rank][bank] == 1)
            start_precharge = max(CYCLE_VAL + T_RTP, dram_state[channel][rank][bank].next_pre);
        else
            start_precharge = max(CYCLE_VAL + T_CWD + T_DATA_TRANS + T_WR, dram_state[channel][rank][bank].next_pre);

        dram_state[channel][rank][bank].next_act = max(start_precharge + T_RP, dram_state[channel][rank][bank].next_act);

        dram_state[channel][rank][bank].next_powerdown = max(start_precharge + T_RP, dram_state[channel][rank][bank].next_powerdown);

        dram_state[channel][rank][bank].next_pre = max(start_precharge + T_RP, dram_state[channel][rank][bank].next_pre);

        dram_state[channel][rank][bank].next_refresh = max(start_precharge + T_RP, dram_state[channel][rank][bank].next_refresh);

        stats_num_precharge[channel][rank][bank]++;

        // reset the cas_issued_current_cycle
        for (int r = 0; r < NUM_RANKS; r++)
            for (int b = 0; b < NUM_BANKS; b++)
                cas_issued_current_cycle[channel][r][b] = 0;

        return 1;
    }
}

// Function to issue an activate command to a specific row
int issue_activate_command(int channel, int rank, int bank, long long int row)
{
    if (!is_activate_allowed(channel, rank, bank))
    {
        printf("PANIC : SCHED_ERROR: ACTIVATE command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        long long int cycle = CYCLE_VAL;

        dram_state[channel][rank][bank].state = ROW_ACTIVE;

        dram_state[channel][rank][bank].active_row = row;

        dram_state[channel][rank][bank].next_pre = max((cycle + T_RAS), dram_state[channel][rank][bank].next_pre);

        dram_state[channel][rank][bank].next_refresh = max((cycle + T_RAS), dram_state[channel][rank][bank].next_refresh);

        dram_state[channel][rank][bank].next_read = max(cycle + T_RCD, dram_state[channel][rank][bank].next_read);

        dram_state[channel][rank][bank].next_write = max(cycle + T_RCD, dram_state[channel][rank][bank].next_write);

        dram_state[channel][rank][bank].next_act = max(cycle + T_RC, dram_state[channel][rank][bank].next_act);

        dram_state[channel][rank][bank].next_powerdown = max(cycle + T_RCD, dram_state[channel][rank][bank].next_powerdown);

        for (int i = 0; i < NUM_BANKS; i++)
            if (i != bank)
                dram_state[channel][rank][i].next_act = max(cycle + T_RRD, dram_state[channel][rank][i].next_act);

        record_activate(channel, rank, cycle);

        stats_num_activate[channel][rank]++;
        stats_num_activate_spec[channel][rank][bank]++;

        average_gap_between_activates[channel][rank] = ((average_gap_between_activates[channel][rank] * (stats_num_activate[channel][rank] - 1)) + (CYCLE_VAL - last_activate[channel][rank])) / stats_num_activate[channel][rank];

        last_activate[channel][rank] = CYCLE_VAL;

        command_issued_current_cycle[channel] = 1;

        return 1;
    }
}

// Function to issue a precharge command to a specific bank
int issue_precharge_command(int channel, int rank, int bank)
{
    if (!is_precharge_allowed(channel, rank, bank))
    {
        printf("PANIC : SCHED_ERROR: PRECHARGE command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        dram_state[channel][rank][bank].state = PRECHARGING;

        dram_state[channel][rank][bank].active_row = -1;

        dram_state[channel][rank][bank].next_act = max(CYCLE_VAL + T_RP, dram_state[channel][rank][bank].next_act);

        dram_state[channel][rank][bank].next_powerdown = max(CYCLE_VAL + T_RP, dram_state[channel][rank][bank].next_powerdown);

        dram_state[channel][rank][bank].next_pre = max(CYCLE_VAL + T_RP, dram_state[channel][rank][bank].next_pre);

        dram_state[channel][rank][bank].next_refresh = max(CYCLE_VAL + T_RP, dram_state[channel][rank][bank].next_refresh);

        stats_num_precharge[channel][rank][bank]++;

        command_issued_current_cycle[channel] = 1;

        return 1;
    }
}

// Function to precharge a rank
int issue_all_bank_precharge_command(int channel, int rank)
{
    if (!is_all_bank_precharge_allowed(channel, rank))
    {
        printf("PANIC : SCHED_ERROR: ALL_BANK_PRECHARGE command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        for (int i = 0; i < NUM_BANKS; i++)
        {
            issue_precharge_command(channel, rank, i);
            command_issued_current_cycle[channel] = 0; /* Since issue_precharge_command would have set this, we need to reset it. */
        }
        command_issued_current_cycle[channel] = 1;
        return 1;
    }
}

// Function to Issue a SWAP operation
int issue_swap(int channel, int rank, int delaymul)
{

    if (!is_refresh_allowed(channel, rank))
    {
        printf("PANIC : SCHED_ERROR: REFRESH command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        long long int cycle = CYCLE_VAL;

        if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_SLOW)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP_DLL + (T_SWAP*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP_DLL + (T_SWAP*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP_DLL + (T_SWAP*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP_DLL + (T_SWAP*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP_DLL + T_RCD + (T_SWAP*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP_DLL + T_RCD + (T_SWAP*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_FAST)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP + T_RCD + (T_SWAP*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP + T_RCD + (T_SWAP*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else if (dram_state[channel][rank][0].state == ACTIVE_POWER_DOWN)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else // rank powered up
        {
            int flag = 0;
            for (int b = 0; b < NUM_BANKS; b++)
            {
                if (dram_state[channel][rank][b].state == ROW_ACTIVE)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag) // at least a single bank is open
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_powerdown);
                    dram_state[channel][rank][b].next_read = max(cycle + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_read);
                    dram_state[channel][rank][b].next_write = max(cycle + T_RP + (T_SWAP*delaymul), dram_state[channel][rank][b].next_write);
                }
            }
            else // everything precharged
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + (T_SWAP*delaymul), dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + (T_SWAP*delaymul), dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + (T_SWAP*delaymul), dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + (T_SWAP*delaymul), dram_state[channel][rank][b].next_powerdown);
                    dram_state[channel][rank][b].next_read = max(cycle + (T_SWAP*delaymul), dram_state[channel][rank][b].next_read);
                    dram_state[channel][rank][b].next_write = max(cycle + (T_SWAP*delaymul), dram_state[channel][rank][b].next_write);
                }
            }
        }
        for (int b = 0; b < NUM_BANKS; b++)
        {
            //dram_state[channel][rank][b].active_row = -1;
            dram_state[channel][rank][b].state = REFRESHING;
        }
        command_issued_current_cycle[channel] = 1;
        return 1;
    }
}

// Function to Issue a MOVE operation
unsigned long long int issue_move(int channel, int rank, int delaymul)
{
    unsigned long long int returnval = 0;

    if (!is_refresh_allowed(channel, rank))
    {
        printf("PANIC : SCHED_ERROR: REFRESH command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        long long int cycle = CYCLE_VAL;

        if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_SLOW)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP_DLL + (T_MOVE*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP_DLL + (T_MOVE*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP_DLL + (T_MOVE*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP_DLL + (T_MOVE*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP_DLL + (T_MOVE*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP_DLL + (T_MOVE*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_FAST)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else if (dram_state[channel][rank][0].state == ACTIVE_POWER_DOWN)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else // rank powered up
        {
            int flag = 0;
            for (int b = 0; b < NUM_BANKS; b++)
            {
                if (dram_state[channel][rank][b].state == ROW_ACTIVE)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag) // at least a single bank is open
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_powerdown);
                    dram_state[channel][rank][b].next_read = max(cycle + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_read);
                    dram_state[channel][rank][b].next_write = max(cycle + T_RP + (T_MOVE*delaymul), dram_state[channel][rank][b].next_write);
                }
            }
            else // everything precharged
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + (T_MOVE*delaymul), dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + (T_MOVE*delaymul), dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + (T_MOVE*delaymul), dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + (T_MOVE*delaymul), dram_state[channel][rank][b].next_powerdown);
                    dram_state[channel][rank][b].next_read = max(cycle + (T_MOVE*delaymul), dram_state[channel][rank][b].next_read);
                    dram_state[channel][rank][b].next_write = max(cycle + (T_MOVE*delaymul), dram_state[channel][rank][b].next_write);
                }
            }
        }
        for (int b = 0; b < NUM_BANKS; b++)
        {
            //dram_state[channel][rank][b].active_row = -1;
            dram_state[channel][rank][b].state = REFRESHING;
            returnval = dram_state[channel][rank][b].next_act;
        }
        command_issued_current_cycle[channel] = 1;
        return returnval;
    }
}

// Function to Issue a STALL operation
int issue_stall(int channel, int rank, int delaymul)
{

    if (!is_refresh_allowed(channel, rank))
    {
        printf("PANIC : SCHED_ERROR: REFRESH command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        long long int cycle = CYCLE_VAL;

        if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_SLOW)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP_DLL + (T_STALL*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP_DLL + (T_STALL*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP_DLL + (T_STALL*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP_DLL + (T_STALL*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP_DLL + (T_STALL*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP_DLL + (T_STALL*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_FAST)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + (T_STALL*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + (T_STALL*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + (T_STALL*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + (T_STALL*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP + (T_STALL*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP + (T_STALL*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else if (dram_state[channel][rank][0].state == ACTIVE_POWER_DOWN)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_powerdown);
                dram_state[channel][rank][b].next_read = max(cycle + T_XP + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_read);
                dram_state[channel][rank][b].next_write = max(cycle + T_XP + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_write);
            }
        }
        else // rank powered up
        {
            int flag = 0;
            for (int b = 0; b < NUM_BANKS; b++)
            {
                if (dram_state[channel][rank][b].state == ROW_ACTIVE)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag) // at least a single bank is open
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_powerdown);
                    dram_state[channel][rank][b].next_read = max(cycle + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_read);
                    dram_state[channel][rank][b].next_write = max(cycle + T_RP + (T_STALL*delaymul), dram_state[channel][rank][b].next_write);
                }
            }
            else // everything precharged
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + (T_STALL*delaymul), dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + (T_STALL*delaymul), dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + (T_STALL*delaymul), dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + (T_STALL*delaymul), dram_state[channel][rank][b].next_powerdown);
                    dram_state[channel][rank][b].next_read = max(cycle + (T_STALL*delaymul), dram_state[channel][rank][b].next_read);
                    dram_state[channel][rank][b].next_write = max(cycle + (T_STALL*delaymul), dram_state[channel][rank][b].next_write);
                }
            }
        }
        for (int b = 0; b < NUM_BANKS; b++)
        {
            //dram_state[channel][rank][b].active_row = -1;
            dram_state[channel][rank][b].state = REFRESHING;
        }
        command_issued_current_cycle[channel] = 1;
        return 1;
    }
}



// Function to issue a refresh
int issue_refresh_command(int channel, int rank)
{

    if (!is_refresh_allowed(channel, rank))
    {
        printf("PANIC : SCHED_ERROR: REFRESH command not issuable in cycle:%lld\n", CYCLE_VAL);
        return 0;
    }
    else
    {
        num_issued_refreshes[channel][rank]++;
        long long int cycle = CYCLE_VAL;

        if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_SLOW)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP_DLL + T_RFC, dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP_DLL + T_RFC, dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP_DLL + T_RFC, dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP_DLL + T_RFC, dram_state[channel][rank][b].next_powerdown);
            }
        }
        else if (dram_state[channel][rank][0].state == PRECHARGE_POWER_DOWN_FAST)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + T_RFC, dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + T_RFC, dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + T_RFC, dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + T_RFC, dram_state[channel][rank][b].next_powerdown);
            }
        }
        else if (dram_state[channel][rank][0].state == ACTIVE_POWER_DOWN)
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                dram_state[channel][rank][b].next_act = max(cycle + T_XP + T_RP + T_RFC, dram_state[channel][rank][b].next_act);
                dram_state[channel][rank][b].next_pre = max(cycle + T_XP + T_RP + T_RFC, dram_state[channel][rank][b].next_pre);
                dram_state[channel][rank][b].next_refresh = max(cycle + T_XP + T_RP + T_RFC, dram_state[channel][rank][b].next_refresh);
                dram_state[channel][rank][b].next_powerdown = max(cycle + T_XP + T_RP + T_RFC, dram_state[channel][rank][b].next_powerdown);
            }
        }
        else // rank powered up
        {
            int flag = 0;
            for (int b = 0; b < NUM_BANKS; b++)
            {
                if (dram_state[channel][rank][b].state == ROW_ACTIVE)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag) // at least a single bank is open
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + T_RP + T_RFC, dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + T_RP + T_RFC, dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + T_RP + T_RFC, dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + T_RP + T_RFC, dram_state[channel][rank][b].next_powerdown);
                }
            }
            else // everything precharged
            {
                for (int b = 0; b < NUM_BANKS; b++)
                {
                    dram_state[channel][rank][b].next_act = max(cycle + T_RFC, dram_state[channel][rank][b].next_act);
                    dram_state[channel][rank][b].next_pre = max(cycle + T_RFC, dram_state[channel][rank][b].next_pre);
                    dram_state[channel][rank][b].next_refresh = max(cycle + T_RFC, dram_state[channel][rank][b].next_refresh);
                    dram_state[channel][rank][b].next_powerdown = max(cycle + T_RFC, dram_state[channel][rank][b].next_powerdown);
                }
            }
        }
        for (int b = 0; b < NUM_BANKS; b++)
        {
            dram_state[channel][rank][b].active_row = -1;
            dram_state[channel][rank][b].state = REFRESHING;
        }
        command_issued_current_cycle[channel] = 1;
        return 1;
    }
}

void issue_forced_refresh_commands(int channel, int rank)
{
    for (int b = 0; b < NUM_BANKS; b++)
    {

        dram_state[channel][rank][b].state = REFRESHING;
        dram_state[channel][rank][b].active_row = -1;

        dram_state[channel][rank][b].next_act = next_refresh_completion_deadline[channel][rank];
        dram_state[channel][rank][b].next_pre = next_refresh_completion_deadline[channel][rank];
        dram_state[channel][rank][b].next_refresh = next_refresh_completion_deadline[channel][rank];
        dram_state[channel][rank][b].next_powerdown = next_refresh_completion_deadline[channel][rank];
    }
}

void gather_stats(int channel)
{
    for (int i = 0; i < NUM_RANKS; i++)
    {

        if (dram_state[channel][i][0].state == PRECHARGE_POWER_DOWN_SLOW)
            stats_time_spent_in_precharge_power_down_slow[channel][i] += PROCESSOR_CLK_MULTIPLIER;
        else if (dram_state[channel][i][0].state == PRECHARGE_POWER_DOWN_FAST)
            stats_time_spent_in_precharge_power_down_fast[channel][i] += PROCESSOR_CLK_MULTIPLIER;
        else if (dram_state[channel][i][0].state == ACTIVE_POWER_DOWN)
            stats_time_spent_in_active_power_down[channel][i] += PROCESSOR_CLK_MULTIPLIER;
        else
        {
            for (int b = 0; b < NUM_BANKS; b++)
            {
                if (dram_state[channel][i][b].state == ROW_ACTIVE)
                {
                    stats_time_spent_in_active_standby[channel][i] += PROCESSOR_CLK_MULTIPLIER;
                    break;
                }
            }
            stats_time_spent_in_power_up[channel][i] += PROCESSOR_CLK_MULTIPLIER;
        }
    }
}

long long int generate_random_addr(int channel)
{
    //Bitlengths of DRAM component addresses
    int channelBitWidth = log_base2(NUM_CHANNELS);
    int rankBitWidth = log_base2(NUM_RANKS);
    int bankBitWidth = log_base2(NUM_BANKS);
    int rowBitWidth = log_base2(NUM_ROWS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);


    long long int unit = 1;
    long long int random = rand();
    long long int random1 = rand();
    long long int filter = 0;
    long long int tempaddr = 0;

    if(ADDRESS_MAPPING == 1){
        random = rand();
        random = random << 15;
        random = random | random1;

        filter = (unit << rowBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << rankBitWidth;
        random = rand();
        filter = (unit << rankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << bankBitWidth;
        random = rand();
        filter = (unit << bankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << channelBitWidth;
        tempaddr = tempaddr | channel;

        tempaddr = tempaddr << colBitWidth;
        random = rand();
        filter = (unit << colBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        //Finally shift it by ByteOffset
        tempaddr = tempaddr << byteOffsetWidth;
        return(tempaddr);
    }
    else{
        random = rand();
        random = random << 15;
        random = random | random1;

        filter = (unit << rowBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << colBitWidth;
        random = rand();
        filter = (unit << colBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << rankBitWidth;
        random = rand();
        filter = (unit << rankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << bankBitWidth;
        random = rand();
        filter = (unit << bankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << channelBitWidth;
        tempaddr = tempaddr | channel;

        //Finally shift it by ByteOffset
        tempaddr = tempaddr << byteOffsetWidth;
        return(tempaddr);

    }
}

long long int generate_random_addr_bs(int channel, int bank)
{
    //Bitlengths of DRAM component addresses
    int channelBitWidth = log_base2(NUM_CHANNELS);
    int rankBitWidth = log_base2(NUM_RANKS);
    int bankBitWidth = log_base2(NUM_BANKS);
    int rowBitWidth = log_base2(NUM_ROWS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);


    long long int unit = 1;
    long long int random = rand();
    long long int random1 = rand();
    long long int filter = 0;
    long long int tempaddr = 0;

    if(ADDRESS_MAPPING == 1){
        random = rand();
        random = random << 15;
        random = random | random1;
        
        filter = (unit << rowBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;
        
        tempaddr = tempaddr << rankBitWidth;
        random = rand();
        filter = (unit << rankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;
        
        tempaddr = tempaddr << bankBitWidth;
        filter = (unit << bankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | bank;
        
        tempaddr = tempaddr << channelBitWidth;
        tempaddr = tempaddr | channel;
        
        tempaddr = tempaddr << colBitWidth;
        random = rand();
        filter = (unit << colBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;
        
        //Finally shift it by ByteOffset
        tempaddr = tempaddr << byteOffsetWidth;
        return(tempaddr);
    }
    else{
        random = rand();
        random = random << 15;
        random = random | random1;

        filter = (unit << rowBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << colBitWidth;
        random = rand();
        filter = (unit << colBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << rankBitWidth;
        random = rand();
        filter = (unit << rankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << bankBitWidth;
        filter = (unit << bankBitWidth) - 1;
        random = random & bank;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << channelBitWidth;
        tempaddr = tempaddr | channel;

        //Finally shift it by ByteOffset
        tempaddr = tempaddr << byteOffsetWidth;
        return(tempaddr);

    }
}

long long int generate_bank_specific_addr(int channel, int bank)
{
    //Bitlengths of DRAM component addresses
    int channelBitWidth = log_base2(NUM_CHANNELS);
    int rankBitWidth = log_base2(NUM_RANKS);
    int bankBitWidth = log_base2(NUM_BANKS);
    int rowBitWidth = log_base2(NUM_ROWS);
    int colBitWidth = log_base2(NUM_COLUMNS);
    int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);


    long long int unit = 1;
    long long int random = rand();
    long long int random1 = rand();
    long long int filter = 0;
    long long int tempaddr = 0;

    if(ADDRESS_MAPPING == 1){
        random = rand();
        random = random << 15;
        random = random | random1;

        filter = (unit << rowBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << rankBitWidth;
        random = rand();
        filter = (unit << rankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << bankBitWidth;
        tempaddr = tempaddr | bank;

        tempaddr = tempaddr << channelBitWidth;
        tempaddr = tempaddr | channel;

        tempaddr = tempaddr << colBitWidth;
        random = rand();
        filter = (unit << colBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        //Finally shift it by ByteOffset
        tempaddr = tempaddr << byteOffsetWidth;
        return(tempaddr);
    }
    else{
        random = rand();
        random = random << 15;
        random = random | random1;

        filter = (unit << rowBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << colBitWidth;
        random = rand();
        filter = (unit << colBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << rankBitWidth;
        random = rand();
        filter = (unit << rankBitWidth) - 1;
        random = random & filter;
        tempaddr = tempaddr | random;

        tempaddr = tempaddr << bankBitWidth;
        tempaddr = tempaddr | bank;

        tempaddr = tempaddr << channelBitWidth;
        tempaddr = tempaddr | channel;

        //Finally shift it by ByteOffset
        tempaddr = tempaddr << byteOffsetWidth;
        return(tempaddr);

    }
}


void print_stats()
{
    long long int activates_for_reads = 0;
    long long int activates_for_spec = 0;
    long long int activates_for_writes = 0;
    long long int read_cmds = 0;
    long long int write_cmds = 0;

    long long int stats_reads_completed_total = 0;
    long long int stats_writes_completed_total = 0;
    long long int stats_average_read_latency_total = 0;
    long long int stats_average_read_queue_latency_total = 0;
    long long int stats_average_write_latency_total = 0;
    long long int stats_average_write_queue_latency_total = 0;

    long long int total_act_counter = 0;
    long long int total_ref_counter = 0;
    for (int c = 0; c < NUM_CHANNELS; c++)
    {
        activates_for_writes = 0;
        activates_for_reads = 0;
        activates_for_spec = 0;
        read_cmds = 0;
        write_cmds = 0;
        for (int r = 0; r < NUM_RANKS; r++)
        {
            total_act_counter = act_counter[c][r]+total_act_counter;
            total_ref_counter = ref_counter[c][r]+total_ref_counter;
            printf("Act_Countc%dr%d :	%7.5f\n", c, r, ((double)act_counter[c][r]*1026)/ref_counter[c][r]);
            for (int b = 0; b < NUM_BANKS; b++)
            {
                activates_for_writes += stats_num_activate_write[c][r][b];
                activates_for_reads += stats_num_activate_read[c][r][b];
                activates_for_spec += stats_num_activate_spec[c][r][b];
                read_cmds += stats_num_read[c][r][b];
                write_cmds += stats_num_write[c][r][b];
            }
        }

        printf("-------- Channel%d_Stats-----------\n", c);
        printf("Reads_Serviced_Channel%d :      %-7lld\n", c, stats_reads_completed[c]);
        stats_reads_completed_total += stats_reads_completed[c];
        printf("Writes_Serviced_Channel%d :     %-7lld\n", c, stats_writes_completed[c]);
        stats_writes_completed_total += stats_writes_completed[c];
        printf("Avg_Read_Latency_Channel%d :    %7.5f\n", c, (double)stats_average_read_latency[c]);
        stats_average_read_latency_total += stats_average_read_latency[c];
        printf("Avg_Read_Q_Latency_Channel%d :  %7.5f\n", c, (double)stats_average_read_queue_latency[c]);
        stats_average_read_queue_latency_total += stats_average_read_queue_latency[c];
        printf("Avg_Write_Q_Latency_Channel%d : %7.5f\n", c, (double)stats_average_write_latency[c]);
        stats_average_write_latency_total += stats_average_write_latency[c];
        printf("Avg_Write_Q_Latency_Channel%d : %7.5f\n", c, (double)stats_average_write_queue_latency[c]);
        stats_average_write_queue_latency_total += stats_average_write_queue_latency[c];
        if((read_cmds - activates_for_reads - activates_for_spec) > 0)
            printf("Read_Page_Hit_Rate_Channel%d :  %7.5f\n", c, ((double)(read_cmds - activates_for_reads - activates_for_spec) / read_cmds));
        else{
            printf("Read_Page_Hit_Rate_Channel%d :  0\n",c);
        }
        printf("Write_Page_Hit_Rate_Channel%d : %7.5f\n", c, ((double)(write_cmds - activates_for_writes) / write_cmds));
        printf("------------------------------------\n");
    }
    printf("-------- TOTAL_Stats-----------\n");
    printf("Total_Act_Count :	%7.5f\n", ((double)total_act_counter*1026)/total_ref_counter);
    printf("Reads_Serviced_total :      %-7lld\n", stats_reads_completed_total);
    printf("Writes_Serviced_total :     %-7lld\n", stats_writes_completed_total);
    printf("Total_Read_Latency :	%7.5f\n", (double)stats_average_read_latency_total/NUM_CHANNELS);
    printf("Total_Read_Q_Latency :      %7.5f\n", (double)stats_average_read_queue_latency_total/NUM_CHANNELS);
    printf("Total_Write_Q_Latency :     %7.5f\n", (double)stats_average_write_latency_total/NUM_CHANNELS);
    printf("Total_Write_Q_Latency :     %7.5f\n", (double)stats_average_write_queue_latency_total/NUM_CHANNELS);
    if((read_cmds - activates_for_reads - activates_for_spec) > 0){
        printf("Read_Page_Hit_Rate_Total :  %7.5f\n", ((double)(read_cmds - activates_for_reads - activates_for_spec) / read_cmds));
    }
    else{
        printf("Read_Page_Hit_Rate_Total :  0\n");
    }
    printf("Write_Page_Hit_Rate_Total : %7.5f\n", ((double)(write_cmds - activates_for_writes) / write_cmds));
    printf("------------------------------------\n");
}

void update_issuable_commands(int channel)
{
    for (int rank = 0; rank < NUM_RANKS; rank++)
    {
        for (int bank = 0; bank < NUM_BANKS; bank++)
            cmd_precharge_issuable[channel][rank][bank] = is_precharge_allowed(channel, rank, bank);

        cmd_all_bank_precharge_issuable[channel][rank] = is_all_bank_precharge_allowed(channel, rank);

        cmd_powerdown_fast_issuable[channel][rank] = is_powerdown_fast_allowed(channel, rank);

        cmd_powerdown_slow_issuable[channel][rank] = is_powerdown_slow_allowed(channel, rank);

        cmd_refresh_issuable[channel][rank] = is_refresh_allowed(channel, rank);

        cmd_powerup_issuable[channel][rank] = is_powerup_allowed(channel, rank);
    }
}

// function that updates the dram state and schedules auto-refresh if
// necessary. This is called every DRAM cycle
void update_memory(int core)
{

    long long int t1 = 0;
    long long int t2 = 0;
    long long int t3 = 0;
    long long int t4 = 0;
    long long int t5 = 0;
    for (int c = 0; c < NUM_CHANNELS; c++)
    {   
        for (int r = 0; r < NUM_RANKS; r++)
        {   
            for (int b = 0; b < NUM_BANKS; b++)
            {   
                t1 += stats_num_activate_write[c][r][b];
                t2 += stats_num_activate_read[c][r][b];
                t3 += stats_num_activate_spec[c][r][b];
                t4 += stats_num_read[c][r][b];
                t5 += stats_num_write[c][r][b];
            }
        }
    }
    activates_for_writes_t = t1;
    activates_for_reads_t = t2;
    activates_for_spec_t = t3;
    read_cmds_t = t4;
    write_cmds_t = t5;


    for (int channel = 0; channel < NUM_CHANNELS; channel++)
    {
        // make every channel ready to receive a new command
        command_issued_current_cycle[channel] = 0;
        for (int rank = 0; rank < NUM_RANKS; rank++)
        {
            //reset variable
            for (int bank = 0; bank < NUM_BANKS; bank++){
                cas_issued_current_cycle[channel][rank][bank] = 0;
                
                if((ref_counter[channel][rank]%1024 == 0) && (resetflag[channel][rank][bank] == 1))
                {
                    if(MITIGATION_MODE == 0){
                        for(unsigned long long int u = 0; u < NUM_ROWS; u++){
                            if(unique[channel][rank][bank][u] == 1){
                                uniquecount++;
                            }
                            unique[channel][rank][bank][u] = 0;
                        }
                    }
                    for(unsigned long long int entry = 0; entry < mg_entries; entry++){
                        row_address_track[channel][rank][bank][entry] = 0;
                        row_address_count[channel][rank][bank][entry] = 0;
                    }
                    spillcounter_tracker[channel][rank][bank] = spillcounter_tracker[channel][rank][bank] + spillcounter[channel][rank][bank];
                    spillcounter[channel][rank][bank] = 0;
                    resetflag[channel][rank][bank] = 0;
                    printf("*");
                }
            }

            // clean out the activate record for
            // CYCLE_VAL - T_FAW
            flush_activate_record(channel, rank, CYCLE_VAL);

            // if we are at the refresh completion
            // deadline
            if (CYCLE_VAL == next_refresh_completion_deadline[channel][rank])
            {
                // calculate the next
                // refresh_issue_deadline
                num_issued_refreshes[channel][rank] = 0;
                last_refresh_completion_deadline[channel][rank] = CYCLE_VAL;
                next_refresh_completion_deadline[channel][rank] = CYCLE_VAL + 8 * T_REFI;
                refresh_issue_deadline[channel][rank] = next_refresh_completion_deadline[channel][rank] - T_RP - 8 * T_RFC;
                forced_refresh_mode_on[channel][rank] = 0;
                issued_forced_refresh_commands[channel][rank] = 0;
            }
            else if ((CYCLE_VAL == refresh_issue_deadline[channel][rank]) && (num_issued_refreshes[channel][rank] < 8))
            {
                // refresh_issue_deadline has been
                // reached. Do the auto-refreshes
                forced_refresh_mode_on[channel][rank] = 1;
                issue_forced_refresh_commands(channel, rank);
                ref_counter[channel][rank]++;

                if(ref_counter[channel][rank]%1024 == 0){
                    for (int bank = 0; bank < NUM_BANKS; bank++){
                        resetflag[channel][rank][bank] = 1;
                    }
                }

            }
            else if (CYCLE_VAL < refresh_issue_deadline[channel][rank])
            {
                //update the refresh_issue deadline
                refresh_issue_deadline[channel][rank] = next_refresh_completion_deadline[channel][rank] - T_RP - (8 - num_issued_refreshes[channel][rank]) * T_RFC;
            }
        }

        // update the variables corresponding to the non-queue
        // variables
        update_issuable_commands(channel);

        // update the request cmds in the queues
        update_read_queue_commands(channel);

        update_write_queue_commands(channel);

        for (int simcore = 0; simcore < core; simcore++)
        {
            // remove finished requests from temp queues
            clean_temp_queues(channel, simcore);
        }
        // remove finished requests
        clean_queues(channel);

    }

    //       if((read_queue_length[0]>0) && read_cmds_t){
    //           if((read_cmds_t - activates_for_reads_t - activates_for_spec_t) > 0)
    //            fprintf(fp,"%lld \t%lld \t%7.5f\n", CYCLE_VAL, read_queue_length[0],100*((double)(read_cmds_t - activates_for_reads_t - activates_for_spec_t) / read_cmds_t));
    //        }
    //    if(CYCLE_VAL > 2000000){
    //        fclose(fp);
    //        exit(0);
    //    }
}

//------------------------------------------------------------
// Calculate Power: It calculates and returns average power used by every Rank on Every
// Channel during the course of the simulation
// Units : Time- ns; Current mA; Voltage V; Power mW;
//------------------------------------------------------------

float calculate_power(int print_stats_type, int chips_per_rank)
{
    /*
       Power is calculated using the equations from Technical Note "TN-41-01: Calculating Memory System Power for DDR"
       The current values IDD* are taken from the data sheets.
       These are average current values that the chip will draw when certain actions occur as frequently as possible.
       i.e., the worst case power consumption
Eg: when ACTs happen every tRC
pds_<component> is the power calculated by directly using the current values from the data sheet. 'pds' stands for
PowerDataSheet. This will the power drawn by the chip when operating under the activity that is assumed in the data
sheet. This mostly represents the worst case power
These pds_<*> components need to be derated in accordance with the activity that is observed. Eg: If ACTs occur slower
than every tRC, then pds_act will be derated to give "psch_act" (SCHeduled Power consumed by Activate)
     */

    /*------------------------------------------------------------
    // total_power is the sum of of 13 components listed below
    // Note: CKE is the ClocK Enable to every chip.
    // Note: Even though the reads and write are to a different rank on the same channel, the Pull-Up and the Pull-Down resistors continue
    // 		to burn some power. psch_termWoth and psch_termWoth stand for the power dissipated in the rank in question when the reads and
    // 		writes are to other ranks on the channel

    psch_act 						-> Power dissipated by activating a row
    psch_act_pdn 				-> Power dissipated when CKE is low (disabled) and all banks are precharged
    psch_act_stby 			-> Power dissipated when CKE is high (enabled) and at least one back is active (row is open)
    psch_pre_pdn_fast  	-> Power dissipated when CKE is low (disabled) and all banks are precharged and chip is in fast power down
    psch_pre_pdn_slow  	-> Power dissipated when CKE is low (disabled) and all banks are precharged and chip is in fast slow  down
    psch_pre_stby 			-> Power dissipated when CKE is high (enabled) and at least one back is active (row is open)
    psch_termWoth 			-> Power dissipated when a Write termiantes at the other set of chips.
    psch_termRoth 			-> Power dissipated when a Read  termiantes at the other set of chips
    psch_termW 					-> Power dissipated when a Write termiantes at the set of chips in question
    psch_dq 						-> Power dissipated when a Read  termiantes at the set of chips in question (Data Pins on the chip are called DQ)
    psch_ref 						-> Power dissipated during Refresh
    psch_rd 						-> Power dissipated during a Read  (does ot include power to open a row)
    psch_wr 						-> Power dissipated during a Write (does ot include power to open a row)

    ------------------------------------------------------------*/

    float pds_act;
    float pds_act_pdn;
    float pds_act_stby;
    float pds_pre_pdn_fast;
    float pds_pre_pdn_slow;
    float pds_pre_stby;
    float pds_wr;
    float pds_rd;
    float pds_ref;
    float pds_dq;
    float pds_termW;
    float pds_termRoth;
    float pds_termWoth;

    float psch_act;
    float psch_pre_pdn_slow;
    float psch_pre_pdn_fast;
    float psch_act_pdn;
    float psch_act_stby;
    float psch_pre_stby;
    float psch_rd;
    float psch_wr;
    float psch_ref;
    float psch_dq;
    float psch_termW;
    float psch_termRoth;
    float psch_termWoth;

    float total_chip_power;


    /*----------------------------------------------------
    //Calculating DataSheet Power
    ----------------------------------------------------*/

    pds_act = (IDD0 - (IDD3N * T_RAS + IDD2N * (T_RC - T_RAS)) / T_RC) * VDD;

    pds_pre_pdn_slow = IDD2P0 * VDD;

    pds_pre_pdn_fast = IDD2P1 * VDD;

    pds_act_pdn = IDD3P * VDD;

    pds_pre_stby = IDD2N * VDD;
    pds_act_stby = IDD3N * VDD;

    pds_wr = (IDD4W - IDD3N) * VDD;

    pds_rd = (IDD4R - IDD3N) * VDD;

    pds_ref = (IDD5 - IDD3N) * VDD;

    /*----------------------------------------------------
    //On Die Termination (ODT) Power:
    //Values obtained from Micron Technical Note
    //This is dependent on the termination configuration of the simulated configuration
    //our simulator uses the same config as that used in the Tech Note
    ----------------------------------------------------*/
    pds_dq = 3.2 * 10;

    pds_termW = 0;

    pds_termRoth = 24.9 * 10;

    pds_termWoth = 20.8 * 11;

    /*----------------------------------------------------
    //Derating worst case power to represent system activity
    ----------------------------------------------------*/
    long long int writes_total = 0, reads_total = 0;
    long long int stats_time_spent_terminating_reads_from_other_ranks_total = 0;
    long long int stats_time_spent_terminating_writes_to_other_ranks_total = 0;
    long long int stats_time_spent_in_precharge_power_down_fast_total = 0;
    long long int stats_time_spent_in_precharge_power_down_slow_total = 0;
    long long int stats_time_spent_in_active_power_down_total = 0;
    long long int stats_time_spent_in_active_standby_total = 0;
    double time_in_pre_stby_total = 0;

    //-------- POWER -----------//
    float background_total = 0;
    float psch_act_total = 0;
    float psch_rd_total = 0;
    float psch_wr_total = 0;
    float psch_dq_total = 0;
    float psch_termW_total = 0;
    float psch_termRoth_total = 0;
    float psch_termWoth_total = 0;
    float psch_ref_total = 0;
    float total_power = 0;

    for(int channel = 0; channel < NUM_CHANNELS; channel++){
        long long int writes = 0, reads = 0;
        long long int stats_time_spent_terminating_reads_from_other_ranks_ch = 0;
        long long int stats_time_spent_terminating_writes_to_other_ranks_ch = 0;
        long long int stats_time_spent_in_precharge_power_down_fast_ch = 0;
        long long int stats_time_spent_in_precharge_power_down_slow_ch = 0;
        long long int stats_time_spent_in_active_power_down_ch = 0;
        long long int stats_time_spent_in_active_standby_ch = 0;
        double time_in_pre_stby_ch = 0;

        //-------- POWER -----------//
        float background_ch = 0;
        float psch_act_ch = 0;
        float psch_rd_ch = 0;
        float psch_wr_ch = 0;
        float psch_dq_ch = 0;
        float psch_termW_ch = 0;
        float psch_termRoth_ch = 0;
        float psch_termWoth_ch = 0;
        float psch_ref_ch = 0;
        float total_rank_power_ch = 0;

        for(int rank = 0; rank < NUM_RANKS; rank++){
            //average_gap_between_activates was initialised to 0. So if it is still
            //0, then no ACTs have happened to this rank.
            //Hence activate-power is also 0
            float total_rank_power = 0;
            if (average_gap_between_activates[channel][rank] == 0)
            {
                psch_act = 0;
            }
            else
            {
                psch_act = pds_act * T_RC / (average_gap_between_activates[channel][rank]);
            }

            psch_act_pdn = pds_act_pdn * ((double)stats_time_spent_in_active_power_down[channel][rank] / CYCLE_VAL);
            psch_pre_pdn_slow = pds_pre_pdn_slow * ((double)stats_time_spent_in_precharge_power_down_slow[channel][rank] / CYCLE_VAL);
            psch_pre_pdn_fast = pds_pre_pdn_fast * ((double)stats_time_spent_in_precharge_power_down_fast[channel][rank] / CYCLE_VAL);

            psch_act_stby = pds_act_stby * ((double)stats_time_spent_in_active_standby[channel][rank] / CYCLE_VAL);

            /*----------------------------------------------------
            //pds_pre_stby assumes that the system is powered up and every
            //row has been precharged during every cycle
            // In reality, the chip could have been in a power-down mode
            //or a row could have been active. The time spent in these modes
            //should be deducted from total time
            ----------------------------------------------------*/
            psch_pre_stby = pds_pre_stby * ((double)(CYCLE_VAL - stats_time_spent_in_active_standby[channel][rank] - stats_time_spent_in_precharge_power_down_slow[channel][rank] - stats_time_spent_in_precharge_power_down_fast[channel][rank] - stats_time_spent_in_active_power_down[channel][rank])) / CYCLE_VAL;

            /*----------------------------------------------------
            //Calculate Total Reads ans Writes performed in the system
            ----------------------------------------------------*/

            for (int i = 0; i < NUM_BANKS; i++)
            {
                writes += stats_num_write[channel][rank][i];
                reads += stats_num_read[channel][rank][i];
                reads_total += reads;
                writes_total += writes;
            }

            /*----------------------------------------------------
            // pds<rd/wr> assumes that there is rd/wr happening every cycle
            // T_DATA_TRANS is the number of cycles it takes for one rd/wr
            ----------------------------------------------------*/
            psch_wr = pds_wr * (writes * T_DATA_TRANS) / CYCLE_VAL;

            psch_rd = pds_rd * (reads * T_DATA_TRANS) / CYCLE_VAL;

            /*----------------------------------------------------
            //pds_ref assumes that there is always a refresh happening.
            //in reality, refresh consumes only T_RFC out of every t_REFI
            ----------------------------------------------------*/
            psch_ref = pds_ref * T_RFC / T_REFI;

            psch_dq = pds_dq * (reads * T_DATA_TRANS) / CYCLE_VAL;

            psch_termW = pds_termW * (writes * T_DATA_TRANS) / CYCLE_VAL;

            psch_termRoth = pds_termRoth * ((double)stats_time_spent_terminating_reads_from_other_ranks[channel][rank] / CYCLE_VAL);
            psch_termWoth = pds_termWoth * ((double)stats_time_spent_terminating_writes_to_other_ranks[channel][rank] / CYCLE_VAL);

            total_chip_power = psch_act + psch_termWoth + psch_termRoth + psch_termW + psch_dq + psch_ref + psch_rd + psch_wr + psch_pre_stby + psch_act_stby + psch_pre_pdn_fast + psch_pre_pdn_slow + psch_act_pdn;

            if(print_stats_type == 1)
                total_rank_power += total_chip_power * chips_per_rank;

            double time_in_pre_stby = (((double)(CYCLE_VAL - stats_time_spent_in_active_standby[channel][rank] - stats_time_spent_in_precharge_power_down_slow[channel][rank] - stats_time_spent_in_precharge_power_down_fast[channel][rank] - stats_time_spent_in_active_power_down[channel][rank])) / CYCLE_VAL);

            if (print_stats_type == 0)
            {
                printf("\n---------------------------------------------------------------\n");
                printf("Channel%d_Rank%d_ReadCycles :           %9.2f\n", channel, rank, (double)reads * T_DATA_TRANS / CYCLE_VAL); //%% cycles the Rank performed a Read
                printf("Channel%d_Rank%d_WriteCycles :          %9.2f\n", channel, rank, (double)writes * T_DATA_TRANS / CYCLE_VAL); //  # %% cycles the Rank performed a Write
                printf("Channel%d_Rank%d_Read_Other :            %9.2f\n", channel, rank,
                        ((double)stats_time_spent_terminating_reads_from_other_ranks[channel][rank] / CYCLE_VAL)); // # %% cycles other Ranks on the channel performed a Read
                stats_time_spent_terminating_reads_from_other_ranks_ch += stats_time_spent_terminating_reads_from_other_ranks[channel][rank];

                printf("Channel%d_Rank%d_Write_Other :           %9.2f\n", channel, rank,
                        ((double)stats_time_spent_terminating_writes_to_other_ranks[channel][rank] / CYCLE_VAL)); // # cycles other Ranks on the channel performed a Write
                stats_time_spent_terminating_writes_to_other_ranks_ch += stats_time_spent_terminating_writes_to_other_ranks[channel][rank];

                printf("Channel%d_Rank%d_PRE_PDN_FAST :          %9.2f\n", channel, rank,
                        ((double)stats_time_spent_in_precharge_power_down_fast[channel][rank] / CYCLE_VAL)); // # cycles the Rank was in Fast Power Down and all Banks were Precharged
                stats_time_spent_in_precharge_power_down_fast_ch += stats_time_spent_in_precharge_power_down_fast[channel][rank];
                printf("Channel%d_Rank%d_PRE_PDN_SLOW :         %9.2f\n", channel, rank,
                        ((double)stats_time_spent_in_precharge_power_down_slow[channel][rank] / CYCLE_VAL)); // # cycles the Rank was in Slow Power Down and all Banks were Precharged
                stats_time_spent_in_precharge_power_down_slow_ch += stats_time_spent_in_precharge_power_down_slow[channel][rank];
                printf("Channel%d_Rank%d_ACT_PDN :               %9.2f\n", channel, rank,
                        ((double)stats_time_spent_in_active_power_down[channel][rank] / CYCLE_VAL)); // # cycles the Rank was in Active Power Down and atleast one Bank was Active
                stats_time_spent_in_active_power_down_ch += stats_time_spent_in_active_power_down[channel][rank];
                printf("Channel%d_Rank%d_ACT_STBY :              %9.2f\n", channel, rank,
                        ((double)stats_time_spent_in_active_standby[channel][rank] / CYCLE_VAL)); // # cycles the Rank was in Standby and atleast one bank was Active
                stats_time_spent_in_active_standby_ch += stats_time_spent_in_active_standby[channel][rank];
                printf("Channel%d_Rank%d_PRE_STBY :              %9.2f\n", channel, rank, time_in_pre_stby); // # cycles the Rank was in Standby and all Banks were Precharged
                time_in_pre_stby_ch += time_in_pre_stby;
                printf("---------------------------------------------------------------\n\n");
            }
            else if (print_stats_type == 1)
            {
                /*----------------------------------------------------
                // Total Power is the sum total of all the components calculated above
                ----------------------------------------------------*/

                printf("Channel%d_Rank%d_Background(mw)          %9.2f\n", channel, rank, psch_act_pdn + psch_act_stby + psch_pre_pdn_slow + psch_pre_pdn_fast + psch_pre_stby); //  # depends only on Power Down time and time all banks were precharged
                background_ch += psch_act_pdn + psch_act_stby + psch_pre_pdn_slow + psch_pre_pdn_fast + psch_pre_stby;
                printf("Channel%d_Rank%d_Act(mW)                 %9.2f\n", channel, rank, psch_act); //  # power spend bringing data to the row buffer
                psch_act_ch += psch_act;
                printf("Channel%d_Rank%d_Read(mW)                %9.2f\n", channel, rank, psch_rd); //  # power spent doing a Read  after the Row Buffer is open
                psch_rd_ch += psch_rd;
                printf("Channel%d_Rank%d_Write(mW)               %9.2f\n", channel, rank, psch_wr); //  # power spent doing a Write after the Row Buffer is open
                psch_wr_ch += psch_wr;
                printf("Channel%d_Rank%d_Read_Terminate(mW)      %9.2f\n", channel, rank, psch_dq); //  # power dissipated in ODT resistors during Read
                psch_dq_ch += psch_dq;
                printf("Channel%d_Rank%d_Write_Terminate(mW)     %9.2f\n", channel, rank, psch_termW); //  # power dissipated in ODT resistors during Write
                psch_termW_ch += psch_termW;
                printf("Channel%d_Rank%d_termRoth(mW)            %9.2f\n", channel, rank, psch_termRoth); //  # power dissipated in ODT resistors during Reads  in other ranks
                psch_termRoth_ch += psch_termRoth;
                printf("Channel%d_Rank%d_termWoth(mW)            %9.2f\n", channel, rank, psch_termWoth); //  # power dissipated in ODT resistors during Writes in other ranks
                psch_termWoth_ch += psch_termWoth;
                printf("Channel%d_Rank%d_Refresh(mW)             %9.2f\n", channel, rank, psch_ref); //  # depends on frequency of Refresh (tREFI)
                psch_ref_ch += psch_ref;
                printf("---------------------------------------------------------------\n");
                printf("Channel%d_Rank%d_Total_Rank_Power(mW)    %9.2f\n", channel, rank, total_rank_power); //  # (Sum of above components)*(num chips in each Rank)
                total_rank_power_ch += total_rank_power;
                printf("---------------------------------------------------------------\n\n");

            }
            else
            {
                printf("PANIC: FN_CALL_ERROR: In calculate_power(), print_stats_type can only be 1 or 0\n");
                assert(-1);
            }
        }
        if (print_stats_type == 0)
        {
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("Channel%d_ReadCycles :           %9.2f\n", channel, (double)reads * T_DATA_TRANS / CYCLE_VAL); //%% cycles the Rank performed a Read
            printf("Channel%d_WriteCycles :          %9.2f\n", channel, (double)writes * T_DATA_TRANS / CYCLE_VAL); //  # %% cycles the Rank performed a Write
            printf("Channel%d_Read_Other :            %9.2f\n", channel,
                    ((double)stats_time_spent_terminating_reads_from_other_ranks_ch / CYCLE_VAL)); // # %% cycles other Ranks on the channel performed a Read
            stats_time_spent_terminating_reads_from_other_ranks_total += stats_time_spent_terminating_reads_from_other_ranks_ch;
            printf("Channel%d_Write_Other :           %9.2f\n", channel,
                    ((double)stats_time_spent_terminating_writes_to_other_ranks_ch/CYCLE_VAL)); // # cycles other Ranks on the channel performed a Write
            stats_time_spent_terminating_writes_to_other_ranks_total += stats_time_spent_terminating_writes_to_other_ranks_ch;
            printf("Channel%d_PRE_PDN_FAST :          %9.2f\n", channel,
                    ((double)stats_time_spent_in_precharge_power_down_fast_ch / CYCLE_VAL)); // # cycles the Rank was in Fast Power Down and all Banks were Precharged
            stats_time_spent_in_precharge_power_down_fast_total += stats_time_spent_in_precharge_power_down_fast_ch;
            printf("Channel%d_PRE_PDN_SLOW :         %9.2f\n", channel,
                    ((double)stats_time_spent_in_precharge_power_down_slow_ch / CYCLE_VAL)); // # cycles the Rank was in Slow Power Down and all Banks were Precharged
            stats_time_spent_in_precharge_power_down_slow_total += stats_time_spent_in_precharge_power_down_slow_ch;
            printf("Channel%d_ACT_PDN :               %9.2f\n", channel,
                    ((double)stats_time_spent_in_active_power_down_ch / CYCLE_VAL)); // # cycles the Rank was in Active Power Down and atleast one Bank was Active
            stats_time_spent_in_active_power_down_total += stats_time_spent_in_active_power_down_ch;
            printf("Channel%d_ACT_STBY :              %9.2f\n", channel,
                    ((double)stats_time_spent_in_active_standby_ch / CYCLE_VAL)); // # cycles the Rank was in Standby and atleast one bank was Active
            stats_time_spent_in_active_standby_total += stats_time_spent_in_active_standby_ch;
            printf("Channel%d_PRE_STBY :              %9.2f\n", channel, (double)time_in_pre_stby_ch); // # cycles the Rank was in Standby and all Banks were Precharged
            time_in_pre_stby_total += time_in_pre_stby_ch;
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
        }
        else if (print_stats_type == 1)
        {
            /*----------------------------------------------------
            // Total Power is the sum total of all the components calculated above
            ----------------------------------------------------*/

            printf("Channel%d_Background(mw) :         %9.2f\n", channel, background_ch); //  # depends only on Power Down time and time all banks were precharged
            background_total += background_ch;
            printf("Channel%d_Act(mW) :                %9.2f\n", channel, psch_act_ch); //  # power spend bringing data to the row buffer
            psch_act_total += psch_act_ch;
            printf("Channel%d_Read(mW) :               %9.2f\n", channel, psch_rd_ch); //  # power spent doing a Read  after the Row Buffer is open
            psch_rd_total += psch_rd_ch;
            printf("Channel%d_Write(mW) :              %9.2f\n", channel, psch_wr_ch); //  # power spent doing a Write after the Row Buffer is open
            psch_wr_total += psch_wr_ch;
            printf("Channel%d_Read_Terminate(mW) :     %9.2f\n", channel, psch_dq_ch); //  # power dissipated in ODT resistors during Read
            psch_dq_total += psch_dq_ch;
            printf("Channel%d_Write_Terminate(mW) :    %9.2f\n", channel, psch_termW_ch); //  # power dissipated in ODT resistors during Write
            psch_termW_total += psch_termW_ch;
            printf("Channel%d_termRoth(mW) :           %9.2f\n", channel, psch_termRoth_ch); //  # power dissipated in ODT resistors during Reads  in other ranks
            psch_termRoth_total += psch_termRoth_ch;
            printf("Channel%d_termWoth(mW) :           %9.2f\n", channel, psch_termWoth_ch); //  # power dissipated in ODT resistors during Writes in other ranks
            psch_termWoth_total += psch_termWoth_ch;
            printf("Channel%d_Refresh(mW) :            %9.2f\n", channel, psch_ref_ch); //  # depends on frequency of Refresh (tREFI)
            psch_ref_total += psch_ref_ch;
            printf("---------------------------------------------------------------\n");
            printf("Channel%d_Total_Rank_Power(mW) :    %9.2f\n", channel, total_rank_power_ch); //  # (Sum of above components)*(num chips in each Rank)
            total_power += total_rank_power_ch;
            printf("---------------------------------------------------------------\n\n");

        }
    }
    if (print_stats_type == 0){
        printf("===================================================\n");
        printf("Total_ReadCycles :           %9.2f\n",  (double)reads_total * T_DATA_TRANS / CYCLE_VAL);
        printf("Total_WriteCycles :          %9.2f\n",  (double)writes_total * T_DATA_TRANS / CYCLE_VAL);
        printf("Total_Read_Other :            %9.2f\n", ((double)stats_time_spent_terminating_reads_from_other_ranks_total / CYCLE_VAL));
        printf("Total_Write_Other :           %9.2f\n", ((double)stats_time_spent_terminating_writes_to_other_ranks_total/CYCLE_VAL));
        printf("Total_PRE_PDN_FAST :          %9.2f\n", ((double)stats_time_spent_in_precharge_power_down_fast_total / CYCLE_VAL));
        printf("Total_PRE_PDN_SLOW :         %9.2f\n",  ((double)stats_time_spent_in_precharge_power_down_slow_total / CYCLE_VAL));
        printf("Total_ACT_PDN :               %9.2f\n", ((double)stats_time_spent_in_active_power_down_total / CYCLE_VAL));
        printf("Total_ACT_STBY :              %9.2f\n", ((double)stats_time_spent_in_active_standby_total / CYCLE_VAL));
        printf("Total_PRE_STBY :              %9.2f\n", time_in_pre_stby_total);
        printf("===================================================\n\n");
    }  else if (print_stats_type == 1){
        printf("===================================================\n");
        printf("Total_Background(mw) :         %9.2f\n", background_total); //  # depends only on Power Down time and time all banks were precharged
        printf("Total_Act(mW) :                %9.2f\n", psch_act_total); //  # power spend bringing data to the row buffer
        printf("Total_Read(mW) :               %9.2f\n", psch_rd_total); //  # power spent doing a Read  after the Row Buffer is open
        printf("Total_Write(mW) :              %9.2f\n", psch_wr_total); //  # power spent doing a Write after the Row Buffer is open
        printf("Total_Read_Terminate(mW) :     %9.2f\n", psch_dq_total); //  # power dissipated in ODT resistors during Read
        printf("Toral_Write_Terminate(mW) :    %9.2f\n", psch_termW_total); //  # power dissipated in ODT resistors during Write
        printf("Total_termRoth(mW) :           %9.2f\n", psch_termRoth_total); //  # power dissipated in ODT resistors during Reads  in other ranks
        printf("Total_termWoth(mW) :           %9.2f\n", psch_termWoth_total); //  # power dissipated in ODT resistors during Writes in other ranks
        printf("Total_Refresh(mW) :            %9.2f\n", psch_ref_total); //  # depends on frequency of Refresh (tREFI)
        printf("---------------------------------------------------------------\n");
        printf("Total_Power(mW) :              %9.2f\n", total_power); //  # (Sum of above components)*(num chips in each Rank)
        printf("---------------------------------------------------------------\n\n");
        printf("===================================================\n\n");

    }
    return total_power;
}
