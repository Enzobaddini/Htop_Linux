#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include "pid.h"
#include "process.h"
#include "ui.h"

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
    

    while((entry = readdir(dir)) != NULL){

        if (is_pid(entry->d_name)){
            values->pid[values->count++] = atoi(entry->d_name);
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

PidList* filter_pids(PidList* raw, Hash* hash, const char* number, const char* pid_name){
    PidList* out = malloc(sizeof(PidList));
    out->pid = malloc(sizeof(int) * 64);
    out->count = 0;
    int capacity = 64;

    for (int i = 0; i < raw->count; i++){
        int int_pid = raw->pid[i];
        if (pid_name[0] != '\0') {
            Process* p = find_process(hash, int_pid);
            if (p != NULL && strncasecmp(p->name, pid_name, strlen(pid_name)) == 0)
                out->pid[out->count++] = int_pid;
        }
        else if (strcmp(number, "0") == 0) {
            out->pid[out->count++] = int_pid;
        }
        else {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", int_pid);
            if (strncmp(number, buf, strlen(number)) == 0)
                out->pid[out->count++] = int_pid;
        }
        if (out->count == capacity){
            capacity *= 2;
            out->pid = realloc(out->pid, capacity * sizeof(int));
        }
    }
    return out;
}


void free_pid_list(PidList* values) {
    if (values != NULL) {
        if (values->pid != NULL){
            free(values->pid);
        }
        free(values);
    }
}

void debug(PidList* values){
    if (values == NULL) return;
    PidList* aux = values;
    for(int i = 0; i < aux->count; i++){
        printf("%d\n", aux->pid[i]);
    }
}

