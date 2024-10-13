#include <stdint.h>
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

void acpi_init(void);


