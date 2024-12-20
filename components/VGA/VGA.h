#ifndef VGA_H
#define VGA_H

#include "PinConfig.h"
#include "Mode.h"
#include "DMAVideoBuffer.h"

class VGA
{
	public:
	Mode mode;
	int bufferCount;
	int bits;
	PinConfig pins;
	int backBuffer;
	DMAVideoBuffer *dmaBuffer;
	bool usePsram;
	int dmaChannel;
	
	public:
	VGA();
	~VGA();
	bool init(const PinConfig pins, const Mode mode, int bits);
	bool start();
	bool show();
	void clear(int rgb = 0);
	void dot(int x, int y, uint8_t r, uint8_t g, uint8_t b);
	void dot(int x, int y, int rgb);
	void dotdit(int x, int y, uint8_t r, uint8_t g, uint8_t b);
	int rgb(uint8_t r, uint8_t g, uint8_t b);
	protected:
	void attachPinToSignal(int pin, int signal);
};

#endif //VGA_h