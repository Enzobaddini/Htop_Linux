typedef struct CPUStats {
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    struct CPUStats *next;
} CPUStats;

CPUStats *inicialize_cpu_stats();

CPUStats* atribuite_cpu_stats(char *file, CPUStats **stats); // parses first 7 fields of /proc/stat's "cpu" line, in name_list's order

void calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2); // returns fraction [0,1]; stats2 must be a later sample than stats1, same field order

void print_stats(CPUStats *stats); 

void free_stats(CPUStats *stats);