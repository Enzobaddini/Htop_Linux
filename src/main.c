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

    CPUStats *cpu_prev = initialize_cpu_stats();
    cpu_prev = parse_cpu_stats("/proc/stat", &cpu_prev);
    if (cpu_prev == NULL) return EXIT_FAILURE;
    int total = cpu_stats_count(cpu_prev);
    float *cpu_usage = (float*)calloc((size_t)total, sizeof(float));
    if (cpu_usage == NULL) return EXIT_FAILURE;

    while (1){   
    

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


        if (cpu_sample_due()) {
            CPUStats *cpu_next = initialize_cpu_stats();
            cpu_next = parse_cpu_stats("/proc/stat", &cpu_next);
            if (cpu_next == NULL) return EXIT_FAILURE;
            int n = cpu_stats_count(cpu_next);
            int t = n < total ? n : total;
            calculate_cpu_usage(cpu_prev, cpu_next, cpu_usage, t);
            free_cpu_stats_list(cpu_prev);
            cpu_prev = cpu_next;
        }

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
        int header_rows = cpu_rows + mem_rows;
        int body_visible = LINES - header_rows;
        if (body_visible < 1) body_visible = 1; 

        handle_scroll_input(ch, &offset, list_pids->count, body_visible, &select_line);

        clear();
        int row = 0;
        row += print_cpu_usage(cpu_usage, total, row, 0);
        row += print_main_memory_usage(mem_info, row, 0);
        row += print_swap_and_cache_info(mem_info, row, 0);
        print_process_info(hash, list_pids, header_rows, offset, body_visible, select_line, ch, &k);
        scroll_window(list_pids->count, body_visible, offset, body_visible, header_rows);
        for (int i = 0; i < raw->count; i++){
            hash = update_process(hash, raw->pid[i]);
        }
        hash = check(hash);
        prune_dead_pids(raw, hash);
        draw_search_bar(find, pid_name, search_focus);
        refresh();
        if (ch == KEY_DC && list_pids->count > 0) kill_window(&k);


        
    }
    
    free_pid_list(raw);
    free_pid_list(list_pids);
    free_hash(hash);
    free(find);
    free(pid_name);
    free(cpu_usage);
    free_cpu_stats_list(cpu_prev);
    
    endwin();
    
   
    return 0;
}