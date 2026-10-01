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
    mousemask(BUTTON1_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);


    int offset = 0;
    int ch;
    int flag = 0;
    char* find = (char*)malloc(sizeof(char) * 1000);
    char* pid_name = (char*)malloc(sizeof(char) * 1000);
    find[0] = '0'; 
    find[1] = '\0';  
    pid_name[0] = '\0';  
    int size = 0;
    int select_line = 0;
    int k = 0;
    int search_focus = 0;


    Hash *hash = create_hash_map(512);
    PidList *raw = parse_pid("/proc");
    
    for (int i = 0; i < raw->count; i++){
        hash = insert_process(hash, raw->pid[i]);
    } 
    
    PidList *list_pids = filter_pids(raw, hash, "0", "");

    prune_dead_pids(list_pids, hash);  

    while (1){   
    
        CPUStats *stats1 = initialize_cpu_stats();
        stats1 = parse_cpu_stats("/proc/stat", &stats1);
        if (stats1 == NULL) return EXIT_FAILURE;

        ch = getch();

        if (ch == KEY_MOUSE) {
            MEVENT ev;
        if (getmouse(&ev) == OK && (ev.bstate & BUTTON1_CLICKED))
            search_focus = search_bar_hit(ev.y, ev.x);
        }
        
        if (search_focus) {
            find = insert_find(ch, find, &size, pid_name);
        } 
        
        else if (ch == '/') {
            search_focus = 1;
        } 
        
        else if (ch == 27) {
            break;
        }


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
        free_pid_list(raw);
        raw = parse_pid("/proc");

        for (int i = 0; i < raw->count; i++){
            hash = update_process(hash, raw->pid[i]);
        }
        hash = check(hash);
        prune_dead_pids(raw, hash);

        free_pid_list(list_pids);
        list_pids = filter_pids(raw, hash, find, pid_name);
        list_pids = organize_process(ch, list_pids, hash, &flag);

        int cpu_rows = cpu_usage_row_count(total);
        int mem_rows = 4;
        int total_lines = cpu_rows + mem_rows + list_pids->count;
        int visible = LINES;


        if (ch == 27) break; 

        handle_scroll_input(ch, &offset, total_lines, visible, &select_line, list_pids->count);

        clear();
        int row = 0;
        row += print_cpu_usage(cpu_usage, total, row, offset);
        row += print_main_memory_usage(mem_info, row, offset);
        row += print_swap_and_cache_info(mem_info, row, offset);
        row += print_process_info(hash, list_pids, row, offset, select_line, ch, &k);
        scroll_window(total_lines, visible, offset, visible, 0);
        for (int i = 0; i < raw->count; i++){
            hash = update_process(hash, raw->pid[i]);
        }
        hash = check(hash);
        prune_dead_pids(raw, hash);
        draw_search_bar(find, pid_name, search_focus);
        refresh();
        if (ch == KEY_DC && list_pids->count > 0) kill_window(&k);

        free(cpu_usage);
        free_cpu_stats_list(stats1);
        free_cpu_stats_list(stats2);

        
    }
    
    free_pid_list(raw);
    free_pid_list(list_pids);
    free_hash(hash);
    free(find);
    free(pid_name);
    
    endwin();
    
   
    return 0;
}