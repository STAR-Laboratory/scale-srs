#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

struct node{
    long long int comptime;
    struct node *next;
};

struct robstructure
{
  int head;
  int tail;
  int inflight;
  struct node * entry;
  long long int * mem_address;
  int * optype;
  long long int * instrpc;
  int tracedone;
  int * cachemiss;
  int *queue;
};

#endif //__PROCESSOR_H__

