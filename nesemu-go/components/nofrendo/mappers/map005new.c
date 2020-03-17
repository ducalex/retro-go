/*
** map005.c
** mapper 5 interface
*/

#include <string.h>
#include <noftypes.h>
#include <nes_mmc.h>
#include <nes.h>
#include <log.h>

static void UpdatePrgBanks()
{

}

static void UpdateChrBanks()
{

}


static void map5_write(uint32 address, uint8 value)
{
   printf("mmc5 write: $%02X to $%04X\n", value, address);

}

static uint8 map5_read(uint32 address)
{
   printf("MMC5 read: $%04X", address);

   return 0;
}

static void map5_init(void)
{

}

static void map5_hblank(int vblank)
{

}

/* incomplete SNSS definition */
static void map5_getstate(SnssMapperBlock *state)
{
   state->extraData.mapper5.dummy = 0;
}

static void map5_setstate(SnssMapperBlock *state)
{
   UNUSED(state);
}

static map_memwrite map5_memwrite[] =
{
   /* $5000 - $5015 handled by sound */
   { 0x5016, 0x5FFF, map5_write },
   { 0x6000, 0x7FFF, map5_write },
   { 0x8000, 0xFFFF, map5_write },
   {     -1,     -1, NULL }
};

static map_memread map5_memread[] =
{
   { 0x5204, 0x5206, map5_read },
   { 0x5C00, 0x5FFF, map5_read },
   { 0x6000, 0x7FFF, map5_read },
   {     -1,     -1, NULL }
};

mapintf_t map5new_intf =
{
   5, /* mapper number */
   "MMC5", /* mapper name */
   map5_init, /* init routine */
   NULL, /* vblank callback */
   map5_hblank, /* hblank callback */
   map5_getstate, /* get state (snss) */
   map5_setstate, /* set state (snss) */
   map5_memread, /* memory read structure */
   map5_memwrite, /* memory write structure */
   NULL /* external sound device */
};
