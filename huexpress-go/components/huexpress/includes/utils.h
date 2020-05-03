/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _UTILS_C
#define _UTILS_C

#include "cleantypes.h"

#define MESSAGE_ERROR(x...) osd_log("!! " x)
#define MESSAGE_INFO(x...) osd_log(" * " x)
#ifndef FINAL_RELEASE
#define MESSAGE_DEBUG(x...) osd_log(" ~ " x)
#else
#define MESSAGE_DEBUG(x...) {}
#endif

static inline bool
one_bit_set(uchar arg)
{
	return (arg != 0) && ((-arg & arg) == arg);
}
#endif
