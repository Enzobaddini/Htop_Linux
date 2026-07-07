#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpu.h"
#include "mem.h"

int main(int argc, char *argv[]) {
    
    CPUStats *stats = inicialize_cpu_stats();
    stats = atribuite_cpu_stats("/proc/stat", &stats);
    
    sleep(1); // window between samples; /proc/stat counters are cumulative, need a delta

    CPUStats *stats2 = inicialize_cpu_stats();
    stats2 = atribuite_cpu_stats("/proc/stat", &stats2);
    
    calculate_cpu_usage(stats, stats2);

    // Free the linked lists
    free_stats(stats);
    free_stats(stats2);
    
    long long mem_info[6];
    get_memory_info(mem_info, "/proc/meminfo");
    memory_used(mem_info);
    memory_information(mem_info);

    return 0;
}