#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

void init_scheduler_vars(); //called from main
void scheduler_stats(); //called from main

void schedule_frfcfs(int);
void schedule_fcfs(int);
void schedule_closepage(int);
void schedule_lps(int);
void schedule_tp(int, unsigned int *runcore);
void schedule_bta(int);
void schedule_camou(int);
void schedule_fs(int, unsigned int *runcore);
void schedule_fsopen(int channel, unsigned int *runcore);
void schedule_bl(int channel, unsigned int *runcore, unsigned long long int **timer_bank, long long int *change_core);
void schedule_blopen(int channel, unsigned int *runcore, unsigned long long int **timer_bank, long long int *change_core);
void schedule_pr(int);
void schedule_pq(int, unsigned int *runcore, long long int **num_issue, unsigned long long int *timer, unsigned long long int ***prefetch_buffer, int buffer_size, unsigned long long int *fakereadreqfunc);
void schedule_pb(int channel, unsigned int *runcore, tracker_t ***per_thread_tracker);
void schedule_bp(int channel, unsigned int *runcore, unsigned int **bank, unsigned int ***issued, unsigned int ***bank_busy, unsigned int **bank_group, unsigned long long int req_id);
void schedule_secmc(int channel, unsigned int *runcore, unsigned int **bank, unsigned int ***issued, unsigned int ***bank_busy, unsigned int **switch_core, unsigned int **bank_group, int skip, unsigned long long int req_id);
void fill_in_tracker(int channel, unsigned int *runcore, tracker_t ***per_thread_tracker);
unsigned long long int get_demreq_addr(int, unsigned int *runcore);
unsigned long long int get_demreq_next_addr(int, unsigned int *runcore);

int misraGries(int c, int r, int b, unsigned long long int rowaddr);
int hydra(int channel, unsigned long int rank, unsigned long int bank, unsigned long int row, unsigned long int col, unsigned long long int physical_address);
void resetCC();
int rqidealRIT(unsigned long long int phyaddr);
#endif //__SCHEDULER_H__

