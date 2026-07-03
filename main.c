#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpu.h"

int main(int argc, char *argv[]) {
    
    char *list[7] = { // order matters: paired positionally with /proc/stat's fields
        "user", "nice", "system", "idle", "iowait", "irq", "softirq" 
    };

    CPUStats *stats = inicialize_cpu_stats();
    stats = atribuite_cpu_stats(stats, "/proc/stat", list);

    sleep(1); // window between samples; /proc/stat counters are cumulative, need a delta
    
    CPUStats *stats2 = inicialize_cpu_stats();
    stats2 = atribuite_cpu_stats(stats2, "/proc/stat", list);
    
    printf("CPU Usage: %.2f%%\n", calculate_cpu_usage(stats, stats2)*100);
    

    // Free the linked lists
    free_stats(stats);
    free_stats(stats2);
    
    return 0;
}