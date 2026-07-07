#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include <string.h>

long long* parse_memory_info(long long *mem_info, const char *file){
    FILE *fp = fopen(file, "r");
    if (fp == NULL){
        perror("Error opening file");
        exit(1);
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
            break; // No need to continue reading after SwapFree
        }
    }

    fclose(fp);
    return mem_info;
}

void print_main_memory_usage(long long* mem_info){
    long long usage = (mem_info[MEM_TOTAL] - mem_info[MEM_AVAILABLE]);
    float percentage = (float) usage / mem_info[MEM_TOTAL] * 100;
    printf("Memory Usage: %lld MB (%.2f%%)\n", usage/1024, percentage);
}

void print_swap_and_cache_info(long long* mem_info){
    printf("Buffers: %lld MB\n", mem_info[MEM_BUFFERS]/1024);
    printf("Cached: %lld MB\n", mem_info[MEM_CACHED]/1024);
    
    long long swap_used = mem_info[SWAP_TOTAL] - mem_info[SWAP_FREE];
    
    float swap_percentage = 0.0;
    if (mem_info[SWAP_TOTAL] != 0){
        swap_percentage = (float) swap_used / mem_info[SWAP_TOTAL] * 100;
    }

    printf("Swap usage: %lld MB (%.2f%%)\n", swap_used/1024, swap_percentage);
}