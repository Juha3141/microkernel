#include <efi.h>
#include <efilib.h>
#include <efishellintf.h>

#include <loader/loader_argument.hpp>
#include <kernel/configurations.hpp>
#include <arch/configurations.hpp>

#define ALIGN_SIZE(size , alignment) (size+(alignment-(size%alignment)))

wchar_t *get_efi_memory_type_str(EFI_MEMORY_TYPE type);
EFI_MEMORY_DESCRIPTOR *find_available_memory_entry(EFI_MEMORY_DESCRIPTOR *memmap_entry , UINT64 memmap_size , 
    UINT64 memmap_descriptor_size , UINT64 minimum_address , UINT64 minimum_size , EFI_MEMORY_DESCRIPTOR *exclude_entry);
EFI_FILE_HANDLE get_volume(EFI_HANDLE image);
unsigned int convert_efi_mem_type_to_kernel(EFI_MEMORY_TYPE efi_mem_type);

extern void jump_to_kernel(struct LoaderArgument *loader_argument , UINT64 kernel_location , UINT64 kernel_stack);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle , EFI_SYSTEM_TABLE *system_table) {
    EFI_STATUS status;
    InitializeLib(image_handle , system_table);
    Print(L"Hello, world!\n");

    // fetch memory map
    UINTN memmap_size = 16384 , memmap_key = 0 , memmap_descriptor_size = 0;
    UINT32 memmap_descriptor_version;
    EFI_MEMORY_DESCRIPTOR *memory_descriptor = AllocatePool(memmap_size);
    
    status = uefi_call_wrapper(BS->GetMemoryMap , 5 , &memmap_size , memory_descriptor , &memmap_key , &memmap_descriptor_size , &memmap_descriptor_version);
    if(status == EFI_BUFFER_TOO_SMALL) {
        Print(L"Unable to get memory map! EFI_BUFFER_TOO_SMALL\n");
        while(1) {
            ;
        }
    }
    else if(status == EFI_INVALID_PARAMETER) {
        Print(L"Unable to get memory map! EFI_INVALID_PARAMETER\n");
        while(1) {
            ;
        }
    }
    Print(L"number of entries : %d\n" , memmap_size/memmap_descriptor_size);
    Print(L"memory_descriptor : 0x%X\n" , memory_descriptor);
    Print(L"memmap size : %d\n" , memmap_size);
    Print(L"memmap descriptor size : %d\n" , memmap_descriptor_size);
    
    UINT64 kernel_memmap_size = ALIGN_SIZE((memmap_size/memmap_descriptor_size)*sizeof(struct LoaderMemoryMap) , 4096);
    struct LoaderMemoryMap *kernel_memmap = AllocatePool(kernel_memmap_size);
    ZeroMem(kernel_memmap , kernel_memmap_size);
    EFI_MEMORY_DESCRIPTOR *memmap_entry = memory_descriptor;
    UINT64 total_memory_size = 0;
    for(int i = 0; i < memmap_size/memmap_descriptor_size; i++) {
        // copy to the memory map for kernel
        kernel_memmap[i].addr_high = (memmap_entry->PhysicalStart >> 32);
        kernel_memmap[i].addr_low  = memmap_entry->PhysicalStart & 0xffffffff;
        kernel_memmap[i].length_high = (((UINT64)(memmap_entry->NumberOfPages*4096)) >> 32);
        kernel_memmap[i].length_low  = ((UINT64)(memmap_entry->NumberOfPages*4096)) & 0xffffffff;
        kernel_memmap[i].type = convert_efi_mem_type_to_kernel(memmap_entry->Type);
        total_memory_size += (memmap_entry->NumberOfPages*4096);
        
        if(memmap_entry->Type == EfiBootServicesCode||memmap_entry->Type == EfiBootServicesData) {
            memmap_entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8*)memmap_entry+memmap_descriptor_size);
            continue;
        }
        Print(L"phys: 0x%llX ~ 0x%llX, pages: %d(%dKiB), type: %s\n" , 
            memmap_entry->PhysicalStart , 
            memmap_entry->PhysicalStart+(memmap_entry->NumberOfPages*4096) , 
            memmap_entry->NumberOfPages , 
            memmap_entry->NumberOfPages*4096/1024 , 
            get_efi_memory_type_str(memmap_entry->Type));
        memmap_entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8*)memmap_entry+memmap_descriptor_size);
    }
    Print(L"Total usuable amount of memory : %dMiB\n" , total_memory_size/1024/1024);
    EFI_FILE_HANDLE kernel_file_handle , ramdisk_file_handle;
    EFI_FILE_INFO *kernel_file_info , *ramdisk_file_info;
    EFI_FILE_HANDLE volume;
    volume = get_volume(image_handle);
    Print(L"volume = 0x%llX\n" , volume);
    status = uefi_call_wrapper(volume->Open , 5 , volume , &kernel_file_handle , L"Kernel.krn" , EFI_FILE_MODE_READ , EFI_FILE_READ_ONLY|EFI_FILE_HIDDEN|EFI_FILE_SYSTEM);
    if(EFI_ERROR(status)) {
        Print(L"Kernel.krn not found!\n");
        while(1) {
            ;
        }
    }
    status = uefi_call_wrapper(volume->Open , 5 , volume , &ramdisk_file_handle , L"ramdisk.img" , EFI_FILE_MODE_READ , EFI_FILE_READ_ONLY|EFI_FILE_HIDDEN|EFI_FILE_SYSTEM);
    if(EFI_ERROR(status)) {
        Print(L"RAM disk image not found!\n");
        while(1) {
            ;
        }
    }
    kernel_file_info = LibFileInfo(kernel_file_handle);
    ramdisk_file_info = LibFileInfo(ramdisk_file_handle);
    Print(L"kernel file size : %d\n" , kernel_file_info->FileSize);
    Print(L"RAM disk file size : %d\n" , ramdisk_file_info->FileSize);
    UINT64 ramdisk_file_size = ramdisk_file_info->FileSize;
    UINT64 kernel_file_size = kernel_file_info->FileSize;
    UINT64 kernel_misc_area_size = kernel_memmap_size+CONFIG_KERNEL_KSTRUCT_SIZE+CONFIG_KERNEL_STACK_SIZE+LOADER_ARGUMENT_LENGTH+ramdisk_file_size;

    // align kernel file size to 4096
    kernel_file_size = ALIGN_SIZE(kernel_file_size , 4096);
    Print(L"aligned loader argument size : %d\n" , LOADER_ARGUMENT_LENGTH);
    Print(L"aligned kernel file size     : %d\n" , kernel_file_size);

    UINT64 kernel_location = CONFIG_KERNEL_ADDRESS , 
           kernel_stack_location = 0x00 , 
           kstruct_mem_location = 0x00 , 
           loader_argument_location = 0x00 , 
           kernel_memmap_location = 0x00 ,  
           kernel_ramdisk_location = 0x00;

    // Find the location for kernel in the memory map
    EFI_MEMORY_DESCRIPTOR *kernel_stack_memory_chunk;
    EFI_MEMORY_DESCRIPTOR *kernel_memory_chunk = 
        find_available_memory_entry(memory_descriptor , memmap_size , memmap_descriptor_size , kernel_location , kernel_file_size , 0x00);
    for(UINT64 k_addr = kernel_location; k_addr < kernel_location+kernel_file_size; k_addr += 8) {
        ((UINT64 *)k_addr)[0] = 0x00;
    }
    // if the selected chunk is big enough to also encompass the kernel's miscellaneous area, locate the miscellaneous area right next to the kernel
    if((kernel_memory_chunk->NumberOfPages*4096) >= kernel_file_size+kernel_misc_area_size) {
        kernel_memmap_location   = kernel_location+kernel_file_size;
        loader_argument_location = kernel_memmap_location+kernel_memmap_size;
        kstruct_mem_location     = loader_argument_location+LOADER_ARGUMENT_LENGTH;
        kernel_stack_location    = kstruct_mem_location+CONFIG_KERNEL_KSTRUCT_SIZE;
        kernel_ramdisk_location  = kernel_stack_location+CONFIG_KERNEL_STACK_SIZE;
        Print(L"Miscellaneous area next to the kernel\n");
    }
    else {
        kernel_stack_memory_chunk = find_available_memory_entry(memory_descriptor , memmap_size , memmap_descriptor_size , kernel_memory_chunk->PhysicalStart+kernel_file_size , kernel_misc_area_size , kernel_memory_chunk);
        kernel_memmap_location   = kernel_stack_memory_chunk->PhysicalStart;
        loader_argument_location = kernel_memmap_location+kernel_memmap_size;
        kstruct_mem_location     = loader_argument_location+LOADER_ARGUMENT_LENGTH;
        kernel_stack_location    = kstruct_mem_location+CONFIG_KERNEL_KSTRUCT_SIZE;
        kernel_ramdisk_location  = kernel_stack_location+CONFIG_KERNEL_STACK_SIZE;
    }
    
    memcpy(((void *)kernel_memmap_location) , kernel_memmap , kernel_memmap_size);
    struct LoaderArgument *loader_argument = (struct LoaderArgument *)loader_argument_location;
    ZeroMem(loader_argument , sizeof(struct LoaderArgument));
    loader_argument->signature = LOADER_ARGUMENT_SIGNATURE;
    loader_argument->kernel_physical_location = kernel_location;
    loader_argument->kernel_size              = kernel_file_size;
    loader_argument->kernel_stack_location    = kernel_stack_location;
    loader_argument->kernel_stack_size        = CONFIG_KERNEL_STACK_SIZE;
    loader_argument->loader_argument_location = loader_argument;
    loader_argument->loader_argument_size     = LOADER_ARGUMENT_LENGTH;
    loader_argument->kstruct_mem_location     = kstruct_mem_location;
    loader_argument->kstruct_mem_size         = CONFIG_KERNEL_KSTRUCT_SIZE;
    loader_argument->ramdisk_location         = kernel_ramdisk_location;
    loader_argument->ramdisk_size             = ramdisk_file_size;
    loader_argument->is_ramdisk_available     = 1;
    loader_argument->video_mode = LOADER_ARGUMENT_VIDEOMODE_GRAPHIC;

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    if(EFI_ERROR(uefi_call_wrapper(BS->LocateProtocol , 3 , &gop_guid , NULL , (void**)&gop))) {
        Print(L"Unable to locate Graphics Output Protocol!\n");
        while(1) {
            ;
        }
    }

    Print(L"Resolution : %ux%u\n" , gop->Mode->Info->HorizontalResolution , gop->Mode->Info->VerticalResolution);
    Print(L"Pixel per scan line : %u\n" , gop->Mode->Info->PixelsPerScanLine);
    Print(L"Frame Buffer : 0x%X ~ 0x%X (Size : %d)\n" , gop->Mode->FrameBufferBase , gop->Mode->FrameBufferBase+gop->Mode->FrameBufferSize , gop->Mode->FrameBufferSize);
    
    loader_argument->dbg_graphic_framebuffer_start  = gop->Mode->FrameBufferBase;
    loader_argument->dbg_graphic_framebuffer_end    = gop->Mode->FrameBufferBase+gop->Mode->FrameBufferSize;
    loader_argument->dbg_graphic_framebuffer_width  = gop->Mode->Info->HorizontalResolution;
    loader_argument->dbg_graphic_framebuffer_height = gop->Mode->Info->VerticalResolution;
    loader_argument->dbg_graphic_framebuffer_depth  = 32;

    loader_argument->memmap_location = kernel_memmap_location;
    loader_argument->memmap_count    = memmap_size/memmap_descriptor_size;

    StrCpy(loader_argument->debug_interface_identifier , "comport1");
    
    UINT64 file_size = kernel_file_info->FileSize;
    Print(L"Kernel file size : %d\n" , file_size);
    uefi_call_wrapper(kernel_file_handle->Read , 3 , kernel_file_handle , &file_size , (void *)loader_argument->kernel_physical_location);
    file_size = ramdisk_file_info->FileSize;
    uefi_call_wrapper(ramdisk_file_handle->Read , 3 , ramdisk_file_handle , &file_size , (void *)loader_argument->ramdisk_location);
    
    Print(L"loader argument location = 0x%X\n" , loader_argument_location);
    Print(L"kernel_memmap_location   = 0x%X~0x%X\n" , kernel_memmap_location , kernel_memmap_location+kernel_memmap_size);
    Print(L"loader_argument_location = 0x%X~0x%X\n" , loader_argument_location , loader_argument_location+LOADER_ARGUMENT_LENGTH);
    Print(L"kstruct_mem_location     = 0x%X~0x%X\n" , kstruct_mem_location , kstruct_mem_location+CONFIG_KERNEL_KSTRUCT_SIZE);
    Print(L"kernel_stack_location    = 0x%X~0x%X\n" , kernel_stack_location , kernel_stack_location+CONFIG_KERNEL_STACK_SIZE);
    Print(L"kernel_ramdisk_location  = 0x%X~0x%X\n" , kernel_ramdisk_location , kernel_ramdisk_location+ramdisk_file_size);
    memmap_size = 16384;
    Print(L"Jumping to location of kernel : 0x%X...\n" , loader_argument->kernel_physical_location);
    status = uefi_call_wrapper(BS->GetMemoryMap , 5 , &memmap_size , memory_descriptor , &memmap_key , &memmap_descriptor_size , &memmap_descriptor_version);
    if(EFI_ERROR(status)) {
        Print(L"Unable to fetch the memory map key! status = 0x%X\n" , status);
        while(1) {
            ;
        }
    }
    status = uefi_call_wrapper(BS->ExitBootServices , 2 , image_handle , memmap_key);
    if(EFI_ERROR(status)) {
        Print(L"Unable to exit boot services! status = 0x%X\n" , status);
        while(1) {
            ;
        }
    }
    
    for(UINT64 s_addr = kernel_stack_location; s_addr < kernel_stack_location+CONFIG_KERNEL_STACK_SIZE; s_addr += 8) {
        ((UINT64 *)s_addr)[0] = 0x00;
    }
    jump_to_kernel(loader_argument , loader_argument->kernel_physical_location , loader_argument->kernel_stack_location+loader_argument->kernel_stack_size);
    
    while(1) {
        ;
    }    
    return EFI_SUCCESS;
}

unsigned int convert_efi_mem_type_to_kernel(EFI_MEMORY_TYPE type) {
    switch(type) {
        case EfiReservedMemoryType:      return MEMORYMAP_RESERVED;
        case EfiLoaderCode:              return MEMORYMAP_EFI_LOADER;
        case EfiLoaderData:              return MEMORYMAP_EFI_LOADER;
        case EfiBootServicesCode:        return MEMORYMAP_EFI_BOOT_SERVICE;
        case EfiBootServicesData:        return MEMORYMAP_EFI_BOOT_SERVICE;
        case EfiRuntimeServicesCode:     return MEMORYMAP_EFI_RUNTIME;
        case EfiRuntimeServicesData:     return MEMORYMAP_EFI_RUNTIME;
        case EfiConventionalMemory:      return MEMORYMAP_USABLE;
        case EfiUnusableMemory:          return MEMORYMAP_UNUSABLE;
        case EfiACPIReclaimMemory:       return MEMORYMAP_ACPI_RECLAIM;
        case EfiACPIMemoryNVS:           return MEMORYMAP_ACPI_NVS;
        case EfiMemoryMappedIO:          return MEMORYMAP_MISCELLANEOUS;
        case EfiMemoryMappedIOPortSpace: return MEMORYMAP_MISCELLANEOUS;
        case EfiPalCode:                 return MEMORYMAP_MISCELLANEOUS;
        case EfiPersistentMemory:        return MEMORYMAP_MISCELLANEOUS;
        case EfiUnacceptedMemoryType:    return MEMORYMAP_UNUSABLE;
        case EfiMaxMemoryType:           return MEMORYMAP_MISCELLANEOUS;
    };
    return MEMORYMAP_MISCELLANEOUS;
}

wchar_t *get_efi_memory_type_str(EFI_MEMORY_TYPE type) {
    switch(type) {
        case EfiReservedMemoryType:      return L"EfiReservedMemoryType";
        case EfiLoaderCode:              return L"EfiLoaderCode";
        case EfiLoaderData:              return L"EfiLoaderData";
        case EfiBootServicesCode:        return L"EfiBootServicesCode";
        case EfiBootServicesData:        return L"EfiBootServicesData";
        case EfiRuntimeServicesCode:     return L"EfiRuntimeServicesCode";
        case EfiRuntimeServicesData:     return L"EfiRuntimeServicesData";
        case EfiConventionalMemory:      return L"EfiConventionalMemory";
        case EfiUnusableMemory:          return L"EfiUnusableMemory";
        case EfiACPIReclaimMemory:       return L"EfiACPIReclaimMemory";
        case EfiACPIMemoryNVS:           return L"EfiACPIMemoryNVS";
        case EfiMemoryMappedIO:          return L"EfiMemoryMappedIO";
        case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
        case EfiPalCode:                 return L"EfiPalCode";
        case EfiPersistentMemory:        return L"EfiPersistentMemory";
        case EfiUnacceptedMemoryType:    return L"EfiUnacceptedMemoryType";
        case EfiMaxMemoryType:           return L"EfiMaxMemoryType";
    };
    return "Unknown";
}

EFI_FILE_HANDLE get_volume(EFI_HANDLE image) {
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *io_volume;
    EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE volume;
    EFI_STATUS status = uefi_call_wrapper(BS->HandleProtocol , 3 , image , &lip_guid , (void **)&loaded_image);
    Print(L"status = %d\n" , status);
    Print(L"loaded_image = 0x%llX\n" , loaded_image);
    Print(L"loaded_image->DeviceHandle = 0x%llX\n" , loaded_image->DeviceHandle);

    return LibOpenRoot(((UINT64)loaded_image->DeviceHandle) >> 32);
}

EFI_MEMORY_DESCRIPTOR *find_available_memory_entry(EFI_MEMORY_DESCRIPTOR *memmap_entry , UINT64 memmap_size , 
    UINT64 memmap_descriptor_size , UINT64 minimum_address , UINT64 minimum_size , EFI_MEMORY_DESCRIPTOR *exclude_entry) {
    for(int i = 0; i < memmap_size/memmap_descriptor_size; i++) {
        if(memmap_entry->Type == EfiReservedMemoryType||memmap_entry->Type == EfiBootServicesCode||memmap_entry->Type == EfiBootServicesData) {
            goto CONTINUE;
        }
        if(memmap_entry == exclude_entry) goto CONTINUE;
        
        UINT64 entry_size = memmap_entry->NumberOfPages*4096;
        UINT64 entry_start = memmap_entry->PhysicalStart , entry_end = memmap_entry->PhysicalStart+entry_size;
        if(entry_end <= minimum_address) goto CONTINUE;
        if(entry_size >= minimum_size) {
            return memmap_entry;
        }

CONTINUE:
        memmap_entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8*)memmap_entry+memmap_descriptor_size);
    }
    return 0x00;
}