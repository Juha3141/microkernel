#include <efi.h>
#include <efilib.h>

EFI_FILE_HANDLE get_volume(EFI_HANDLE image) {
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *io_volume;
    EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE volume;

    uefi_call_wrapper(BS->HandleProtocol , 3 , image , &(lip_guid) , (void **)&loaded_image);
    uefi_call_wrapper(BS->HandleProtocol , 3 , loaded_image->DeviceHandle , &fs_guid , (void *)&io_volume);

    uefi_call_wrapper(io_volume->OpenVolume , 2 , io_volume , &volume);
    return volume;
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

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle , EFI_SYSTEM_TABLE *system_table) {
    EFI_STATUS status;
    InitializeLib(image_handle , system_table);
    Print(L"Hello, world!\n");

    // fetch memory map
    UINTN memmap_size = 16384 , memmap_key = 0 , memmap_descriptor_size = 0;
    UINT32 memmap_descriptor_version;
    EFI_MEMORY_DESCRIPTOR *memory_descriptor = 0x100000;
    Print(L"memory_descriptor = 0x%X\n" , memory_descriptor);
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
    
    EFI_MEMORY_DESCRIPTOR *memmap_entry = memory_descriptor;
    UINT64 total_memory_size = 0;
    for(int i = 0; i < memmap_size/memmap_descriptor_size; i++) {
        if(memmap_entry->Type == EfiReservedMemoryType) {
            memmap_entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8*)memmap_entry+memmap_descriptor_size);
            continue;
        }
        total_memory_size += (memmap_entry->NumberOfPages*4096);
        if(memmap_entry->Type == EfiBootServicesCode||memmap_entry->Type == EfiBootServicesData) {
            memmap_entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8*)memmap_entry+memmap_descriptor_size);
            continue;
        }
        Print(L"phys: 0x%llX ~ 0x%llX, pages: %d(%dkiB), type: %s\n" , 
            memmap_entry->PhysicalStart , 
            memmap_entry->PhysicalStart+(memmap_entry->NumberOfPages*4096) , 
            memmap_entry->NumberOfPages , 
            memmap_entry->NumberOfPages*4096/1024 , 
            get_efi_memory_type_str(memmap_entry->Type));
        memmap_entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8*)memmap_entry+memmap_descriptor_size);
    }
    Print(L"Total amount of memory : %dMB\n" , total_memory_size/1024/1024);


    EFI_FILE_HANDLE volume = get_volume(image_handle);
    EFI_FILE_HANDLE kernel_file_handle;
    EFI_FILE_INFO *kernel_file_info;
    uefi_call_wrapper(volume->Open , 5 , volume , &kernel_file_handle , L"Kernel.krn" , EFI_FILE_MODE_READ , EFI_FILE_READ_ONLY|EFI_FILE_HIDDEN|EFI_FILE_SYSTEM);
    kernel_file_info = LibFileInfo(kernel_file_handle);
    Print(L"kernel file size : %d\n" , kernel_file_info->FileSize);


    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    if(EFI_ERROR(uefi_call_wrapper(BS->LocateProtocol , 3 , &gop_guid , NULL , (void**)&gop))) {
        Print(L"Unable to locate Graphics Output Protocol!\n");
        while(1) {
            ;
        }
    }

    Print(L"Resolution : %ux%u\n" , gop->Mode->Info->HorizontalResolution , gop->Mode->Info->VerticalResolution);
    Print(L"Pixel format : %u\n" , gop->Mode->Info->PixelFormat);
    Print(L"Pixel per scan line : %u\n" , gop->Mode->Info->PixelsPerScanLine);
    Print(L"Frame Buffer : 0x%X ~ 0x%X (Size : %d)\n" , gop->Mode->FrameBufferBase , gop->Mode->FrameBufferBase+gop->Mode->FrameBufferSize , gop->Mode->FrameBufferSize);
    
    UINT8 *frame_buffer = (UINT8 *)gop->Mode->FrameBufferBase;
    for(int i = 0; i < gop->Mode->Info->PixelsPerScanLine*4; i += 4) {
        frame_buffer[i]   = 0xFF; // B
        frame_buffer[i+1] = 0xD8; // G
        frame_buffer[i+2] = 0x00; // R
        frame_buffer[i+4] = 0x00; // Resv
    }

    while(1) {
        ;
    }
    return EFI_SUCCESS;
}