/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *  additionnal code by Eke-Eke (SMS Plus GX)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   Z80 Memory handlers
 *
 ******************************************************************************/

#ifndef _MEMZ80_H_
#define _MEMZ80_H_

/* Global data */
extern uint8 data_bus_pullup;

/* Function prototypes */
extern uint8 z80_read_unmapped(void);
extern void gg_port_w(uint16 port, uint8 data);
extern uint8 gg_port_r(uint16 port);
extern void ggms_port_w(uint16 port, uint8 data);
extern uint8 ggms_port_r(uint16 port);
extern void sms_port_w(uint16 port, uint8 data);
extern uint8 sms_port_r(uint16 port);
extern void tms_port_w(uint16 port, uint8 data);
extern uint8 tms_port_r(uint16 port);
extern void coleco_port_w(uint16 port, uint8 data);
extern uint8 coleco_port_r(uint16 port);

#endif /* _MEMZ80_H_ */
