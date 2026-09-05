#ifndef PID_H
#define PID_H

typedef struct PidList{
    int *pid;
    int count;
}PidList;

struct hash;
typedef struct hash Hash;

PidList* parse_pid(const char* file);

PidList* filter_pids(PidList* raw, Hash* hash, const char* number, const char* pid_name);

int is_pid(const char* name);

void free_pid_list(PidList* values);

void debug(PidList* values);

#endif