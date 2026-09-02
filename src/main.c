#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "pid.h"
#include "process.h"
#include "ui.h"
#include <ncurses.h>



int main() {
    
    initscr();
    set_escdelay(25);
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    timeout(1000);
    curs_set(0);

    int offset = 0;
    int ch;
    int flag = 0;

    Hash *hash = create_hash_map(512);
    PidList *list_pids = parse_pid("/proc", "1");

    for (int i = 0; i < list_pids->count; i++){
        hash = insert_process(hash, list_pids->pid[i]);
    }
    prune_dead_pids(list_pids, hash);  

    while (1){   
    
        CPUStats *stats1 = initialize_cpu_stats();
        stats1 = parse_cpu_stats("/proc/stat", &stats1);
        if (stats1 == NULL) return EXIT_FAILURE;

        ch = getch();
        if (ch == 27) break;

        CPUStats *stats2 = initialize_cpu_stats();
        stats2 = parse_cpu_stats("/proc/stat", &stats2);
        if (stats2 == NULL) return EXIT_FAILURE;

        int n1 = cpu_stats_count(stats1);
        int n2 = cpu_stats_count(stats2);
        int total = n1 < n2 ? n1 : n2;
        if (total <= 0) return EXIT_FAILURE;

        float *cpu_usage = (float*)calloc((size_t)total, sizeof(float));
        if (cpu_usage == NULL) return EXIT_FAILURE;

        calculate_cpu_usage(stats1, stats2, cpu_usage, total);

        long long mem_info[MEM_INFO_COUNT];
        memset(mem_info, 0, sizeof(mem_info));
        parse_memory_info(mem_info, "/proc/meminfo");
        
        mark_all_stale(hash);
        free_pid_list(list_pids);
        list_pids = parse_pid("/proc", "1");
        
        for (int i = 0; i < list_pids->count; i++){
            hash = update_process(hash, list_pids->pid[i]);
        }
        hash = check(hash);
        prune_dead_pids(list_pids, hash);  
        list_pids = organize_process(ch, list_pids, hash, &flag);

        int cpu_rows = cpu_usage_row_count(total);
        int mem_rows = 4;
        int total_lines = cpu_rows + mem_rows + list_pids->count;
        int visible = LINES;


        if (ch == 27) break; 

        handle_scroll_input(ch, &offset, total_lines, visible);

        clear();
        int row = 0;
        row += print_cpu_usage(cpu_usage, total, row, offset);
        row += print_main_memory_usage(mem_info, row, offset);
        row += print_swap_and_cache_info(mem_info, row, offset);
        row += print_process_info(hash, list_pids, row, offset);
        scroll_window(total_lines, visible, offset, visible, 0);
        refresh();

        free(cpu_usage);
        free_cpu_stats_list(stats1);
        free_cpu_stats_list(stats2);

        
    }
    
    free_pid_list(list_pids);
    free_hash(hash);
    
    endwin();
    
   
    return 0;
}