#include <lord84.h>
#include <stdio.h>
#include "acpi.h"
#include "../mm/vmm.h"
#include "pci.h"

#define PCIE_CONF_SPACE_WIDTH       4096
#define PCI_CONF_SPACE_WIDTH        256

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

    pci_header_0_t *header = (pci_header_0_t*)config_space_base_addr;
    uint64_t device_id = 0;

    for(uint64_t i = 0; i < end_pci_num; i++){

        /* If vendor id is 0xffff, that means that this segment is unimplemented */
        if(header->header.vendor_id == 0xffff){
            header = (pci_header_0_t*)((uint64_t)header + PCIE_CONF_SPACE_WIDTH);
            continue;
        }

        uint64_t header_type = header->header.header_type;

        switch (header_type) {
            case PCI_HEADER_TYPE_GENERAL:
                /* This current structure is fine */
                kprintf("pci: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", header->header.device_id, header->header.header_type, header->header.class_code, header->header.subclass, header->header.vendor_id);
                serial_kprintf("pci: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", header->header.device_id, header->header.header_type, header->header.class_code, header->header.subclass, header->header.vendor_id);
                break;
            case PCI_HEADER_TYPE_PCI2PCI:
                /* Cast the structure to a type 1 structure (PCI to PCI bridge) */
                ;
                pci_header_1_t *type_1 = (pci_header_1_t*)header;

                kprintf("pci: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", type_1->header.device_id, type_1->header.header_type, type_1->header.class_code, type_1->header.subclass, type_1->header.vendor_id);
                serial_kprintf("pci: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", type_1->header.device_id, type_1->header.header_type, type_1->header.class_code, type_1->header.subclass, type_1->header.vendor_id);
                break;
            default:
                /* Check if this is a multifunction header */
                if((header_type & PCI_HEADER_TYPE_MULTI) != 0){
                    /* Mask the 7th bit (multi-function bit) so we get the header type */
                    uint64_t multi_header_type = header_type & 0x3f;

                    for(uint64_t i = 0; i < PCI_FUNCTION_MAX; i++){

                    }



                }
        }
        header = (pci_header_0_t*)((uint64_t)header + PCIE_CONF_SPACE_WIDTH);

    }

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

    /* Map the base address to virtual memory so we can access it */
    extern uint64_t *kernel_page_map;

    for(uint64_t i = 0; i < end_pci_num; i++){
        vmm_map_page(kernel_page_map, config_space_base_addr + i * PCIE_CONF_SPACE_WIDTH, config_space_base_addr + i * PCIE_CONF_SPACE_WIDTH - hhdmoffset, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);
    }
    

    pci_header_t *header = (pci_header_t*)config_space_base_addr;
    kprintf("Vendor ID: 0x{xn}", header->vendor_id);

    enumerate_conf_space();
}

void check_device(uint64_t bus, uint64_t device, pci_header_t *header){

    /* If vendor id is 0xffff, that means that this device is unimplemented */
    if(header->vendor_id == 0xffff){
        return;
    }

    if((header->header_type & PCI_HEADER_TYPE_MULTI) != 0){
        /* Mask the 7th bit (multi-function bit) so we get the header type */
        uint64_t multi_header_type = multi_header_type & 0x3f;

        for(uint64_t function = 0; function < PCI_FUNCTION_MAX; function++){
            uint64_t *addr = (uint64_t*)(config_space_base_addr + ((bus) << 20 | device << 15 | function << 12));
            pci_header_t *function = (pci_header_t*)((uint64_t)addr);
            kprintf("pci multi: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", function->device_id, function->header_type, function->class_code, function->subclass, function->vendor_id);
            serial_kprintf("pci multi: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", function->device_id, function->header_type, function->class_code, function->subclass, function->vendor_id);
        }

        return;
    }

    kprintf("pci: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", header->device_id, header->header_type, header->class_code, header->subclass, header->vendor_id);
    serial_kprintf("pci: device: 0x{x} type: 0x{x} class: 0x{x} subclass: 0x{x} vendorid: 0x{xn}", header->device_id, header->header_type, header->class_code, header->subclass, header->vendor_id);

    return;
}