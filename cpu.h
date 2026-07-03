typedef struct cpu_stats {
    unsigned long long info;
    char name[10];
    struct cpu_stats *next;
} CPUStats;

CPUStats *inicialize_cpu_stats();

CPUStats* atribuite_cpu_stats(CPUStats *stats, char *file, char **name_list); // parses first 7 fields of /proc/stat's "cpu" line, in name_list's order

CPUStats* create_node(CPUStats *stats, unsigned long long info, char *field_name); // create a new node and fill it with the information from /proc/stat

float calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2); // returns fraction [0,1]; stats2 must be a later sample than stats1, same field order

void print_stats(CPUStats *stats); 

void free_stats(CPUStats *stats);