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
    uint8_t access_size;
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

/* Copied from OSDEV wiki */
typedef struct fadt_t{
    desc_header_t header;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    gas_t ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
  
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    gas_t X_PM1aEventBlock;
    gas_t X_PM1bEventBlock;
    gas_t X_PM1aControlBlock;
    gas_t X_PM1bControlBlock;
    gas_t X_PM2ControlBlock;
    gas_t X_PMTimerBlock;
    gas_t X_GPE0Block;
    gas_t X_GPE1Block;

    gas_t sleep_ctrl_reg;
    gas_t sleep_status_reg;

    uint64_t hypervisor_vendor_id;
    
    uint8_t wbinvd;
    uint8_t wbinvd_flush;

    uint8_t proc_c1;
    uint8_t p_lvl2_up;
    uint8_t pwr_button;
    uint8_t slp_button;
    uint8_t fix_rtc;
    uint8_t rtc_s4;
    uint8_t tmr_val_ext;
    uint8_t dck_cap;


}__attribute((packed)) fadt_t;




void acpi_init(void);
uint64_t *find_acpi_table(char *signature);
uint64_t *find_ics(uint64_t type);
uint32_t find_iso(uint8_t legacy);


