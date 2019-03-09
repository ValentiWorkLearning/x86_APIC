/*
* functions for parsing ACPI Tables (just for finding LAPIC)
* p4nthr0, 2018
*/

#include <acpi.h>

void * get_ebda_addr();
int memcmp(const void *s1, const void *s2, u32 n);

static u8 chksum(void *mem, u32 size)
{
    u8 sum = 0;
    u8 *mem_ptr = (u8 *) mem;

    while(size--)
    {
        sum += *mem_ptr++;
    }

    return sum;
}

static int validate_rsdp_chksum(rsdp_t *rsdp)
{
    u32 size = (rsdp->revision == 0) ? 20 : rsdp->length;
    return (chksum(rsdp, size) == 0);
}

/*
* Search RSDP table in physical memory
* returns: 1 on success, 0 on failure
*/
int acpi_get_rsdp(rsdp_t **rsdp)
{
    // Searching for a valid RSDP structure signature and checksum match
    // Search on 1KB of Extended BIOS Data Area (40:0Eh) (16-byte boundaries)
    u8 *ebda_start = (u8 *) get_ebda_addr();
    for (int i = 0; i < 1024; i += 16)
    {
        if (memcmp(ebda_start + i, RSDP_SIGN_STR, RSDP_SIGN_SIZE) == 0)
        {
            if (validate_rsdp_chksum((rsdp_t *)(ebda_start + i)))
            {
                *rsdp = (rsdp_t *)(ebda_start + i);
                return 1;
            }
        }
    }

    // Search on BIOS read-only memory space between E0000h-FFFFFh
    // (16-byte boundaries)
    for (u32 bios_mem = 0xE0000; bios_mem < 0xFFFFF; bios_mem += 16)
    {
        if (memcmp((void *) bios_mem, RSDP_SIGN_STR, RSDP_SIGN_SIZE) == 0)
        {
            if (validate_rsdp_chksum((rsdp_t *)(bios_mem)))
            {
                *rsdp = (rsdp_t *)(bios_mem);
                return 1;
            }
        }
    }

    return 0;
}

static int get_madt_from_rsdt(rsdt_t *rsdt, madt_t **madt)
{
    // get size of table
    u32 size = rsdt->header.length;
    u32 num_of_entries = (size - sizeof(struct DESCRIPTION_HEADER)) / sizeof(u32);

    // sanity checks
    if (memcmp(rsdt, RSDT_SIGN_STR, RSDT_SIGN_SIZE) || chksum(rsdt, size))
    {
        return 0;
    }

    // for each RSDT entry, search for MADT
    for (u32 i = 0; i < num_of_entries; i++)
    {
        if (memcmp((void *) rsdt->entry[i], MADT_SIGN_STR, MADT_SIGN_SIZE) == 0)
        {
            *madt = (madt_t *) rsdt->entry[i];
            return 1;
        }
    }

    return 0;
}

/* For now, we can't dereference 64-bit pointers
static int get_madt_from_xsdt(xsdt_t *xsdt, madt_t **madt)
{
    // get size of table
    u32 size = xsdt->header.length;
    u32 num_of_entries = (size - sizeof(struct DESCRIPTION_HEADER)) / sizeof(u64);

    // sanity checks
    if (memcmp(rsdt, XSDT_SIGN_STR, XSDT_SIGN_SIZE) || chksum(xsdt, size))
    {
        return 0;
    }

    // for each XSDT entry, search for MADT
    for (u32 i = 0; i < num_of_entries; i++)
    {

    }

    return 0;
}
*/

/*
* Search MADT in physical memory
* returns: 1 on success, 0 on failure
*/
int acpi_get_madt(rsdp_t *rsdp, madt_t **madt)
{
    // For ACPI 1.0, use RSDT
    if (rsdp->revision == 0)
    {
        return get_madt_from_rsdt((rsdt_t *) rsdp->rdst_addr, madt);
    }

    return get_madt_from_rsdt((rsdt_t *) rsdp->rdst_addr, madt);
    /* return get_madt_from_xsdt((xsdt_t *) rsdp->xsdt_addr, madt); */
}

/*
* Loop around MADT.interrupt_controller[] structures
* Calls match(), whenever a specific type is found
*/
void acpi_for_each_madt_intc(madt_t *madt, u8 type, void (*match)(void *))
{
    u32 size = madt->header.length;
    struct MADT_INTC_HDR *madt_intc_hdr = (struct MADT_INTC_HDR *) &madt->interrupt_controller[0];
    void *madt_end_ptr = ((u8 *) madt) + size;

    // loop around MADT.interrupt_controller[]
    if (!memcmp(madt, MADT_SIGN_STR, MADT_SIGN_SIZE) && !chksum(madt, size))
    {
        while ((void *) madt_intc_hdr < madt_end_ptr)
        {
            if (madt_intc_hdr->type == type)
            {
                match(madt_intc_hdr);
            }

            madt_intc_hdr = (struct MADT_INTC_HDR *) (((u8 *) madt_intc_hdr) + madt_intc_hdr->length);
        }
    }
}
