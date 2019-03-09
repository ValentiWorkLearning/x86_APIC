/*
* basic data types
*/

#ifndef TYPES_H
#define TYPES_H

// integer definitions
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef struct _u64
{
    u32 low;
    u32 high;
} u64;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif
