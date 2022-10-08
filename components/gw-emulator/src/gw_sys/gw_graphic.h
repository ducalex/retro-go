/*

This program implements the graphics rendering for the different variation
of SM510 family microcomputer.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/

#ifndef _GW_GRAPHIC_H_
#define _GW_GRAPHIC_H_

/****************************/
// H1..4
#define NB_SEGS_COL   4

//a1...a16
#define NB_SEGS_ROW  16

//a,b,c
#define NB_SEGS_BUS   3

//a,b,c base address
#define ADD_SEGA_BASE    0x60
#define ADD_SEGB_BASE    0x70
#define ADD_SEGC_BASE    0x50

/* Function prototypes */
void gw_gfx_init();
void gw_gfx_sm500_rendering(uint16 *framebuffer);
void gw_gfx_sm510_rendering(uint16 *framebuffer);

#endif /* _GW_GRAPHIC_H_ */
