#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpu.h"
#include "mem.h"
#include "pid.h"
#include "process.h"

int main() {
    CPUStats *stats1 = initialize_cpu_stats();
    stats1 = parse_cpu_stats("/proc/stat", &stats1);
    if (stats1 == NULL) return EXIT_FAILURE;

    sleep(1);

    CPUStats *stats2 = initialize_cpu_stats();
    stats2 = parse_cpu_stats("/proc/stat", &stats2);
    if (stats2 == NULL) return EXIT_FAILURE;

    int total = (int) sysconf(_SC_NPROCESSORS_CONF) + 1;
    float *cpu_usage = (float*)malloc(total * sizeof(float));
    if (cpu_usage == NULL) return EXIT_FAILURE;

    calculate_cpu_usage(stats1, stats2, cpu_usage);
    print_cpu_usage(cpu_usage, total);

    free_cpu_stats_list(stats1);
    free_cpu_stats_list(stats2);
    free(cpu_usage);

    long long mem_info[MEM_INFO_COUNT];
    parse_memory_info(mem_info, "/proc/meminfo");
    print_main_memory_usage(mem_info);
    print_swap_and_cache_info(mem_info);

    printf("\n");

    PidList *list_pids = parse_pid("/proc");
    Hash *hash = create_hash_map(512);

    for (int i = 0; i < list_pids->count; i++)
        hash = insert_process(hash, list_pids->pid[i]);

    print_process_info(hash);

    mark_all_stale(hash);

    sleep(1);

    free_pid_list(list_pids);
    list_pids = parse_pid("/proc");

    for (int i = 0; i < list_pids->count; i++)
        hash = update_process(hash, list_pids->pid[i]);

    hash = check(hash);

    printf("\n");
    print_process_info(hash);

    free_hash(hash);
    free_pid_list(list_pids);

    return 0;
}