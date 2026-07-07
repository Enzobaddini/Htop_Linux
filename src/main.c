#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpu.h"
#include "mem.h"

int main(int argc, char *argv[]) {
    
    CPUStats *stats = initialize_cpu_stats();
    stats = parse_cpu_stats("/proc/stat", &stats);
    if (stats == NULL) {
        return EXIT_FAILURE;
    }

    sleep(1); // window between samples; /proc/stat counters are cumulative, need a delta

    CPUStats *stats2 = initialize_cpu_stats();
    stats2 = parse_cpu_stats("/proc/stat", &stats2);
    if (stats2 == NULL) {
        return EXIT_FAILURE;
    }

    int total = (int) sysconf(_SC_NPROCESSORS_CONF) + 1;
    float* list_cpu = (float*)malloc(total*sizeof(float));
    if (list_cpu == NULL)  {
        return EXIT_FAILURE;
    }
    calculate_cpu_usage(stats, stats2, list_cpu);

    print_cpu_usage(list_cpu, total);
    
    // Free the linked lists
    free_cpu_stats_list(stats);
    free_cpu_stats_list(stats2);
    free(list_cpu);

    long long mem_info[MEM_INFO_COUNT];
    parse_memory_info(mem_info, "/proc/meminfo");
    print_main_memory_usage(mem_info);
    print_swap_and_cache_info(mem_info);

    return 0;
}