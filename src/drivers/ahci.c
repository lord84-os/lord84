
#include <SFB25.h>
#include <stdio.h>
#include "../hal/apic.h"
#include "../sys/pci.h"
#include "../mm/vmm.h"
#include "../hal/idt.h" 

#define AHCI_MSE            0x02
#define AHCI_BME            0x03
#define AHCI_INT_ENABLED   (1 << 10)

#define AHCI_CLASS_ID       0x01
#define AHCI_SUBCLASS_ID    0x06

#define AHCI_HOST_CAP_REG   0x00
#define AHCI_GHC_REG        0X04
#define AHCI_INT_STATUS_REG 0x08
#define AHCI_PORTS_IMPL_REG 0x0C
#define AHCI_BOHC_REG       0x28

uint64_t ahci_base_address = 0;

uint32_t ahci_read_reg(uint16_t reg){
    return *(uint32_t*)((uint64_t)ahci_base_address + reg);
}

void ahci_write_reg(uint16_t reg, uint32_t data){
    *(uint32_t*)((uint64_t)ahci_base_address + reg) = data;
}


void ahci_init(){
    pci_header_0_t *header = (pci_header_0_t *)pci_find_device(AHCI_CLASS_ID, AHCI_SUBCLASS_ID);

    if(!header){
        klog(LOG_ERROR, __func__, "AHCI controller not found!");
        kkill();
    }

    kprintf("size of header: 0x{xn}", sizeof(pci_header_0_t));

    extern uint64_t hhdmoffset;

    ahci_base_address = header->bar5 & 0xfffff000;

    kprintf("ahci: 0x{x}\n", ahci_base_address);

    /* Enable bus master, memory space and interrupts */
    header->header.command |= AHCI_MSE | AHCI_BME | AHCI_INT_ENABLED;

    /* Map the AHCI registers */
    kernel_map_pages((uint64_t*)ahci_base_address, 1, PTE_BIT_RW | PTE_BIT_NX | PTE_BIT_UNCACHABLE);

    ahci_base_address += hhdmoffset;

    /* BIOS/OS Handoff */
    kprintf("ahci: Performing BIOS/OS handoff\n");
    ahci_write_reg(AHCI_BOHC_REG, ahci_read_reg(AHCI_BOHC_REG) | 0x2); // Set the OS Owned Semaphore bit - OS now owns the HBA

    uint32_t bohc = ahci_read_reg(AHCI_BOHC_REG);

    /* Wait for the handoff to complete*/
    while((bohc & 0b01) != 1 && (bohc & 0b1) != 0){
        apic_sleep(200);
        bohc = ahci_read_reg(AHCI_BOHC_REG);
    }

    /* Reset the controller */
    ahci_write_reg(AHCI_GHC_REG, 1);

    /* Set the IRQ and enable interrupts */
    kprintf("ahci: Requesting pin {d}\n", header->interrupt_pin);

}