#ifndef CPU_H
#define CPU_H

typedef struct CPUStats {
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    struct CPUStats *next;
} CPUStats;

CPUStats *initialize_cpu_stats();

CPUStats* parse_cpu_stats(const char *file, CPUStats **stats);

int cpu_stats_count(CPUStats *stats);

void calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2, float *results, int nresults);

int cpu_usage_row_count(int total);

int print_cpu_usage(float *results, int total, int start_row, int offset);

void print_cpu_stats_debug(CPUStats *stats); 

void free_cpu_stats_list(CPUStats *stats);

int cpu_sample_due(void);

#endif