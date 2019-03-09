/*
* functions for smp management
* p4nthr0, 2018
*/

#include <types.h>
#include <acpi.h>
#include <stdbool.h>

#define LAPIC_ID                                0x20
#define LAPIC_VER                               0x30
#define LAPIC_TPR                               0x80
#define LAPIC_APR                               0x90
#define LAPIC_PPR                               0xA0
#define LAPIC_EOI                               0xB0
#define LAPIC_LDR                               0xD0
#define LAPIC_DFR                               0xE0
#define LAPIC_SPIV                              0xF0
#define         LAPIC_SPIV_ENABLE_APIC          0x100
#define LAPIC_ISR                               0x100
#define LAPIC_TMR                               0x180
#define LAPIC_IRR                               0x200
#define LAPIC_ESR                               0x280
#define LAPIC_ICR                               0x300

/* The ICR consists of an int-vector ORed with the following flags: */

/* Destination modes: */
#define         LAPIC_ICR_DS_SELF               0x40000 /* self */
#define         LAPIC_ICR_DS_ALLINC             0x80000 /* all including self */
#define         LAPIC_ICR_DS_ALLEX              0xC0000 /* all excluding self */

/* Only INIT IPI uses this: */
#define         LAPIC_ICR_TM_LEVEL              0x8000  /* level vs edge mode */
/* Only INIT de-assert would NOT use this: */
#define         LAPIC_ICR_LEVELASSERT           0x4000  /* level assert */
/* But, Pentium4 assumes other IPIs have it set, according to Intel specs */
/* So... just always use it. */

#define         LAPIC_ICR_STATUS_PEND           0x1000  /* status check, readonly */
#define         LAPIC_ICR_DM_LOGICAL            0x800   /* logical destination mode */

/* Delivery mode: */
/* FIXED delivery is 0x0 */
#define         LAPIC_ICR_DM_LOWPRI             0x100   /* send to lowest-priority CPU */
#define         LAPIC_ICR_DM_SMI                0x200   /* send SMI */
#define         LAPIC_ICR_DM_NMI                0x400   /* send NMI */
#define         LAPIC_ICR_DM_INIT               0x500   /* send INIT */
#define         LAPIC_ICR_DM_SIPI               0x600   /* send Startup IPI */


#define LAPIC_LVTT                              0x320
#define LAPIC_LVTPC                             0x340
#define LAPIC_LVT0                              0x350
#define LAPIC_LVT1                              0x360
#define LAPIC_LVTE                              0x370
#define LAPIC_TICR                              0x380
#define LAPIC_TCCR                              0x390
#define LAPIC_TDCR                              0x3E0


#define APIC_BROADCAST_ID 0xFF
//#define MAX_CPUS          APIC_BROADCAST_ID

#define LAPIC_ADDR_DEFAULT  0xFEE00000uL
#define IOAPIC_ADDR_DEFAULT 0xFEC00000uL

u32 mp_LAPIC_addr = LAPIC_ADDR_DEFAULT;

#define MP_LAPIC_READ(x)   (*((volatile u32 *) (mp_LAPIC_addr+(x))))
#define MP_LAPIC_WRITE(x,y) (*((volatile u32 *) (mp_LAPIC_addr+(x))) = (y))


#define MP_IOAPIC_READ(x)   (*((volatile u32 *) (mp_IOAPIC_addr+(x))))
#define MP_IOAPIC_WRITE(x,y) (*((volatile u32 *) (mp_IOAPIC_addr+(x))) = (y))

#define MAX_CPU 255
typedef struct _CPU
{
    u32 isbsp;
    u32 lapic_id;
    u64 lapic_base;
} __attribute__((packed)) cpu_t;

static cpu_t cpu_info[MAX_CPU] = { 0 };
static int num_of_cpus = 0;
static int incrementable_counter = 0;
static int apic_counter_initial_value = 0;
static int apic_counter_counted_value = 0;


static u32 adressForWrite[100];
static u32 arrindex={0};

static void get_lapic_info_from_madt(void *madt_intc_hdr)
{
    struct MADT_INTC_LAPIC *madt_intc_lapic = (struct MADT_INTC_LAPIC *) madt_intc_hdr;

    if (madt_intc_lapic->flags)
    {
        cpu_info[num_of_cpus].lapic_id = madt_intc_lapic->apic_id;
        if (num_of_cpus == 0)
        {
            cpu_info[num_of_cpus].isbsp = 1;
        }

        num_of_cpus++;
    }
}

static void update_lapic_addr_for_all_cpus(void *madt_intc_hdr)
{
    struct MADT_INTC_LAPIC_OVERRIDE *madt_intc_lapic_override = (struct MADT_INTC_LAPIC_OVERRIDE *) madt_intc_hdr;

    for (int i = 0; i < num_of_cpus; i++)
    {
        cpu_info[i].lapic_base = madt_intc_lapic_override->local_apic_addr;
    }
}

volatile u32 tick;    /* defined in interrupt_handler.c */
static void
smp_setup_LAPIC_timer_int (void)
{
  /* Temporary handler for the PIT-generated timer IRQ */
  tick++;

 // outb (0x20, 0x20);            /* EOI -- still using PIC */

  /* Cheat a little here: the C compiler will insert 
   *   leave
   *   ret
   * but we want to fix the stack up and IRET so I just
   * stick that in right here. */
  asm volatile ("leave");
  asm volatile ("iret");
}

static void
smp_LAPIC_timer_irq_handler (void)
{
  /* Temporary handler for the LAPIC-generated timer interrupt */
  /* just EOI and ignore it */
  MP_LAPIC_WRITE (LAPIC_EOI, 0);        /* send to LAPIC -- this int came from LAPIC */
  asm volatile ("leave");
  asm volatile ("iret");
}


void
LAPIC_enable_timer(char vec, bool periodic, char divisor) 
{
  char div_flag = 0xB;         /* default: div-by-1 */

  switch(divisor) {
  case 1: div_flag = 0xB; break;
  case 2: div_flag = 0x0; break;
  case 4: div_flag = 0x1; break;
  case 8: div_flag = 0x2; break;
  case 16: div_flag = 0x3; break;
  case 32: div_flag = 0x8; break;
  case 64: div_flag = 0x9; break;
  case 128: div_flag = 0xA; break;
  default: break;
  }

  if (periodic)
    MP_LAPIC_WRITE (LAPIC_LVTT, (1<<17) | (u32)vec); 
  else
    MP_LAPIC_WRITE (LAPIC_LVTT, (u32)vec); 
  MP_LAPIC_WRITE (LAPIC_TDCR, div_flag); 
}


void
LAPIC_start_timer (u32 count)
{
  MP_LAPIC_WRITE (LAPIC_TICR, count);
}

void init_apic_counter( u32* _apicAdress )
{
    MP_LAPIC_WRITE (LAPIC_TPR, 0x00);      /* task priority = 0x0 */
    MP_LAPIC_WRITE (LAPIC_LVTT, 0x10000);  /* disable timer int */
    MP_LAPIC_WRITE (LAPIC_LVTPC, 0x10000); /* disable perf ctr int */
    MP_LAPIC_WRITE (LAPIC_LVT0, 0x08700); /* enable normal external ints */
    MP_LAPIC_WRITE (LAPIC_LVT1, 0x00400); /* enable NMI */
    MP_LAPIC_WRITE (LAPIC_LVTE, 0x10000); /* disable error ints */
    MP_LAPIC_WRITE (LAPIC_SPIV, 0x0010F); /* enable APIC: spurious vector = 0xF */
    /* be sure: */
    MP_LAPIC_WRITE (LAPIC_LVT1, 0x00400); /* enable NMI */
    MP_LAPIC_WRITE (LAPIC_LVTE, 0x10000); /* disable error ints */

    adressForWrite[0] = MP_LAPIC_READ(LAPIC_TPR);
    adressForWrite[1] = MP_LAPIC_READ(LAPIC_LVTT); 
    adressForWrite[2] = MP_LAPIC_READ(LAPIC_LVTPC); 
    adressForWrite[3] = MP_LAPIC_READ(LAPIC_LVT0); 
    adressForWrite[4] = MP_LAPIC_READ(LAPIC_LVT1); 
    adressForWrite[5] = MP_LAPIC_READ(LAPIC_LVTE); 
    adressForWrite[6] = MP_LAPIC_READ(LAPIC_LVT1);
    adressForWrite[7] = MP_LAPIC_READ(LAPIC_SPIV); 
    adressForWrite[8] = MP_LAPIC_READ(LAPIC_LVTE);
    arrindex = 8;
    apic_counter_initial_value = MP_LAPIC_READ(LAPIC_TICR);
    apic_counter_counted_value = MP_LAPIC_READ(LAPIC_TCCR);
    LAPIC_enable_timer(0,true,128);
    LAPIC_start_timer(100);
}



/*
* Fills cpu_info, parsing ACPI tables (MADT entries)
* returns: number of cpus found
*/
int smp_init_cpu_info()
{
    rsdp_t *rsdp;
    madt_t *madt;

    u32 apicFirstCoreBase;

    if (acpi_get_rsdp(&rsdp))
    {
        if (acpi_get_madt(rsdp, &madt))
        {
            acpi_for_each_madt_intc(madt, MADT_INTC_LAPIC_TYPE, get_lapic_info_from_madt);

            for (int i = 0; i < num_of_cpus; i++)
            {
                cpu_info[i].lapic_base.low = madt->local_apic_addr;
                cpu_info[i].lapic_base.high = 0;
            }

            acpi_for_each_madt_intc(madt, MADT_INTC_LAPIC_OVERRIDE_TYPE, update_lapic_addr_for_all_cpus);
        }
    }
    apicFirstCoreBase = cpu_info[0].lapic_base.low;
    init_apic_counter(&apicFirstCoreBase);
    return num_of_cpus;
}