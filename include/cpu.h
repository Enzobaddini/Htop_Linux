typedef struct CPUStats {
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    struct CPUStats *next;
} CPUStats;

CPUStats *initialize_cpu_stats();

CPUStats* parse_cpu_stats(const char *file, CPUStats **stats); // parses first 7 fields of /proc/stat's "cpu" line, in name_list's order

void calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2, float *results); // returns fraction [0,1]; stats2 must be a later sample than stats1, same field order

void print_cpu_usage(float *results, int total);

void print_cpu_stats_debug(CPUStats *stats); 

void free_cpu_stats_list(CPUStats *stats);