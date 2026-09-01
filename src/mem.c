#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include <string.h>
#include <ncurses.h>

long long* parse_memory_info(long long *mem_info, const char *file){
    FILE *fp = fopen(file, "r");
    if (fp == NULL){
        return NULL;
    }
    char line[256];
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lld", &mem_info[MEM_TOTAL]);
        }
        else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %lld", &mem_info[MEM_AVAILABLE]);
        }
        else if (strncmp(line, "Buffers:", 8) == 0){
            sscanf(line, "Buffers: %lld", &mem_info[MEM_BUFFERS]);
        }
        else if (strncmp(line, "Cached:", 7) == 0){
            sscanf(line, "Cached: %lld", &mem_info[MEM_CACHED]);
        }
        else if (strncmp(line, "SwapTotal:", 10) == 0){
            sscanf(line, "SwapTotal: %lld", &mem_info[SWAP_TOTAL]);
        }
        else if (strncmp(line, "SwapFree:", 9) == 0){
            sscanf(line, "SwapFree: %lld", &mem_info[SWAP_FREE]);
            break;
        }
    }

    fclose(fp);
    return mem_info;
}

int print_main_memory_usage(long long* mem_info, int start_row, int offset){
    int screen_row = start_row - offset;
    if (screen_row >= 0 && screen_row < LINES){
        long long usage = mem_info[MEM_TOTAL] - mem_info[MEM_AVAILABLE];
        float percentage = (float) usage / mem_info[MEM_TOTAL] * 100;
        mvprintw(screen_row, 0, "Memory Usage: %lld MB (%.2f%%)", usage/1024, percentage);
    }
    return 1;
}

int print_swap_and_cache_info(long long* mem_info, int start_row, int offset){
    for (int i = 0; i < 3; i++){
        int screen_row = start_row + i - offset;
        if (screen_row < 0 || screen_row >= LINES) continue;

        if (i == 0) mvprintw(screen_row, 0, "Buffers: %lld MB", mem_info[MEM_BUFFERS]/1024);
        else if (i == 1) mvprintw(screen_row, 0, "Cached: %lld MB", mem_info[MEM_CACHED]/1024);
        else {
            long long swap_used = mem_info[SWAP_TOTAL] - mem_info[SWAP_FREE];
            float swap_pct = mem_info[SWAP_TOTAL] ? (float)swap_used / mem_info[SWAP_TOTAL] * 100 : 0.0;
            mvprintw(screen_row, 0, "Swap usage: %lld MB (%.2f%%)", swap_used/1024, swap_pct);
        }
    }
    return 7;
}