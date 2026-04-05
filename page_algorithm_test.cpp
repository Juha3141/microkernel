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
    if(phys_addr_start == phys_addr_end) return 0;
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
    max_t page_size = levels_of_page_size[page_size_idx];

    max_t addr_size = phys_addr_end-phys_addr_start;
    
    max_t aligned_phys_addr   = align_round_up(phys_addr_start , page_size);
    max_t alignment_padding_phys = aligned_phys_addr-phys_addr_start;
    max_t aligned_linear_addr = align_round_up(linear_addr_start , page_size);
    max_t alignment_padding_linear = aligned_linear_addr-linear_addr_start;

    max_t aligned_phys_end = align_round_down(phys_addr_end , page_size);
    max_t alignment_padding_phys_end = phys_addr_end-aligned_phys_end;

    max_t aligned_linear_end = align_round_down(linear_addr_start+addr_size , page_size);
    max_t alignment_padding_linear_end = linear_addr_start+addr_size-aligned_linear_end;

    if(alignment_padding_phys < alignment_padding_linear) {
        alignment_padding_phys += page_size;
        aligned_phys_addr      += page_size;
    }
    printf(" =========== lower unaligned area mapping ============ \n");
    figure_out_page_mapping_alignment(phys_addr_start , phys_addr_start+alignment_padding_linear , linear_addr_start);
    printf(" ========== lower unaligned area mapping end ========= \n");

    printf("aligned_phys_addr   : 0x%-13llx ~ 0x%-13llx (sz=0x%llx)\n" , aligned_phys_addr , aligned_phys_end , aligned_phys_end-aligned_phys_addr);
    printf("padding             : 0x%llx\n" , alignment_padding_phys);
    printf("aligned_linear_addr : 0x%-13llx ~ 0x%-13llx (sz=0x%llx)\n" , aligned_linear_addr , aligned_linear_end , aligned_linear_end-aligned_linear_addr);
    printf("padding             : 0x%llx\n" , alignment_padding_linear);

    max_t page_count = (aligned_linear_end-aligned_linear_addr)/page_size;
    map_pages(linear_addr_start+alignment_padding_linear , page_size , page_count , phys_addr_start+alignment_padding_phys);


    printf(" =========== upper unaligned area mapping ============ \n");
    figure_out_page_mapping_alignment(aligned_phys_end , phys_addr_end , aligned_linear_end);
    printf(" ========== upper unaligned area mapping end ========= \n");
    return 0;
}

int main(void) {
    figure_out_page_mapping_alignment(0x1c02000 , 0x39edf000 , 0xffaa0721000);

}