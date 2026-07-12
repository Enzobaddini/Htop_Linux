#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

Hash* create_hash_map(int size){
    Hash* hash = (Hash*)malloc(sizeof(Hash));
    if (hash == NULL) {
        perror("Error alocating hash struct");
        exit(1);
    }
    hash->size = size;
    hash->buckets = (Process*)malloc(sizeof(Process)*hash->size);
    if (hash->buckets == NULL) {
        perror("Error alocating hash buckets struct");
        free(hash);
        exit(1);
    }
    for (int i = 0; i <hash->size; i++) {
        hash->buckets[i] = NULL;
    }

    return hash;
}



Hash* insert_process(Hash* hash, int pid){

    Process* data = parse_process(pid);

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