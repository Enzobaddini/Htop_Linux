#ifndef PID_H
#define PID_H

typedef struct PidList{
    int *pid;
    int count;
}PidList;

PidList* parse_pid(const char* file, const char* number);

int is_pid(const char* name);

void free_pid_list(PidList* values);

void debug(PidList* values);

#endif