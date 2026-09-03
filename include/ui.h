#ifndef UI_H
#define UI_H


void handle_scroll_input(int ch, int* offset, int total, int visible);

void scroll_window(int total, int visible, int offset, int bar_height, int start_row);

PidList* organize_process(int ch, PidList* list, Hash* hash, int* flag);

int compare_by_pid(const void* a, const void* b);

int compare_by_name(const void* a, const void* b);

int compare_by_cpu(const void* a, const void* b);

char* insert_find(int ch, char* find, int* tam);

#endif

#include "pid.h"
#include "process.h"