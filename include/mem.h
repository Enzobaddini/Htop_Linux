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

void print_main_memory_usage(long long* mem_info);

void print_swap_and_cache_info(long long* mem_info);
