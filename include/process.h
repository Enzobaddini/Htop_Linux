#ifndef PROCESS_H
#define PROCESS_H

typedef struct process{
    int pid; 
    char name[256]; 
    char state; 
    int ppid; 
    long long user_time; 
    long long kernel_time; 
    int num_threads; 
    long long rss; 
    int last_cpu; 
    int stale;
    struct process* next;
} Process;

typedef struct hash{
    Process** buckets;
    int size;
} Hash;

#define LIST_START_ROW 25

int print_process_info(Hash* hash, PidList* list_pid, int start_row, int offset, int visible, int select, int ch, int* k);

void free_hash(Hash* hash);

Hash* check(Hash* hash);

void mark_all_stale(Hash* hash);

Hash* update_process(Hash* hash, int pid);

Hash* remove_process(Hash* hash, int pid);

Process* find_process(Hash* hash, int pid);

Hash* insert_process(Hash* hash, int pid);

Process* parse_process(int pid);

Hash* create_hash_map(int size);

void prune_dead_pids(PidList* list, Hash* hash);

#endif

#include "pid.h"
