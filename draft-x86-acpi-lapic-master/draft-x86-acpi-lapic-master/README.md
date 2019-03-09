# draft-x86-acpi-lapic
ACPI and Local APIC in x86 architecture

**[Find LAPIC in ACPI Tables]**   
$ make    
$ qemu-system-x86_64 -hda kernel.bin -cpu core2duo -smp cores=4 -s -S   
$ gdb --nh --command debug.py --batch

```
cpu_info[0].isbsp=0x1
cpu_info[0].lapic_id=0x0
cpu_info[0].lapic_base.low=0xfee00000
cpu_info[0].lapic_base.high=0x0


cpu_info[1].isbsp=0x0
cpu_info[1].lapic_id=0x1
cpu_info[1].lapic_base.low=0xfee00000
cpu_info[1].lapic_base.high=0x0


cpu_info[2].isbsp=0x0
cpu_info[2].lapic_id=0x2
cpu_info[2].lapic_base.low=0xfee00000
cpu_info[2].lapic_base.high=0x0


cpu_info[3].isbsp=0x0
cpu_info[3].lapic_id=0x3
cpu_info[3].lapic_base.low=0xfee00000
cpu_info[3].lapic_base.high=0x0
```
