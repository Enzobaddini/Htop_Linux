#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include <string.h>

CPUStats* inicialize_cpu_stats() {
    
    return NULL;
}

CPUStats* atribuite_cpu_stats(CPUStats *stats, char *file, char **name_list) {
    
    FILE* fp = fopen(file, "r");
    if (fp == NULL){
        fprintf(stderr, "Error opening /proc/stat\n");
        exit(1);
    }
    char buffer[100], *text;
    char *temp;
    int start = 0;

    fscanf(fp, "%[^\n]", buffer); // discards the "cpu" label token

    temp = strtok(buffer, " ");
    
    //gets the first 7 fields of the /proc/stat file
    while(start < 7) { 
        temp = strtok(NULL, " ");
        unsigned long long info = strtoull(temp, &text, 10);
        stats = create_node(stats, info, name_list[start]);
        start++;
    }
    
    print_stats(stats);
    fclose(fp);

    return stats;
}

CPUStats* create_node(CPUStats *stats, unsigned long long info, char *field_name) {
    CPUStats *aux = stats;
    
    CPUStats *new = malloc(sizeof *new);
    if(new == NULL){
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }

    // fill the new node with the information from /proc/stat
    new->info = info; 
    strcpy(new->name, field_name);
    new->next = NULL;
    
    if (stats == NULL){
        return new;
    }
    
    while(aux->next != NULL){
        aux = aux->next;
    }

    aux->next = new;

    return stats;
}

float calculate_cpu_usage(CPUStats *stats1, CPUStats *stats2) {

    unsigned long long delta_idle = 0, idle1 = 0, idle2 = 0;
    unsigned long long delta_total = 0, total1 = 0, total2 = 0;
    
    CPUStats *aux1 = stats1, *aux2 = stats2;
    
    
    while (aux1 != NULL && aux2 != NULL){
        total1 += aux1->info;
        total2 += aux2->info;
        
        if (strcmp(aux1->name, "idle") == 0 || strcmp(aux1->name, "iowait") == 0){
            idle1 += aux1->info;
            idle2 += aux2->info;
        }
        
        aux1 = aux1->next;
        aux2 = aux2->next;
    }

    //calculate the difference between the two linked lists
    delta_idle = idle2 - idle1; 
    delta_total = total2 - total1;
    
    return (float) (delta_total - delta_idle) / delta_total;
    

}

void print_stats(CPUStats *stats){
    CPUStats *aux = stats;
    while (aux != NULL){
        printf("%s: ", aux->name);
        printf("%llu ", aux->info);
        printf("\n");
        aux = aux->next;
    }
    printf("\n");
}

void free_stats(CPUStats *stats){
    CPUStats *aux = stats;
    while (aux != NULL){
        CPUStats *temp = aux;
        aux = aux->next;
        free(temp);
    }
}