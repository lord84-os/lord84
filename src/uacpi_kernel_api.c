#include <uacpi/uacpi.h>
#include "sys/pci.h"
#include "mm/vmm.h"
#include "mm/kmalloc.h"
#include <limine.h>
#include <stdio.h>
#include <SFB25.h>
#include <io.h>

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address){
    extern struct limine_rsdp_request rsdp_request;
    out_rsdp_address = (uacpi_phys_addr*)rsdp_request.response->address;
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle){

    if(address.segment != 0){
        klog(LOG_ERROR, __func__, "Multiple segments not implemented!");
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    l84_pci_function_return ret = check_device(address.bus, address.device);

    return ret.func_addr[address.function];
}

void uacpi_kernel_pci_device_close(uacpi_handle handle){
    return;
}

uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value){
    if(byte_width == 1){
        uint8_t read = *(uint8_t*)((uint64_t)device + offset);
        *value = read;
    }else if(byte_width == 2){
        uint16_t read = *(uint16_t*)((uint64_t)device + offset);
        *value = read;
    }else if(byte_width == 4){
        uint32_t read = *(uint32_t*)((uint64_t)device + offset);
        *value = read;
    }else{
        return UACPI_STATUS_INTERNAL_ERROR;
    }

    return UACPI_STATUS_OK;
};

uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value){
    if(byte_width == 1){
        *(uint8_t*)((uint64_t)device + offset) = value;
    }else if(byte_width == 2){
        *(uint16_t*)((uint64_t)device + offset) = value;
    }else if(byte_width == 4){
        *(uint32_t*)((uint64_t)device + offset) = value;
    }else{
        return UACPI_STATUS_INTERNAL_ERROR;
    }

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle){
    return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_io_unmap(uacpi_handle handle){
    asm("nop");
}

uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value){
    if(byte_width == 1){
        *value = inb((uint16_t)offset);
    }else if(byte_width == 2){
        *value = inw((uint16_t)offset);
    }else if(byte_width == 4){
        *value = inl((uint16_t)offset);
    }else{
        return UACPI_STATUS_INTERNAL_ERROR;
    }

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value){
    if(byte_width == 1){
        outb((uint16_t)offset, value);
    }else if(byte_width == 2){
        outw((uint16_t)offset, value);
    }else if(byte_width == 4){
        outl((uint16_t)offset, value);
    }else{
        return UACPI_STATUS_INTERNAL_ERROR;
    }

    return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len){
    kernel_map_pages((void*)addr, len, PTE_BIT_RW);
    return (void*)addr;
}

void uacpi_kernel_unmap(void *addr, uacpi_size len){
    kernel_unmap_pages(addr, len);
}

void *uacpi_kernel_alloc(uacpi_size size){
    void *ret = kmalloc(size);

    if(ret == NULL){
        klog(LOG_ERROR, __func__, "OOM");
        kkill();
    }

    return ret;

}

void uacpi_kernel_free(void *mem){

    if(mem == NULL){
        return;
    }

    kfree(mem);
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *str){
    switch(level){
        case UACPI_LOG_ERROR:
            kprintf("uacpi: error: %s\n", str);
        case UACPI_LOG_WARN:
            kprintf("uacpi: warn: %s\n", str);
        case UACPI_LOG_INFO:
            kprintf("uacpi: info: %s\n", str);
        default:
            asm("nop");

    }
}