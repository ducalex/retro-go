/* gameplaySP
 *
 * Copyright (C) 2023 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"

int serial_mode = SERIAL_MODE_AUTO;

static u32 serial_irq_cycles = 0;

// Timings are very aproximate, hopefully they are good enough.
#define CLOCK_CYC_256KHZ_8BIT        524    // CLOCK / 256KHz * 8
#define CLOCK_CYC_256KHZ_32BIT      2097    // CLOCK / 256KHz * 32
#define CLOCK_CYC_2MHZ_8BIT           67    // CLOCK / 2MHz * 8
#define CLOCK_CYC_2MHZ_32BIT         268    // CLOCK / 2MHz * 32
#define CLOCK_CYC_64KHZ_32BIT       8192    // CLOCK / 64kHz * 32

// Available serial modes
#define SERIAL_MODE_NORMAL      0
#define SERIAL_MODE_MULTI       1
#define SERIAL_MODE_UART        2
#define SERIAL_MODE_GPIO        3
#define SERIAL_MODE_JOYBUS      4

static u32 get_serial_mode(u16 siocnt, u16 rcnt) {
  if (rcnt & 0x8000) {
    if (rcnt & 0x4000)
      return SERIAL_MODE_JOYBUS;
    else
      return SERIAL_MODE_GPIO;
  } else {
    if (siocnt & 0x2000)
      return (siocnt & 0x1000) ? SERIAL_MODE_UART : SERIAL_MODE_MULTI;
    else
      return SERIAL_MODE_NORMAL;
  }
}

cpu_alert_type write_rcnt(u16 value) {
  u16 pval = read_ioreg(REG_RCNT);
  write_ioreg(REG_RCNT, value);

  switch (get_serial_mode(read_ioreg(REG_SIOCNT), value)) {
  case SERIAL_MODE_GPIO:
    // Check if we are pulling PD high. This resets the RFU module.
    if (serial_mode == SERIAL_MODE_RFU) {
      if ((value & 0x20) && !(pval & 0x02))
        rfu_reset();
    }
    break;

  case SERIAL_MODE_NORMAL:
    break;
  };

  return CPU_ALERT_NONE;
}

cpu_alert_type write_siocnt(u16 value) {
  u16 oldval = read_ioreg(REG_SIOCNT);
  u16 newval = (value & 0x7F8B) | (oldval & 0x0004);

  switch (get_serial_mode(newval, read_ioreg(REG_RCNT))) {
  case SERIAL_MODE_NORMAL:
    // For connected Wireless devices
    if (serial_mode == SERIAL_MODE_RFU) {
      // If Start bit is set, we are in master mode (internal clock), and
      // there's no ongoing transmission, we start a transmission.
      if ((newval & 0x0080) && (newval & 0x1) && !serial_irq_cycles) {
        // We simulate the data is immediately sent to the RFU module
        // and some data is received back (we simulate some delay too).
        u32 rval = rfu_transfer(read_ioreg32(REG_SIODATA32_L));
        write_ioreg(REG_SIODATA32_L, rval & 0xFFFF);
        write_ioreg(REG_SIODATA32_H, rval >> 16);

        // We schedule a future event to clear the "working" bit
        // and potentially generate an IRQ.
        serial_irq_cycles = (newval & 0x2) ? CLOCK_CYC_2MHZ_8BIT :
                                             CLOCK_CYC_256KHZ_8BIT;
        if (newval & 0x1000)
          serial_irq_cycles <<= 2;   // 4 times longer to send 32 bits.
      }

      // RFU SI/SO ack logic emulation.
      if (newval & 0x1) {   // Clock internal (master mode)
        // When GBA goes busy (device should be already busy) we can switch to ready.
        if ((newval & 0x0008) && !(oldval & 0x0008))
          newval &= ~0x0004;
      } else {   // Clock external (slave mode)
        // If SO goes high we make SI go high as well, to signal busy.
        if ((newval & 0x0008) && !(oldval & 0x0008))
          newval |= 0x0004;
        // Once SO goes low (GBA not busy anymore), lower SI as well.
        if (!(newval & 0x0008) && (oldval & 0x0008))
          newval &= ~0x0004;
      }
    }
    else if (serial_mode == SERIAL_MODE_GBP) {
      // Serial configured in slave mode, waiting for incoming word.
      if ((newval & 0x0080) && !(newval & 0x1) && !serial_irq_cycles) {
        // Send a new GBP sequence
        u32 rval = gbp_transfer(read_ioreg32(REG_SIODATA32_L));
        write_ioreg(REG_SIODATA32_L, rval & 0xFFFF);
        write_ioreg(REG_SIODATA32_H, rval >> 16);

        // Schedule the interrupt/reception. We simulate a 64KHz clock, which
        // translates to ~32 exchanges per frame (~2 rumble updates per frame).
        serial_irq_cycles = CLOCK_CYC_64KHZ_32BIT;
      }
    }
    break;
  };

  // Update reg with its new value
  write_ioreg(REG_SIOCNT, newval);

  return CPU_ALERT_NONE;
}

// How many cycles until the next serial event happens. Return MAX otherwise.
u32 serial_next_event() {
  if (serial_irq_cycles)
    return serial_irq_cycles;
  return ~0U;
}

// Account for consumed cycles and return if a serial IRQ should be raised.
bool update_serial(unsigned cycles) {
  // Might wanna check if the connected device has some update (IRQ).
  switch (serial_mode) {
  case SERIAL_MODE_RFU:
    if (rfu_update(cycles))
      return true;
    break;
  };

  if (serial_irq_cycles) {
    if (serial_irq_cycles > cycles)
      serial_irq_cycles -= cycles;
    else {
      // Event happening now!
      serial_irq_cycles = 0;
      // Clear the send bit, signal data is ready.
      // Set the device busy bit, to perform the weird SO/SI handshake.
      write_ioreg(REG_SIOCNT, (read_ioreg(REG_SIOCNT) & ~0x80) | 0x04);
      // Return if IRQs are enabled.
      return read_ioreg(REG_SIOCNT) & 0x4000;
    }
  }

  return false;
}

