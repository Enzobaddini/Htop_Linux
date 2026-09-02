#ifndef MEM_H
#define MEM_H

enum MemoryInfoIndex{
    MEM_TOTAL,
    MEM_AVAILABLE,
    MEM_BUFFERS,
    MEM_CACHED,
    SWAP_TOTAL,
    SWAP_FREE,
    MEM_INFO_COUNT
}; 

long long* parse_memory_info(long long *mem_info, const char *file);

int print_main_memory_usage(long long* mem_info, int start_row, int offset);

int print_swap_and_cache_info(long long* mem_info, int start_row, int offset);

#endif