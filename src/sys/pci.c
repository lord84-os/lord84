#include <lord84.h>
#include <stdio.h>
#include "acpi.h"
#include "../mm/vmm.h"
#include "pci.h"

#define PCIE_CONF_SPACE_WIDTH       4096
#define PCI_CONF_SPACE_WIDTH        256

#define PCI_HEADER_TYPE_GENERAL     0x0     // General device
#define PCI_HEADER_TYPE_PCI2PCI     0x1     // PCI to PCI bridge
#define PCI_HEADER_TYPE_PCI2CB      0x2     // PCI to CardBus bridge

#define ID_REGISTER                 0x0     // Vendor ID, 0-15; Device ID, 16-31

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
    /* Counter for the while loop below. We know that the PCIe configuration space is max
       4096 bytes wide, so we make sure we dont go over that */
    uint64_t counter = 0;

    pci_header_0_t *header = (pci_header_0_t*)config_space_base_addr;

    while(counter < PCIE_CONF_SPACE_WIDTH){

        /* If Vendor ID is 0xffff, that means that that device does not exist */
        if(header->vendor_id == 0xffff){
            break;
        }

        kprintf("pci: vendor_id: 0x{x} class: 0x{x} subclass: 0x{x} header type: 0x{xn}", header->vendor_id, header->class_code, header->subclass, header->header_type);
        
        switch (header->header_type) {
            case PCI_HEADER_TYPE_GENERAL:
                counter += sizeof(pci_header_0_t);
                header = (pci_header_t*)((uint64_t)header + sizeof(pci_header_0_t));
                break;
            case PCI_HEADER_TYPE_PCI2PCI:
                counter += sizeof(pci_header_1_t);
                header = (pci_header_t*)((uint64_t)header + sizeof(pci_header_1_t));
                break;
            default:
                kprintf("uhoh\n");
                return;
        }

    }
}

void pci_init(){
    mcfg = (mcfg_t*)find_acpi_table("MCFG");

    if(!mcfg){
        klog(LOG_ERROR, __func__, "Failed to find MCFG table");
        kprintf("pci: PCIe not supported! Halting");
        kkill();
    }

    mcfg = (mcfg_t*)((uint64_t)mcfg + hhdmoffset);

    parse_conf_space();

    /* Map the base address to virtual memory so we can access it */
    extern uint64_t *kernel_page_map;

    vmm_map_page(kernel_page_map, config_space_base_addr, config_space_base_addr - hhdmoffset, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);

    pci_header_t *header = (pci_header_t*)config_space_base_addr;
    kprintf("Vendor ID: 0x{xn}", header->vendor_id);

    enumerate_conf_space();
}