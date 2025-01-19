#include <uacpi/uacpi.h>
#include "sys/pci.h"
#include <limine.h>
#include <stdio.h>
#include <lord84.h>

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

    if(ret.func_addr[address.function] == 0){
        klog(LOG_ERROR, __func__, "Multiple segments not implemented!");
        return UACPI_STATUS_INTERNAL_ERROR;
    }

    return ret.func_addr[address.function];
}

void uacpi_kernel_pci_device_close(uacpi_handle handle){
    return;
}

uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value);