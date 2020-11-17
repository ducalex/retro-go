#include "stdbool.h"
#include "stdarg.h"
#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "cpu.h"
#include "sound.h"
#include "mem.h"
#include "lcd.h"
#include "rtc.h"


void emu_init()
{
	emu_reset();
}

/*
 * emu_reset is called to initialize the state of the emulated
 * system. It should set cpu registers, hardware registers, etc. to
 * their appropriate values at powerup time.
 */
void emu_reset()
{
	hw_reset();
	lcd_reset();
	cpu_reset();
	mbc_reset();
	sound_reset();
}

/*
	Time intervals throughout the code, unless otherwise noted, are
	specified in double-speed machine cycles (2MHz), each unit
	roughly corresponds to 0.477us.

	For CPU each cycle takes 2dsc (0.954us) in single-speed mode
	and 1dsc (0.477us) in double speed mode.

	Although hardware gbc LCDC would operate at completely different
	and fixed frequency, for emulation purposes timings for it are
	also specified in double-speed cycles.

	line = 228 dsc (109us)
	frame (154 lines) = 35112 dsc (16.7ms)
	of which
		visible lines x144 = 32832 dsc (15.66ms)
		vblank lines x10 = 2280 dsc (1.08ms)
*/
void IRAM_ATTR emu_run(bool draw)
{
    fb.enabled = draw;
    pcm.pos = 0;

    /* FIXME: djudging by the time specified this was intended
    to emulate through vblank phase which is handled at the
    end of the loop. */
    cpu_emulate(2280);

    /* FIXME: R_LY >= 0; comparsion to zero can also be removed
    altogether, R_LY is always 0 at this point */
    while (R_LY > 0 && R_LY < 144) {
        /* Step through visible line scanning phase */
        cpu_emulate(lcd.cycles);
    }

    /* VBLANK BEGIN */
    if (draw && fb.blit_func) {
		(fb.blit_func)();
    }

	// sys_vsync();

    rtc_tick();

	sound_mix();

    if (!(R_LCDC & 0x80)) {
        /* LCDC operation stopped */
        /* FIXME: djudging by the time specified, this is
        intended to emulate through visible line scanning
        phase, even though we are already at vblank here */
        cpu_emulate(32832);
    }

    while (R_LY > 0) {
        /* Step through vblank phase */
        cpu_emulate(lcd.cycles);
    }
}

void emu_die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	abort();
}
