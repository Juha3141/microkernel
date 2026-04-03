#include <iostream>
using namespace std;

#define CONFIG_PAGE_SIZE 4096ULL                // 4KB
#define CONFIG_LARGE_PAGE_SIZE 2097152ULL       // 2MB
#define CONFIG_ENORMOUS_PAGE_SIZE 1073741824ULL // 1GB

#define LARGE_PS_THRESHOLD ((CONFIG_LARGE_PAGE_SIZE)*4)
#define ENORMOUS_PS_THRESHOLD ((CONFIG_ENORMOUS_PAGE_SIZE)*4)

#define align_round_up(val , step)   ((step) == 0 ? (val) : ((val)%(step) == 0 ? (val) : ((val)+(step - (val)%(step)))))
#define align_round_down(val , step) ((step) == 0 ? (val) : ((val)%(step) == 0 ? (val) : (val)-((val)%(step))))

typedef unsigned long long max_t;

void map_pages(max_t linear_addr , max_t page_size , max_t page_count , max_t physical_address) {
    printf(" ----- map_pages() : phys=0x%llx~0x%llx la=0x%llx~0x%llx  ps=0x%llx count=%llu\n" , 
        physical_address , physical_address+(page_size*page_count) , linear_addr , linear_addr+(page_size*page_count) , page_size , page_count);
}

max_t figure_out_page_mapping_alignment(max_t phys_addr_start , max_t phys_addr_end , max_t linear_addr_start) {
    // default will be levels_of_page_size[0]
   constexpr max_t levels_of_page_size[] = {
        CONFIG_PAGE_SIZE , 
        CONFIG_LARGE_PAGE_SIZE , 
        CONFIG_ENORMOUS_PAGE_SIZE , 
    };
    constexpr max_t levels_of_ps_threshold[] = {
        0 , 
        LARGE_PS_THRESHOLD , 
        ENORMOUS_PS_THRESHOLD , 
    };
    constexpr int lps_sz  = sizeof(levels_of_page_size)/sizeof(levels_of_page_size[0]);
    constexpr int lpst_sz = sizeof(levels_of_ps_threshold)/sizeof(levels_of_ps_threshold[0]);

    max_t required_map_size = phys_addr_end - phys_addr_start;
    int page_size_idx = -1;
    /** 1. Determine page size */
    for(int i = lpst_sz-1; i >= 0; i--) {
        // If it exceeds 
        if(required_map_size > levels_of_ps_threshold[i]) {
            page_size_idx = i;
            break;
        }
    }
    printf("page size : 0x%llx(%dth from the hierarchy)\n" , levels_of_page_size[page_size_idx] , page_size_idx);
    
    max_t phys_addr_offset   = 0;
    max_t linear_addr_offset = 0;
    max_t larger_page_size = levels_of_page_size[page_size_idx];
    
    while(((phys_addr_start+phys_addr_offset)%larger_page_size != 0)
       || ((linear_addr_start+linear_addr_offset)%larger_page_size != 0)) {
        max_t phys_addr = phys_addr_start+phys_addr_offset;
        max_t linear_addr = linear_addr_start+linear_addr_offset;
        if(phys_addr%larger_page_size != 0) phys_addr_offset += smaller_page_size;
        if(linear_addr%larger_page_size != 0) linear_addr_offset += smaller_page_size;
    }
    map_pages(linear_addr_start , smaller_page_size , min(phys_addr_offset , linear_addr_offset)/smaller_page_size , phys_addr_start);

    if(smaller_page_size == larger_page_size) {
        return;
    }
    
    linear_addr_start = linear_addr_start + min(phys_addr_offset , linear_addr_offset);
    phys_addr_start   = phys_addr_start   + min(phys_addr_offset , linear_addr_offset);

    printf("phys_addr_offset   : 0x%-12llx(%lldkB)  phys_addr_aligned   : 0x%-12llx\n" , phys_addr_offset , phys_addr_offset/1024 , phys_addr_start+phys_addr_offset);
    printf("linear_addr_offset : 0x%-12llx(%lldkB)  linear_addr_aligned : 0x%-12llx\n" , linear_addr_offset , linear_addr_offset/1024 , linear_addr_start+linear_addr_offset);
    printf("wasted memory      : %lldkB\n" , (max(phys_addr_offset , linear_addr_offset)-min(phys_addr_offset , linear_addr_offset))/1024);

    printf("smallest possible page size : 0x%llx(%dth from the hierarchy)\n" , levels_of_page_size[smallest_possible_ps_idx] , smallest_possible_ps_idx);

    // linear_addr_start + 
    // map_pages(linear_addr_start , larger_page_size ,  , phys_addr_start);

    // figure_out_page_mapping_alignment(phys_addr_start , phys_addr_end , linear_addr_start);
}

int main(void) {
    int N;
    scanf("%d" , &N);
    for(int i = 0; i < N; i++) {
        max_t addr_start, addr_end;
        max_t linear_addr_start;
        
        scanf("%llx %llx %llx" , &addr_start , &addr_end , &linear_addr_start);
        printf("Mapping physical address 0x%-10llx ~ 0x%-10llx --> 0x%10llx ~ 0x%10llx\n" , addr_start , addr_end , linear_addr_start , linear_addr_start + (addr_end-addr_start));
        figure_out_page_mapping_alignment(addr_start , addr_end , linear_addr_start);
    }

}