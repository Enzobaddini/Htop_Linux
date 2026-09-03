#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include "pid.h"
#include "process.h"
#include "ui.h"
#include <ncurses.h>


static Hash* current_hash = NULL;

void handle_scroll_input(int ch, int* offset, int total, int visible) {

    int max_offset = (total > visible) ? (total - visible) : 0;
    
    switch (ch) {
        case KEY_DOWN:
            if (*offset < max_offset) {
                (*offset)++;
            }
            break;
        case KEY_UP:
            if (*offset > 0) {
                (*offset)--;
            }
            break;
        case KEY_NPAGE:
            *offset += visible;
            break;
        case KEY_PPAGE:
            *offset -= visible;
            break;
        case KEY_END:
            *offset = max_offset;
            break;
        case KEY_HOME:
            *offset = 0;
            break;
    }
    if (*offset < 0) *offset = 0;
    if (*offset > max_offset) *offset = max_offset;
    
}


void scroll_window(int total, int visible, int offset, int bar_height, int start_row) {
    if (total <= visible || total == 0) {
        for (int i = 0; i < bar_height; i++)
            mvaddch(start_row + i, COLS - 1, ACS_VLINE);
        return;
    }

    int thumb_size = (visible * bar_height) / total;
    if (thumb_size < 1) thumb_size = 1;

    int thumb_pos = (offset * (bar_height - thumb_size)) / (total - visible);


    for (int i = 0; i < bar_height; i++) {
        if (i >= thumb_pos && i < thumb_pos + thumb_size)
            mvaddch(start_row + i, COLS - 1, ACS_CKBOARD);
        else
            mvaddch(start_row + i, COLS - 1, ACS_VLINE);
    }
}

PidList* organize_process(int ch, PidList* list, Hash* hash, int* flag){


    switch (ch) {
        
    case 32:

        qsort(list->pid, list->count, sizeof(int), compare_by_pid);
        (*flag) = 1;
        break;
        
    case 9:
            
        current_hash = hash;
        qsort(list->pid, list->count, sizeof(int), compare_by_name);
        (*flag) = 2;
        break;
        
    case 10:

        current_hash = hash;
        qsort(list->pid, list->count, sizeof(int), compare_by_cpu);
        (*flag) = 3;
        break;    
        
    default:

        break;    
            
    }
    

    if (*flag == 1) qsort(list->pid, list->count, sizeof(int), compare_by_pid);
    else if (*flag == 2) qsort(list->pid, list->count, sizeof(int), compare_by_name);
    else if (*flag == 3) qsort(list->pid, list->count, sizeof(int), compare_by_cpu);

    
    return list;
}


int compare_by_pid(const void* a, const void* b) {
    int pid_a = *(const int*)a;
    int pid_b = *(const int*)b;
    return (pid_a > pid_b) - (pid_a < pid_b);
}

int compare_by_name(const void* a, const void* b) {
    int pid_a = *(const int*)a;
    int pid_b = *(const int*)b;

    Process* process_a = find_process(current_hash, pid_a);
    Process* process_b = find_process(current_hash, pid_b);

    if (process_a == NULL || process_b == NULL) {
        return 0; 
    }

    return strcasecmp(process_a->name, process_b->name);

}

int compare_by_cpu(const void* a, const void* b) {
    int pid_a = *(const int*)a;
    int pid_b = *(const int*)b;

    Process* process_a = find_process(current_hash, pid_a);
    Process* process_b = find_process(current_hash, pid_b);

    if (process_a == NULL || process_b == NULL) {
        return 0; 
    }

    long long cpu_time_a = process_a->user_time + process_a->kernel_time;
    long long cpu_time_b = process_b->user_time + process_b->kernel_time;

    return (cpu_time_b > cpu_time_a) - (cpu_time_b < cpu_time_a);
}

char* insert_find(int ch, char* find, int* tam) {
    if (find != NULL) {    
        if (ch >= 48 && ch <= 57){
            find[(*tam)] = (char) ch;
            (*tam)++;
            find[*tam] = '\0';

        }

        else if (*tam > 0 && (ch == KEY_BACKSPACE || ch == 8 || ch == 127)) {
            (*tam)--;
            find[*tam] = '\0';
        }
    }
    return find;
}


