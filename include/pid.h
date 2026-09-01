
typedef struct PidList{
    int *pid;
    int count;
}PidList;

PidList* parse_pid(const char* file);

int is_pid(const char* name);

void free_pid_list(PidList* values);

void debug(PidList* values);