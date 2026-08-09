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

void print_process_info(Hash* hash);

void free_hash(Hash* hash);

Hash* check(Hash* hash);

void mark_all_stale(Hash* hash);

Hash* update_process(Hash* hash, int pid);

Hash* remove_process(Hash* hash, int pid);

Process* find_process(Hash* hash, int pid);

Hash* insert_process(Hash* hash, int pid);

Process* parse_process(int pid);

Hash* create_hash_map(int size);