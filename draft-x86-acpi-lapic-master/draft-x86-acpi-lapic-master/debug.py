
import gdb

bkpaddr = '*0x7c77'

class ShowCpuInfo(gdb.Breakpoint):
    def stop(self):
        # load symbol
        gdb.execute('set confirm off')
        gdb.execute('add-symbol-file kernel.elf 0x21000')

        # get variables
        cpu_info = int(gdb.parse_and_eval('&cpu_info'))
        # cpu_info = gdbvalue.cast(gdb.lookup_type('long').pointer()).dereference()

        num_of_cpus = int(gdb.parse_and_eval('num_of_cpus'))
        # num_of_cpus = gdbvalue.cast(gdb.lookup_type('long').pointer()).dereference()

        incrementable_counter = int(gdb.parse_and_eval('incrementable_counter' ))
        print( incrementable_counter )

        apic_counter_initial_value = int(gdb.parse_and_eval('apic_counter_initial_value' ))
        print( apic_counter_initial_value )

        apic_counter_counted_value = int(gdb.parse_and_eval('apic_counter_counted_value' ))
        print( apic_counter_initial_value )

        apic_counter_counted_value = int(gdb.parse_and_eval('apic_counter_counted_value' ))
        print( apic_counter_initial_value )

        apic_adress = int (gdb.parse_and_eval('apic_adress'))
        print( '0x%x'% apic_adress )

        arrindex = int(gdb.parse_and_eval('arrindex'))

        adressForWrite = int(gdb.parse_and_eval('adressForWrite'))
        for a in range(arrindex):
            print('')
            ptr = adressForWrite + a*4
            value = gdb.Value(ptr).cast(gdb.lookup_type('unsigned long').pointer()).dereference()
            print('adress for write [%d].=0x%x' % (a, int(value)))


        # dump cpu_info structure
        for i in range(num_of_cpus):
            ptr = cpu_info + i * 16

            print('')
            value = gdb.Value(ptr).cast(gdb.lookup_type('unsigned long').pointer()).dereference()
            print('cpu_info[%d].isbsp=0x%x' % (i, int(value)))

            value = gdb.Value(ptr + 4).cast(gdb.lookup_type('unsigned long').pointer()).dereference()
            print('cpu_info[%d].lapic_id=0x%x' % (i, int(value)))

            value = gdb.Value(ptr + 8).cast(gdb.lookup_type('unsigned long').pointer()).dereference()
            print('cpu_info[%d].lapic_base.low=0x%x' % (i, int(value)))

            value = gdb.Value(ptr + 12).cast(gdb.lookup_type('unsigned long').pointer()).dereference()
            print('cpu_info[%d].lapic_base.high=0x%x' % (i, int(value)))
            print('')

        return True

gdb.execute('target remote localhost:1234')
ShowCpuInfo(bkpaddr)
gdb.execute('continue')