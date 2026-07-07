#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include <string.h>

long long* get_memory_info(long long *mem_info, char *file){
    FILE *fp = fopen(file, "r");
    if (fp == NULL){
        perror("Error opening file");
        exit(1);
    }
    char line[256];
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lld", &mem_info[0]);
        }
        else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %lld", &mem_info[1]);
        }
        else if (strncmp(line, "Buffers:", 8) == 0){
            sscanf(line, "Buffers: %lld", &mem_info[2]);
        }
        else if (strncmp(line, "Cached:", 7) == 0){
            sscanf(line, "Cached: %lld", &mem_info[3]);
        }
        else if (strncmp(line, "SwapTotal:", 10) == 0){
            sscanf(line, "SwapTotal: %lld", &mem_info[4]);
        }
        else if (strncmp(line, "SwapFree:", 9) == 0){
            sscanf(line, "SwapFree: %lld", &mem_info[5]);
            break; // No need to continue reading after SwapFree
        }
    }
    fclose(fp);
    return mem_info;
}

void memory_used(long long* mem_info){
    long long usage = (mem_info[0] - mem_info[1]);
    float percentage = (float) usage / mem_info[0] * 100;
    printf("Memory Usage: %lld MB (%.2f%%)\n", usage/1024, percentage);
}

void memory_information(long long* mem_info){
    printf("Buffers: %lld MB\n", mem_info[2]/1024);
    printf("Cached: %lld MB\n", mem_info[3]/1024);
    
    long long swap_used = mem_info[4] - mem_info[5];
    float swap_percentage = (float) swap_used / mem_info[4] * 100;
    printf("Swap usage: %lld MB (%.2f%%)\n", swap_used/1024, swap_percentage);
}