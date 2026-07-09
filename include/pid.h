typedef struct PidList{
    int *pid;
    int count;
}PidList;

PidList* parse_pid(const char* file);

int is_pid(const char* name);

void debug(PidList* values);

void free_pid_list(PidList* values);