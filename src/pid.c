#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include "pid.h"

int is_pid(const char* name){
    if (name[0] == '\0') return 0;
    for (int i = 0; name[i] != '\0'; i++){
        if (!isdigit(name[i])) return 0;
    }
    return 1;
}

PidList* parse_pid(const char* file) {
    int capacity = 64;
    
    PidList* values = (PidList*)malloc(sizeof(PidList));
    if (values == NULL){
        perror("Error allocating PidList");
        exit(1);
    }

    values->pid = (int*)malloc(sizeof(int)*capacity);
    if (values->pid == NULL) {
        free(values);
        perror("Error allocating pid array");
        exit(1);
    }
    
    values->count = 0;
    

    DIR* dir = opendir(file);
    if(dir == NULL){
        free(values->pid);
        free(values);
        perror("Error opening directory");
        exit(1);
    }

    struct dirent* entry;
    int int_pid;

    while((entry = readdir(dir)) != NULL){

        if (is_pid(entry->d_name)){
            int_pid = atoi(entry->d_name);
            values->pid[values->count++] = int_pid;
            if (values->count == capacity){
                capacity *= 2;
                int* temp = (int*)realloc(values->pid, capacity * sizeof(int));
                if(temp == NULL){
                    closedir(dir);
                    free(values->pid);
                    free(values);
                    perror("Error reallocing the array");
                    exit(1);
                }
                values->pid = temp;
            }
            
        }
    }

    closedir(dir);
    
    return values;
}


void free_pid_list(PidList* values) {
    if (values != NULL) {
        if (values->pid != NULL){
            free(values->pid);
        }
        free(values);
    }
}