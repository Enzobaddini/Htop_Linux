#ifndef UI_H
#define UI_H

#include <ncurses.h>

void handle_scroll_input(int ch, int* offset, int total, int visible, int* select, int process_count);

void scroll_window(int total, int visible, int offset, int bar_height, int start_row);

PidList* organize_process(int ch, PidList* list, Hash* hash, int* flag);

int compare_by_pid(const void* a, const void* b);

int compare_by_name(const void* a, const void* b);

int compare_by_cpu(const void* a, const void* b);

char* insert_find(int ch, char* find, int* size, char* pid_name);

WINDOW *create_centered_window(int height, int width);

void kill_window(int* k);


#endif

#include "pid.h"
#include "process.h"
