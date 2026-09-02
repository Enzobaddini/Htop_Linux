#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include <string.h>
#include <ncurses.h>

CPUStats* initialize_cpu_stats() {

    return NULL;
}

CPUStats* parse_cpu_stats(const char *file, CPUStats **stats) {
    
    FILE* fp = fopen(file, "r");

    if (fp == NULL){
        fprintf(stderr, "Error opening /proc/stat\n");
        return NULL;
    }
    
    CPUStats *tail = NULL;
   
    char line[256];

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "cpu", 3) != 0){ //if the line does not start with "cpu", stop the search.
            break;
        }

        CPUStats *new = (CPUStats*) malloc(sizeof(CPUStats));
        if (new == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        int parsed;
        new->next = NULL;

        if(strncmp(line, "cpu ", 4) == 0){ //if the line has "cpu " (with a space), it is the first line, which contains the total CPU usage.
            parsed = sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu", &new->user, &new->nice, &new->system, &new->idle, &new->iowait, &new->irq, &new->softirq);
        }
        else { //if the line has "cpu" followed by a number, it is a specific CPU core.
            parsed = sscanf(line, "%*s %llu %llu %llu %llu %llu %llu %llu", &new->user, &new->nice, &new->system, &new->idle, &new->iowait, &new->irq, &new->softirq);

        }
        
        if (parsed != 7) {
            fprintf(stderr, "Invalid format.\n");
            free(new);
            continue;
        }


        if (*stats == NULL) {
            *stats = new;
            tail = new;
        } else {
            tail->next = new;
            tail = new;
        }

    }
    
    fclose(fp);

    return *stats;
}


int cpu_stats_count(CPUStats *stats) {
    int n = 0;
    for (CPUStats *aux = stats; aux != NULL; aux = aux->next)
        n++;
    return n;
}

void calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2, float *results, int nresults) {

    unsigned long long delta_idle = 0, idle1 = 0, idle2 = 0;
    unsigned long long delta_total = 0, total1 = 0, total2 = 0;
    int count = 0;

    CPUStats *aux1 = stats1, *aux2 = stats2;
    
    while(aux1 != NULL && aux2 != NULL && count < nresults){
    
        total1 = aux1->user + aux1->nice + aux1->system + aux1->idle + aux1->iowait + aux1->irq + aux1->softirq;
        total2 = aux2->user + aux2->nice + aux2->system + aux2->idle + aux2->iowait + aux2->irq + aux2->softirq;

        idle1 = aux1->idle + aux1->iowait;
        idle2 = aux2->idle + aux2->iowait;
        
        if (total2 < total1 || idle2 < idle1) {
            results[count++] = 0.0f;
            aux1 = aux1->next;
            aux2 = aux2->next;
            continue;
        }

        //calculate the difference between the two samples
        delta_idle = idle2 - idle1; 
        delta_total = total2 - total1;
        
        if (delta_idle > delta_total) {
            results[count++] = 0.0f;
            aux1 = aux1->next;
            aux2 = aux2->next;
            continue;
        }


        if (delta_total == 0) {
            results[count++] = 0.0f;
            aux1 = aux1->next;
            aux2 = aux2->next;
            continue;
        }

        results[count++] = ((float) (delta_total - delta_idle) / delta_total) * 100;
        
        aux1 = aux1->next;
        aux2 = aux2->next;
    }
}

int cpu_usage_row_count(int total) {
    if (total <= 0) return 0;
    if (total == 1) return 1;
    return (1 + ((total - 1) + 2) / 3);
}

int print_cpu_usage(float *results, int total, int start_row, int offset){
    for (int index = 0; index < total; index++){
        int virtual_row;
        int pos_x;

        if (index == 0) {
            virtual_row = start_row;
            pos_x = 0;
        } else {
            int core = index - 1;
            virtual_row = start_row + 1 + core / 3;
            pos_x = (core % 3) * 20;
        }

        int screen_row = virtual_row - offset;
        if (screen_row < 0 || screen_row >= LINES)
            continue;

        if (index == 0)
            mvprintw(screen_row, pos_x, "CPU Total %.2f%%", results[index]);
        else
            mvprintw(screen_row, pos_x, "CPU %d (%.2f%%)", index, results[index]);
    }

    return cpu_usage_row_count(total);
}

void free_cpu_stats_list(CPUStats *stats){
    CPUStats *aux = stats;
    while (aux != NULL){
        CPUStats *temp = aux;
        aux = aux->next;
        free(temp);
    }
}