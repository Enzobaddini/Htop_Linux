#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "pid.h"
#include "process.h"
#include <ncurses.h>
#include <errno.h>

Hash* create_hash_map(int size){
    Hash* hash = (Hash*)malloc(sizeof(Hash));
    if (hash == NULL) {
        perror("Error alocating hash struct");
        exit(1);
    }
    hash->size = size;
    hash->buckets = (Process**)malloc(sizeof(Process*)*hash->size);
    if (hash->buckets == NULL) {
        perror("Error alocating hash buckets struct");
        free(hash);
        exit(1);
    }
    for (int i = 0; i < hash->size; i++) {
        hash->buckets[i] = NULL;
    }

    return hash;
}

Process* parse_process(int pid){
    
    char arq1[64] = "/proc/";
    snprintf(arq1, sizeof(arq1), "/proc/%d/stat", pid);

    FILE* fp = fopen(arq1, "r");
    if (fp == NULL){
        return NULL;
    }
    char line[4096];
    if(fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return NULL;
    }

    Process* data = (Process*)calloc(1, sizeof(Process));
    if (data == NULL) {
        perror("Error allocating memory for Process struct");
        fclose(fp);
        exit(1);
    }
    
    sscanf(line, "%d", &data->pid);
    char* first_paren = strchr(line, '(');
    char* last_paren = strrchr(line, ')');
    
    if (first_paren == NULL || last_paren == NULL) {
        fclose(fp);
        free(data);
        return NULL;
    }
    
    int size_name = (int)(last_paren - first_paren - 1);
    if (size_name < 0) size_name = 0;
    if (size_name > (int)sizeof(data->name) - 1)
        size_name = (int)sizeof(data->name) - 1;
    memcpy(data->name, first_paren + 1, (size_t)size_name);
    data->name[size_name] = '\0';

    char* rest = last_paren + 2;
    int field = 3;

    char*token = strtok(rest, " ");

    while(token != NULL){
        switch(field){
            case 3:
                data->state = token[0];
                break;
            case 4:
                data->ppid = atoi(token);
                break;
            case 14:
                data->user_time = atoll(token);
                break;
            case 15:
                data->kernel_time = atoll(token);
                break;
            case 20:
                data->num_threads = atoi(token);
                break;
            case 24:
                data->rss = atoll(token) * (sysconf(_SC_PAGESIZE) / 1024);
                break;
            case 39:
                data->last_cpu = atoi(token);
                break;
        }
        field++;
        token = strtok(NULL, " ");
    }

    data->stale = 0;
    data->next = NULL;
    
    fclose(fp);
    return data;
    
}


Hash* insert_process(Hash* hash, int pid){

    Process* data = parse_process(pid);

    if (data == NULL) {
        return hash;
    }

    int index = pid % hash->size;
    Process* current = hash->buckets[index];
    
    data->next = current;
    hash->buckets[index] = data;
    
    return hash;
}

Process* find_process(Hash* hash, int pid) {
    int index = pid % hash->size;

    Process *current = hash->buckets[index];

    while (current != NULL) {

        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

Hash* remove_process(Hash* hash, int pid) {
    int index = pid % hash->size;

    Process *current = hash->buckets[index];
    Process *previous = NULL;

    while (current != NULL) {

        if (current->pid == pid) {

           
            if (previous == NULL) {
                hash->buckets[index] = current->next;
            }
            
            else {
                previous->next = current->next;
            }

            free(current);
            return hash;
        }

        previous = current;
        current = current->next;
    }

    return hash;
}

Hash* update_process(Hash* hash, int pid){
    Process* existing = find_process(hash, pid);

    if (existing == NULL){
        return insert_process(hash, pid);
    }

    Process* fresh = parse_process(pid);
    if (fresh == NULL){
        return hash;
    }

    fresh->next = existing->next;
    *existing = *fresh;
    free(fresh);
    existing->stale = 0;

    return hash;
}

void mark_all_stale(Hash* hash){
    for (int i = 0; i < hash->size; i++){
        Process* data = hash->buckets[i];
        while(data != NULL){
            data->stale = 1;
            data = data->next;
        }
    }
}

Hash* check(Hash* hash){
    for (int i = 0; i < hash->size; i++){
        Process* data = hash->buckets[i];
        while(data != NULL){
            Process* next = data->next;
            if(data->stale){
                hash = remove_process(hash, data->pid);
            }
            data = next;
        }
    }
    return hash;
}

void free_hash(Hash* hash){
    for (int i = 0; i < hash->size; i++){
        Process* data = hash->buckets[i];
        while(data != NULL){
            Process* temp = data->next;
            free(data);
            data = temp;
        }
    }
    free(hash->buckets);
    free(hash);
}

int print_process_info(Hash* hash, PidList* list_pid, int start_row, int offset, int select, int ch, int* k){
    int virtual_row = start_row;
    int max_len = COLS > 1 ? COLS - 1 : 0;
    char line[512];


    for (int i = 0; i < list_pid->count; i++){
        Process* data = find_process(hash, list_pid->pid[i]);
        int screen_row = virtual_row - offset;
        if (data != NULL && screen_row >= 0 && screen_row < LINES){
            snprintf(line, sizeof(line),
                "PID: %d, Name: %s, State: %c, PPID: %d, User Time: %lld, Kernel Time: %lld, Threads: %d, RSS: %lld, Last CPU: %d",
                data->pid, data->name, data->state, data->ppid, data->user_time, data->kernel_time,
                data->num_threads, data->rss, data->last_cpu);
            if (i == select) attron(A_REVERSE);
            mvaddnstr(screen_row, 0, line, max_len);
            if (i == select) attroff(A_REVERSE);
        }
        if (i == select && ch == KEY_DC) {
            *k = (kill(list_pid->pid[i], SIGTERM) == 0) ? 0 : errno;
        }
        virtual_row++;
    }

    return virtual_row - start_row;
}


void prune_dead_pids(PidList* list, Hash* hash) {
    int write = 0;
    for (int read = 0; read < list->count; read++) {
        if (find_process(hash, list->pid[read]) != NULL) {
            list->pid[write++] = list->pid[read];
        }
    }
    list->count = write;
}
