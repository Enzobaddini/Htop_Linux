typedef struct process{
    int pid; //1
    char name[256]; //2
    char state; //3
    int ppid; //4 
    long long user_time; //14
    long long kernel_time; //15
    int num_threads; //20
    long long rss; //24
    int last_cpu; //39
    struct process* next;
} Process;

typedef struct hash{
    Process** buckets;
    int size;
} Hash;
