#
# simple bootloader to start protected mode kernel
# p4nthr0, 2018

.set BOOTSEG, 0x07c0
.set KRNLSEG, 0x2000
.set CODE32, 0x8
.set DATA32, 0x10
.set CODE16, 0x18
.set DATA16, 0x20

.code16
start:
    ljmp    $BOOTSEG, $main

# some bootloader data
gdt:
    .quad 0x0000000000000000    # NULL Segment
    .quad 0x00CF9A000000FFFF    # Code 32
    .quad 0x00CF92000000FFFF    # Data 32
    .quad 0x000F9A000000FFFF    # Code 16
    .quad 0x000F92000000FFFF    # Data 16

gdtr:
    .short  gdtr-gdt-1
    .long   BOOTSEG * 16 + gdt

dap:
  .byte     0x10
  .byte     0x00
dap_num_of_sectors:
    .short  0x000C
dap_seg_offset:
    .long   0x20000000
dap_sector_index_low:
    .long   0x00000001
dap_sector_index_high:
    .long   0x00000000

main:
    # init data and stack segments
    movw    %cs, %ax
    movw    %ax, %ds
    movw    %ax, %es
    xorw    %ax, %ax
    movw    %ax, %ss
    movw    $(BOOTSEG << 4), %sp
    cld

    # load kernel at 0x20000
    movw    $0x4200, %ax
    movw    $dap, %si
    int     $0x13

    # enable protected mode
    call    boot_enable_pm

.code32
    movl    $DATA32, %eax
    movl    %eax, %ds
    movl    %eax, %es
    movl    %eax, %fs
    movl    %eax, %gs
    movl    %eax, %ss
    movl    $(KRNLSEG << 4), %esp

    # get entry point and call
    movl    KRNLSEG*16+0x18, %eax
    call    *%eax
    jmp     .

.code16
boot_enable_pm:
    cli
    lgdt    gdtr
    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0

    # get 16-bit return
    xorl    %eax, %eax
    popw    %ax

    # absolute address
    addl    $(BOOTSEG * 16), %eax

    # use 32-bit iret
    pushfl
    pushl   $CODE32
    pushl   %eax
    iretl

. = 510
.byte 0x55
.byte 0xAA
