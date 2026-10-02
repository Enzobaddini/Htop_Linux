#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include "pid.h"
#include "process.h"
#include "ui.h"
#include <ncurses.h>
#include <string.h>
#include <errno.h>


static Hash* current_hash = NULL;

void handle_scroll_input(int ch, int* offset, int count, int visible, int* select) {
    int max_select = (count > 0) ? (count - 1) : 0;
    int max_offset = (count > visible) ? (count - visible) : 0;

    switch (ch) {
        case KEY_DOWN:  (*select)++;        break;
        case KEY_UP:    (*select)--;        break;
        case KEY_NPAGE: *select += visible; break;
        case KEY_PPAGE: *select -= visible; break;
        case KEY_HOME:  *select = 0;        break;
        case KEY_END:   *select = max_select; break;
    }
    if (*select < 0) *select = 0;
    if (*select > max_select) *select = max_select;

    if (*select < *offset) *offset = *select;
    else if (*select >= *offset + visible) *offset = *select - visible + 1;

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

char* insert_find(int ch, char* find, int* size, char* pid_name) {
    if (ch >= 48 && ch <= 57) { 
        if (pid_name[0] != '\0') { 
            pid_name[0] = '\0';
            *size = 0; 
        }
        find[*size] = (char) ch;
        (*size)++;
        find[*size] = '\0';
    }
    
    else if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122) || ch == 95) {
        if (!(find[0] == '0' && find[1] == '\0')) { 
            *size = 0; 
        }
        pid_name[*size] = (char) ch;
        (*size)++;
        pid_name[*size] = '\0';
        
        find[0] = '0';
        find[1] = '\0';
    }
    
    else if (*size > 0 && (ch == KEY_BACKSPACE || ch == 8 || ch == 127)) {
        
        if (pid_name[0] != '\0') {
            (*size)--;
            pid_name[*size] = '\0'; 
        }
        else { 
            (*size)--;
            find[*size] = '\0'; 
        }
    }
    return find;
}


WINDOW *create_centered_window(int height, int width){
    int max_y, max_x;

    getmaxyx(stdscr, max_y, max_x);

    if (height > max_y) height = max_y;
    if (width  > max_x) width  = max_x;

    int start_y = (max_y - height) / 2;
    int start_x = (max_x - width) / 2;

    return newwin(height, width, start_y, start_x);
}


void kill_window(int* k){
    WINDOW* win = create_centered_window(10, 70);
    if (win == NULL){
        *k = 0;
        return;
    }
    box(win, 0, 0);

    const char* msg;
    if(*k == 0) msg = "SIGTERM sent to the process.";
    else if(*k == EPERM) msg = "You don't have permission to kill this process!";
    else msg = strerror(*k);
    *k = 0;

    int col = (getmaxx(win) - (int)strlen(msg)) / 2;
    if (col < 1) col = 1;

    wattron(win, A_BOLD);
    mvwprintw(win, getmaxy(win) / 2 - 1, col, "%s", msg);
    wattroff(win, A_BOLD);
    wrefresh(win);

    napms(3500);
    flushinp();           
    werase(win);
    wrefresh(win);
    delwin(win);

    touchwin(stdscr);
    refresh();
}

int search_bar_hit(int y, int x){
    int bx = COLS - SEARCH_W - 2;       
    return y == 0 && x >= bx && x < bx + SEARCH_W;
}

void draw_search_bar(const char* find, const char* pid_name, int focus){
    int bx = COLS - SEARCH_W - 2;
    if (bx < 0) return;

    const char* text = pid_name[0] ? pid_name : (strcmp(find, "0") ? find : "");

    int len = (int)strlen(text);
    int max = SEARCH_W - 4;

    if (len > max)
        text += len - max;

    char shown[SEARCH_W];
    char buf[SEARCH_W + 1];

    if (text[0] == '\0' && !focus)
        shown[0] = '\0';
    else
        snprintf(shown, sizeof(shown), "%s%s",
                 text,
                 focus ? "_" : "");

    snprintf(buf, sizeof(buf),
             "[%-*.*s]",
             SEARCH_W - 2,
             SEARCH_W - 2,
             shown);

    if (focus)
        attron(A_REVERSE);

    mvaddstr(0, bx, buf);

    if (focus)
        attroff(A_REVERSE);
}


