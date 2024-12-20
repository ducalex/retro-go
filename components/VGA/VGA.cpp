#include "VGA.h"
#include <esp_rom_gpio.h>
#include <esp_rom_sys.h>
#include <hal/gpio_hal.h>
#include <driver/periph_ctrl.h>
#include <driver/gpio.h>
#include <rom/gpio.h>
#include <driver/rtc_io.h>
#include <soc/lcd_cam_struct.h>
#include <math.h>
#include <esp_private/gdma.h>
#include "rg_system.h"

#ifndef min
#define min(a,b)((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b)((a)>(b)?(a):(b))
#endif

//borrowed from esp code
#define HAL_FORCE_MODIFY_U32_REG_FIELD(base_reg, reg_field, field_val)    \
{                                                           \
	uint32_t temp_val = base_reg.val;                       \
	typeof(base_reg) temp_reg;                              \
	temp_reg.val = temp_val;                                \
	temp_reg.reg_field = (field_val);                       \
	(base_reg).val = temp_reg.val;                          \
}

VGA::VGA()
{
	bufferCount = 1;
	dmaBuffer = 0;
	usePsram = true;
	dmaChannel = 0;
	backBuffer = 0;
}

VGA::~VGA()
{
	bits = 0;
	backBuffer = 0;
}

extern int Cache_WriteBack_Addr(uint32_t addr, uint32_t size);

void VGA::attachPinToSignal(int pin, int signal)
{
	esp_rom_gpio_connect_out_signal(pin, signal, false, false);
	gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[pin], PIN_FUNC_GPIO);
	gpio_set_drive_capability((gpio_num_t)pin, (gpio_drive_cap_t)3);
}

bool VGA::init(const PinConfig pins, const Mode mode, int bits)
{
	this->pins = pins;
	this->mode = mode;
	this->bits = bits;
	backBuffer = 0;

	//TODO check start

	dmaBuffer = new DMAVideoBuffer(mode.vRes, mode.hRes * (bits / 8), mode.vClones, true, usePsram, bufferCount);
	if(!dmaBuffer->isValid())
	{
		delete dmaBuffer;
		return false;
	}
	RG_LOGI("Allocated DMA buffer.");

	periph_module_enable(PERIPH_LCD_CAM_MODULE);
	periph_module_reset(PERIPH_LCD_CAM_MODULE);
	LCD_CAM.lcd_user.lcd_reset = 1;
	esp_rom_delay_us(100);
	RG_LOGI("Configure lcd periph.");


	//f=240000000/(n+1)
	//n=240000000/f-1;
	int N = round(240000000.0/(double)mode.frequency);
	if(N < 2) N = 2;
	//clk = source / (N + b/a)
	LCD_CAM.lcd_clock.clk_en = 1;
	LCD_CAM.lcd_clock.lcd_clk_sel = 2;			// PLL240M
	// - For integer divider, LCD_CAM_LCD_CLKM_DIV_A and LCD_CAM_LCD_CLKM_DIV_B are cleared.
	// - For fractional divider, the value of LCD_CAM_LCD_CLKM_DIV_B should be less than the value of LCD_CAM_LCD_CLKM_DIV_A.
	LCD_CAM.lcd_clock.lcd_clkm_div_a = 0;
	LCD_CAM.lcd_clock.lcd_clkm_div_b = 0;
	LCD_CAM.lcd_clock.lcd_clkm_div_num = N; 	// 0 => 256; 1 => 2; 14 compfy
	LCD_CAM.lcd_clock.lcd_ck_out_edge = 0;
	LCD_CAM.lcd_clock.lcd_ck_idle_edge = 0;
	LCD_CAM.lcd_clock.lcd_clk_equ_sysclk = 1;


	LCD_CAM.lcd_ctrl.lcd_rgb_mode_en = 1;
	LCD_CAM.lcd_user.lcd_2byte_en = (bits==8)?0:1;
    LCD_CAM.lcd_user.lcd_cmd = 0;
    LCD_CAM.lcd_user.lcd_dummy = 0;
    LCD_CAM.lcd_user.lcd_dout = 1;
    LCD_CAM.lcd_user.lcd_cmd_2_cycle_en = 0;
    LCD_CAM.lcd_user.lcd_dummy_cyclelen = 0;//-1;
    LCD_CAM.lcd_user.lcd_dout_cyclelen = 0;
	LCD_CAM.lcd_user.lcd_always_out_en = 1;
    LCD_CAM.lcd_ctrl2.lcd_hsync_idle_pol = mode.hPol ^ 1;
    LCD_CAM.lcd_ctrl2.lcd_vsync_idle_pol = mode.vPol ^ 1;
    LCD_CAM.lcd_ctrl2.lcd_de_idle_pol = 1;

	LCD_CAM.lcd_misc.lcd_bk_en = 1;
    LCD_CAM.lcd_misc.lcd_vfk_cyclelen = 0;
    LCD_CAM.lcd_misc.lcd_vbk_cyclelen = 0;

	LCD_CAM.lcd_ctrl2.lcd_hsync_width = mode.hSync - 1;				//7 bit
    LCD_CAM.lcd_ctrl.lcd_hb_front = mode.blankHorizontal() - 1;		//11 bit
    LCD_CAM.lcd_ctrl1.lcd_ha_width = mode.hRes - 1;					//12 bit
    LCD_CAM.lcd_ctrl1.lcd_ht_width = mode.totalHorizontal();			//12 bit

	LCD_CAM.lcd_ctrl2.lcd_vsync_width = mode.vSync - 1;				//7bit
    HAL_FORCE_MODIFY_U32_REG_FIELD(LCD_CAM.lcd_ctrl1, lcd_vb_front, mode.vSync + mode.vBack - 1);		//8bit
    LCD_CAM.lcd_ctrl.lcd_va_height = mode.vRes * mode.vClones - 1;					//10 bit
    LCD_CAM.lcd_ctrl.lcd_vt_height = mode.totalVertical() - 1;		//10 bit

	LCD_CAM.lcd_ctrl2.lcd_hs_blank_en = 1;
	HAL_FORCE_MODIFY_U32_REG_FIELD(LCD_CAM.lcd_ctrl2, lcd_hsync_position, 0);//mode.hFront);

	LCD_CAM.lcd_misc.lcd_next_frame_en = 1; //?? limitation
	RG_LOGI("set output bits.");

	if(bits == 8)
	{
		int pins[8] = {
			this->pins.r[2], this->pins.r[3], this->pins.r[4],
			this->pins.g[3], this->pins.g[4], this->pins.g[5],
			this->pins.b[3], this->pins.b[4]
		};
		for (int i = 0; i < bits; i++)
			if (pins[i] >= 0)
				attachPinToSignal(pins[i], LCD_DATA_OUT0_IDX + i);
	}
	else if(bits == 16)
	{
		int pins[16] = {
			this->pins.r[0], this->pins.r[1], this->pins.r[2], this->pins.r[3], this->pins.r[4],
			this->pins.g[0], this->pins.g[1], this->pins.g[2], this->pins.g[3], this->pins.g[4], this->pins.g[5],
			this->pins.b[0], this->pins.b[1], this->pins.b[2], this->pins.b[3], this->pins.b[4]
		};
		for (int i = 0; i < bits; i++)
		{
			if (pins[i] >= 0)
			{
				RG_LOGI("Set pin%i to %i", i, pins[i]);
				attachPinToSignal(pins[i], LCD_DATA_OUT0_IDX + i);
			}
		}
	}
	attachPinToSignal(this->pins.hSync, LCD_H_SYNC_IDX);
	attachPinToSignal(this->pins.vSync, LCD_V_SYNC_IDX);
	RG_LOGI("conf gdma.");

	gdma_channel_alloc_config_t dma_chan_config =
	{
		.direction = GDMA_CHANNEL_DIRECTION_TX,
	};
	gdma_channel_handle_t dmaCh;
	gdma_new_channel(&dma_chan_config, &dmaCh);
	dmaChannel = (int)dmaCh;
	gdma_connect(dmaCh, GDMA_MAKE_TRIGGER(GDMA_TRIG_PERIPH_LCD, 0));
	gdma_transfer_ability_t ability =
	{
        .sram_trans_align = 4,
        .psram_trans_align = 64,
    };
    gdma_set_transfer_ability(dmaCh, &ability);

	//TODO check end

	return true;
}

bool VGA::start()
{
	//TODO check start
	//very delicate... dma might be late for peripheral
	gdma_reset((gdma_channel_handle_t)dmaChannel);
    esp_rom_delay_us(1);
    LCD_CAM.lcd_user.lcd_start = 0;
    LCD_CAM.lcd_user.lcd_update = 1;
	esp_rom_delay_us(1);
	LCD_CAM.lcd_misc.lcd_afifo_reset = 1;
    LCD_CAM.lcd_user.lcd_update = 1;
	gdma_start((gdma_channel_handle_t)dmaChannel, (intptr_t)dmaBuffer->getDescriptor());
    esp_rom_delay_us(1);
    LCD_CAM.lcd_user.lcd_update = 1;
	LCD_CAM.lcd_user.lcd_start = 1;
	//TODO check end
	return true;
}

bool VGA::show()
{
	//TODO check start
	dmaBuffer->flush(backBuffer);
	if(bufferCount <= 1)
		return true;
	dmaBuffer->attachBuffer(backBuffer);
	backBuffer = (backBuffer + 1) % bufferCount;
	//TODO check end
	return true;
}

void VGA::dot(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	int v = y;
	int h = x;
	if(x >= mode.hRes || y >= mode.vRes) return;
	if(bits == 8)
		dmaBuffer->getLineAddr8(y, backBuffer)[x] = (r >> 5) | ((g >> 5) << 3) | (b & 0b11000000);
	else if(bits == 16)
		dmaBuffer->getLineAddr16(y, backBuffer)[x] = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);

}

void VGA::dot(int x, int y, int rgb)
{
	int v = y;
	int h = x;
	if(x >= mode.hRes || y >= mode.vRes) return;
	if(bits == 8)
		dmaBuffer->getLineAddr8(y, backBuffer)[x] = rgb;
	else if(bits == 16)
		dmaBuffer->getLineAddr16(y, backBuffer)[x] = rgb;

}

void VGA::dotdit(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	if(x >= mode.hRes || y >= mode.vRes) return;
	if(bits == 8)
	{
		r = min((rand() & 31) | (r & 0xe0), 255);
		g = min((rand() & 31) | (g & 0xe0), 255);
		b = min((rand() & 63) | (b & 0xc0), 255);
		dmaBuffer->getLineAddr8(y, backBuffer)[x] = (r >> 5) | ((g >> 5) << 3) | (b & 0b11000000);
	}
	else
	if(bits == 16)
	{
		r = min((rand() & 7) | (r & 0xf8), 255);
		g = min((rand() & 3) | (g & 0xfc), 255);
		b = min((rand() & 7) | (b & 0xf8), 255);
		dmaBuffer->getLineAddr16(y, backBuffer)[x] = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
	}
}

int VGA::rgb(uint8_t r, uint8_t g, uint8_t b)
{
	if(bits == 8)
		return (r >> 5) | ((g >> 5) << 3) | (b & 0b11000000);
	else if(bits == 16)
		return (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
	return 0;
}

void VGA::clear(int rgb)
{
	for(int y = 0; y < mode.vRes; y++)
		for(int x = 0; x < mode.hRes; x++)
			dot(x, y, rgb);
}