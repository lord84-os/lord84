#include <stdint.h>
#include <stdbool.h>
typedef struct rsdp_t {
    uint64_t signature;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} __attribute((packed)) rsdp_t;

typedef struct desc_header_t {
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t oem_tableid[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute((packed)) desc_header_t;

typedef struct rsdt_t {
    desc_header_t header;
    uint32_t entries_base[];
} __attribute((packed)) rsdt_t;

typedef struct xsdt_t {
    desc_header_t header;
    uint64_t entries_base[];
} __attribute((packed)) xsdt_t;

typedef struct ics_t {
    uint8_t type;
    uint8_t length;
}__attribute((packed)) ics_t;

typedef struct madt_t {
    desc_header_t header;
    uint32_t lic_address;
    uint32_t flags;
    ics_t ics[];
} __attribute((packed)) madt_t;

typedef struct lapic_ao_t {
    ics_t ics;
    uint16_t reserved;
    uint64_t lapic_address;
}__attribute((packed)) lapic_ao_t;

typedef struct gas_t {
    uint8_t address_space_id;
    uint8_t reg_bit_width;
    uint8_t reg_bit_offset;
    uint8_t reserved;
    uint64_t address;
}__attribute((packed)) gas_t;

typedef struct hpet_t {
    desc_header_t header;
    uint32_t event_timer_blkid;
    gas_t base_address;
    uint8_t hpet_number;
    uint16_t minimum_clk_tick;
    uint8_t oem_attribute;
}__attribute((packed)) hpet_t;

typedef struct ioapic_t{
    ics_t ics;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;
}__attribute((packed)) ioapic_t;

typedef struct iso_t{
    ics_t ics;
    uint8_t bus;
    uint8_t source;
    uint32_t gsi;
    uint16_t flags;
}__attribute((packed)) iso_t;



void acpi_init(void);
uint64_t *find_acpi_table(char *signature);
uint64_t *find_ics(uint64_t type);
uint32_t find_iso(uint8_t legacy);


