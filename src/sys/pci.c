#include <lord84.h>
#include <stdio.h>
#include "acpi.h"
#include "../mm/vmm.h"
#include "pci.h"

#define PCIE_CONF_SPACE_WIDTH       4096
#define PCI_CONF_SPACE_WIDTH        256
#define PCI_BUS_MAX_DEVICES         32

#define PCI_HEADER_TYPE_GENERAL     0x0         // General device
#define PCI_HEADER_TYPE_PCI2PCI     0x1         // PCI to PCI bridge
#define PCI_HEADER_TYPE_PCI2CB      0x2         // PCI to CardBus bridge
#define PCI_HEADER_TYPE_MULTI       (1 << 7)    // Multifunction device

#define PCI_FUNCTION_MAX            8           // There are always a max of 8 functions per device


mcfg_t *mcfg;

uint64_t config_space_base_addr = 0;
uint8_t start_pci_num = 0;
uint8_t end_pci_num = 0;

extern uint64_t hhdmoffset;
extern uint64_t *kernel_page_map;

void parse_conf_space(){
    uint64_t num = (mcfg->header.length - sizeof(desc_header_t)) / sizeof(conf_space_t);

    kprintf("addr: 0x{xn}", (uint64_t)mcfg);
    kprintf("header length: {dn}", mcfg->header.length);
    kprintf("num: {dn}", num);

    for(uint64_t i = 0; i < num; i++){
        conf_space_t header = mcfg->conf_spaces[i];

        kprintf("header {dn}", i);
        kprintf(" base addr: 0x{xn}", header.base_ecm);
        kprintf(" PCI segment group: {dn}", header.pci_seg_group);
        kprintf(" start pci bus number: 0x{xn}", header.start_pci_num);
        kprintf(" end pci bus number: 0x{xn}", header.end_pci_num);

        config_space_base_addr = header.base_ecm + hhdmoffset;
        start_pci_num = header.start_pci_num;
        end_pci_num = header.end_pci_num;
    }
}

void enumerate_conf_space(){
    for(uint64_t i = 0; i < end_pci_num; i++){
        for(uint64_t j = 0; j < 32; j++){
            check_device(i, j);
        }
    }
}

/* pci_header_t *find_device(uint64_t class, int subclass)

    - Parameters:
    class - class code
    subclass - subclass. if set to -1 then subclass is ignored

    - Return:
    Returns pointer to the requested PCI header. Returns NULL if not found

    - TODO:
    Handle returning multiple functions

    */

pci_header_t *find_device(uint64_t class, int subclass){
    l84_pci_function_return pci_function_return;

    for(uint64_t i = 0; i < end_pci_num; i++){
        for(uint64_t j = 0; j < 32; j++){

            pci_function_return = check_device(i, j);

            if(pci_function_return.func_addr[0] == 0){
                continue;
            }

            /* Get the address of the first function */
            pci_header_t *header = (pci_header_t*)pci_function_return.func_addr[0];


            if(header->class_code == class){
                
                /* If no subclass is wanted, then just return the header */
                if(subclass == -1){
                    return header;
                }

                if(subclass == header->subclass){
                    return header;
                }

            }

        }
    }

    /* Found nothing, return null */
    return NULL;
}

void pci_init(){
    mcfg = (mcfg_t*)find_acpi_table("MCFG");

    if(!mcfg){
        klog(LOG_ERROR, __func__, "Failed to find MCFG table");
        kprintf("pci: device: {d} PCIe not supported! Halting");
        kkill();
    }

    mcfg = (mcfg_t*)((uint64_t)mcfg + hhdmoffset);

    parse_conf_space();

    extern uint64_t *kernel_page_map;

    /* Map the initial bus */
    for(uint64_t i = 0; i < end_pci_num; i++){
        vmm_map_page(kernel_page_map, config_space_base_addr + i * PCIE_CONF_SPACE_WIDTH, config_space_base_addr + i * PCIE_CONF_SPACE_WIDTH - hhdmoffset, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);
    }
    

    pci_header_t *header = (pci_header_t*)config_space_base_addr;
    kprintf("Vendor ID: 0x{xn}", header->vendor_id);

    enumerate_conf_space();

}

l84_pci_function_return check_device(uint64_t bus, uint64_t device){

    l84_pci_function_return ret = {0};

    pci_header_t *header = (pci_header_t*)get_header(bus, device, 0);

    vmm_map_page(kernel_page_map, (uint64_t)header, (uint64_t)header - hhdmoffset, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);

    /* If vendor id is 0xffff, that means that this device is unimplemented */
    if(header->vendor_id == 0xffff){
        return (l84_pci_function_return){ 0, {0} };
    }

    if((header->header_type & PCI_HEADER_TYPE_MULTI) != 0){
        /* Mask the 7th bit (multi-function bit) so we get the header type */
        uint64_t multi_header_type = header->header_type & 0x3f;

        ret.multi = true;

        for(uint64_t function = 0; function < PCI_FUNCTION_MAX; function++){
            uint64_t *addr = (uint64_t*)(config_space_base_addr + ((bus) << 20 | device << 15 | function << 12));
            pci_header_t *func = (pci_header_t*)((uint64_t)addr);

            vmm_map_page(kernel_page_map, (uint64_t)func, (uint64_t)func - hhdmoffset, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);

            if(func->vendor_id != 0xffff){
                kprintf("pci multi: bus: 0x{x} device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", bus, func->device_id, multi_header_type, func->class_code, func->subclass, func->vendor_id);
                serial_kprintf("pci multi: bus: 0x{x} device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", bus, func->device_id, multi_header_type, func->class_code, func->subclass, func->vendor_id);
                ret.func_addr[function] = (uint64_t)func;
            }
        }
    }

    ret.multi = false;
    ret.func_addr[0] = (uint64_t)header;

    kprintf("pci: bus: 0x{x} device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", bus, header->device_id, header->header_type, header->class_code, header->subclass, header->vendor_id);
    serial_kprintf("pci: bus: 0x{x} device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", bus, header->header_type, header->class_code, header->subclass, header->vendor_id);

    return ret;
}

void check_bus(uint64_t bus){

    for(uint64_t i = 0; i < PCI_BUS_MAX_DEVICES; i++){
        check_device(bus, i);
    }

}

uint64_t get_header(uint64_t bus, uint64_t device, uint64_t function){
    return config_space_base_addr + ((bus * 256) + (device * 8) + function) * PCIE_CONF_SPACE_WIDTH;
}