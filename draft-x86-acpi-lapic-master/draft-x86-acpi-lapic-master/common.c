/*
* miscellaneous definitions
*/

#include <types.h>

/* get EBDA base address */
void * get_ebda_addr()
{
    void *ebda = (void *) (*((u16 *) 0x40E) << 4);
    return ebda;
}

/* from openbsd */
int memcmp(const void *s1, const void *s2, u32 n)
{
	if (n != 0)
    {
		const unsigned char *p1 = s1, *p2 = s2;

		do
        {
			if (*p1++ != *p2++)
				return (*--p1 - *--p2);
		} while (--n != 0);
	}

	return 0;
}
