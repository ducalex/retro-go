/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes/mem.h: Memory emulation header
**
*/

#pragma once

#define MEM_ADDRSPACE 0x10000
#define MEM_PAGESIZE  0x800
#define MEM_PAGEMASK  0x7FF
#define MEM_PAGESHIFT (11)
#define MEM_PAGECOUNT (MEM_ADDRSPACE / MEM_PAGESIZE)

#define MEM_RAMSIZE   0x800

#define MEM_PAGE_HAS_READ_HANDLER   (1 << 0)
#define MEM_PAGE_HAS_WRITE_HANDLER  (1 << 2)
#define MEM_PAGE_HAS_MEMORY         (1 << 3)

#define MEM_PAGE_NOT_MAPPED         NULL

#define MEM_HANDLERS_MAX     32

#define LAST_MEMORY_HANDLER  { -1, -1, NULL }

typedef struct
{
   uint32 min_range, max_range;
   uint8 (*handler)(uint32 address);
} mem_read_handler_t;

typedef struct
{
   uint32 min_range, max_range;
   void (*handler)(uint32 address, uint8 value);
} mem_write_handler_t;

typedef struct
{
   // System RAM
   uint8 ram[MEM_RAMSIZE];

   /* Memory map */
   uint8 *pages[MEM_PAGECOUNT];
   uint32 flags[MEM_PAGECOUNT];

   /* Special memory handlers */
   mem_read_handler_t read_handlers[MEM_HANDLERS_MAX];
   mem_write_handler_t write_handlers[MEM_HANDLERS_MAX];

   /* Dummy memory to trap access to unmapped regions */
   uint8 *dummy; // [MEM_PAGESIZE]
} mem_t;

mem_t *mem_init_(void);
void mem_shutdown(void);
void mem_reset(void);
void mem_setpage(uint32 page, uint8 *ptr);
uint8 *mem_getpage(uint32 page);
uint8 mem_getbyte(uint32 address);
uint32 mem_getword(uint32 address);
void mem_putbyte(uint32 address, uint8 value);
