
ENTRY(smp_init_cpu_info)

SECTIONS
{
    . = 0x21000;
    .text :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
        *(.bss)
    }
}
