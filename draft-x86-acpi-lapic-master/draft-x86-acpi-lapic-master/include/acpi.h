/*
* Some definitions of ACPI Tables
* p4nthr0, 2018
*/

#ifndef ACPI_H
#define ACPI_H

#include <types.h>

// Some ACPI Table Signatures
#define RSDP_SIGN_STR   "RSD PTR "
#define RSDP_SIGN_SIZE  8

#define RSDT_SIGN_STR   "RSDT"
#define RSDT_SIGN_SIZE  4

#define XSDT_SIGN_STR   "XSDT"
#define XSDT_SIGN_SIZE  4

#define MADT_SIGN_STR   "APIC"
#define MADT_SIGN_SIZE  4

// Root System Description Pointer (RSDP)
typedef struct _RSDP
{
    u8  signature[8];   // "RSD PTR "
    u8  checksum;       // checksum of bytes 0 to 19 including checksum field
    u8  oemid[6];       // oem supplied string
    u8  revision;       // ACPI 1.0 (0), Current Version (2)
    u32 rdst_addr;      // 32-bit physical address of RSDT
    u32 length;         // length of the table, in bytes, including header
    u64 xsdt_addr;      // 64-bit physical address of XSDT
    u8  ex_chksum;      // checksum of entire table, including both checksum fields
    u8  reserved[3];
} __attribute__((packed)) rsdp_t;

// System Description Table Header
struct DESCRIPTION_HEADER
{
    u8  signature[4];       // table identifier
    u32 length;             // length of the entire table, including the header
    u8  revision;
    u8  checksum;           // the entire table, must add to zero
    u8  oemid[6];           // oem supplied string
    u8  oem_table_id[8];    // oem uses to identify the particular data table
    u32 oem_revision;       // oem supplied revision number
    u32 creator_id;         // vendor id of utility that created the table
    u32 creator_revision;   // revision of utility that created the table
} __attribute__((packed));

// Root System Description Table (RSDT)
typedef struct _RSDT
{
    struct DESCRIPTION_HEADER header;
    u32 entry[];

} __attribute__((packed)) rsdt_t;

// Extended System Description Table (XSDT)
typedef struct _XSDT
{
    struct DESCRIPTION_HEADER header;
    u64 entry[];

} __attribute__((packed)) xsdt_t;

// MADT Interrupt Controller
struct MADT_INTC_HDR
{
    u8  type;
    u8  length;
} __attribute__((packed));

// MADT => Processor Local APIC
struct MADT_INTC_LAPIC
{
    struct MADT_INTC_HDR header;
    u8  acpi_proc_uid;  //
    u8  apic_id;        // processor's local apic id
    u32 flags;          // 0 = unusable
} __attribute__((packed));

// MADT => Local APIC Address Override Structure
struct MADT_INTC_LAPIC_OVERRIDE
{
    struct MADT_INTC_HDR header;
    u16 reserved;
    u64 local_apic_addr;
} __attribute__((packed));

// Multiple APIC Description Table (MADT)
typedef struct _MADT
{
    struct DESCRIPTION_HEADER header;
    u32 local_apic_addr;    // 32-bit physical address
    u32 flags;              // multiple APIC flags (PCAT_COMPAT=1)
    u8  interrupt_controller[];
} __attribute__((packed)) madt_t;

#define MADT_INTC_LAPIC_TYPE            0
#define MADT_INTC_LAPIC_OVERRIDE_TYPE   5

int acpi_get_rsdp(rsdp_t **rsdp);
int acpi_get_madt(rsdp_t *rsdp, madt_t **madt);
void acpi_for_each_madt_intc(madt_t *, u8 type, void (*match)(void *));

#endif
