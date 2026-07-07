#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include <string.h>


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
        new->next = NULL;

        if(strncmp(line, "cpu ", 4) == 0){ //if the line has "cpu " (with a space), it is the first line, which contains the total CPU usage.
            if (sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu", &new->user, &new->nice, &new->system, &new->idle, &new->iowait, &new->irq, &new->softirq) != 7) {
                fprintf(stderr, "Invalid format. \n");
            }

            else sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu", &new->user, &new->nice, &new->system, &new->idle, &new->iowait, &new->irq, &new->softirq);

        }
        else {
             //if the line has "cpu" followed by a number, it is a specific CPU core.
            if (sscanf(line, "%*s %llu %llu %llu %llu %llu %llu %llu", &new->user, &new->nice, &new->system, &new->idle, &new->iowait, &new->irq, &new->softirq) != 7){
                fprintf(stderr, "Invalid format. \n");
            }

            else sscanf(line, "%*s %llu %llu %llu %llu %llu %llu %llu", &new->user, &new->nice, &new->system, &new->idle, &new->iowait, &new->irq, &new->softirq);
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


void calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2, float *results) {

    unsigned long long delta_idle = 0, idle1 = 0, idle2 = 0;
    unsigned long long delta_total = 0, total1 = 0, total2 = 0;
    int count = 0;

    CPUStats *aux1 = stats1, *aux2 = stats2;
    
    while(aux1 != NULL && aux2 != NULL){
    
        total1 = aux1->user + aux1->nice + aux1->system + aux1->idle + aux1->iowait + aux1->irq + aux1->softirq;
        total2 = aux2->user + aux2->nice + aux2->system + aux2->idle + aux2->iowait + aux2->irq + aux2->softirq;

        idle1 = aux1->idle + aux1->iowait;
        idle2 = aux2->idle + aux2->iowait;
        

        //calculate the difference between the two samples
        delta_idle = idle2 - idle1; 
        delta_total = total2 - total1;
        
        results[count++] = ((float) (delta_total - delta_idle) / delta_total) * 100;
        
        aux1 = aux1->next;
        aux2 = aux2->next;
    }
}

void print_cpu_usage(float *results, int total){
    int index = 0;
    while (index < total){

        if (index == 0){
            printf("CPU Total %.2f%%\n", results[index]);
            index++;
        } 
        
        else {
            printf("CPU %d (%.2f%%) ", index, results[index]);
            if (index % 3 == 0) printf("\n");
            index++;
        }
        
    }

    printf("\n");

}

void print_cpu_stats_debug(CPUStats *stats){
    CPUStats *aux = stats;
    while (aux != NULL){
        printf("%llu %llu %llu %llu %llu %llu %llu\n", aux->user, aux->nice, aux->system, aux->idle, aux->iowait, aux->irq, aux->softirq);
        aux = aux->next;
    }
}

void free_cpu_stats_list(CPUStats *stats){
    CPUStats *aux = stats;
    while (aux != NULL){
        CPUStats *temp = aux;
        aux = aux->next;
        free(temp);
    }
}