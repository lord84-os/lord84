#include <stdint.h>
void pci_init();

typedef struct pci_header_t {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
}__attribute((packed)) pci_header_t;

typedef struct pci_header_0_t {
    pci_header_t header;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbus_cis_ptr;
    uint16_t subsytem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base;
    uint8_t capabilities_ptr;
    uint8_t reserved1;
    uint16_t reserved2;
    uint32_t reserved3;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
}__attribute((packed)) pci_header_0_t;

typedef struct pci_header_1_t {
    pci_header_t header;
    uint32_t bar0;
    uint32_t bar1;
    uint8_t primary_bus_number;
    uint8_t secondary_bus_number;
    uint8_t subordinate_bus_number;
    uint8_t secondary_latency_timer;
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetch_base_;
    uint16_t prefetch_limit;
    uint32_t prefetch_base_upper;
    uint32_t prefetch_limit_upper;
    uint16_t io_base_upper;
    uint16_t io_limit_upper;
    uint8_t capability_ptr;
    uint8_t reserved1;
    uint16_t reserved2;
    uint32_t expansion_rom_base;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;

}__attribute((packed)) pci_header_1_t;

void check_device(uint64_t bus, uint64_t device);

uint64_t get_header(uint64_t bus, uint64_t device, uint64_t function);