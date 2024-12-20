#ifndef MODE_H
#define MODE_H
#include <stdint.h>

class Mode;
class Mode
{
	public:
	static const Mode MODE_640x400x70;
	static const Mode MODE_320x200x70;
	static const Mode MODE_640x480x60;
	static const Mode MODE_320x240x60;
	static const Mode MODE_800x600x56;
	static const Mode MODE_800x600x60;
	static const Mode MODE_400x300x60;
	static const Mode MODE_1024x768x43;
	static const Mode MODE_1024x768x60;
	static const Mode MODE_1280x720x60;

	public:
	uint32_t hFront, hSync, hBack, hRes, hPol;
	uint32_t vFront, vSync, vBack, vRes, vPol, vClones;
	uint32_t frequency;
	
	Mode()
	{
	}
	
	Mode(const 
	
	Mode &m)
	{
		this->hFront = m.hFront;
		this->hSync = m.hSync;
		this->hBack = m.hBack;
		this->hRes = m.hRes;
		this->hPol = m.hPol;
		this->vFront = m.vFront;
		this->vSync = m.vSync;
		this->vBack = m.vBack;
		this->vRes = m.vRes;
		this->vPol = m.vPol;
		this->frequency = m.frequency;
		this->vClones = m.vClones;
	}

	
	
	Mode(int hFront, int hSync, int hBack, int hRes, int vFront, int vSync, int vBack, int vRes, int frequency, 
		int hPol = 1, int vPol = 1, int vClones = 1)
	{
		this->hFront = hFront;
		this->hSync = hSync;
		this->hBack = hBack;
		this->hRes = hRes;
		this->hPol = hPol;
		this->vFront = vFront;
		this->vSync = vSync;
		this->vBack = vBack;
		this->vRes = vRes;
		this->vPol = vPol;
		this->frequency = frequency;
		this->vClones = vClones;
	}

	int totalHorizontal() const
	{
		return hFront + hSync + hBack + hRes;
	}

	int totalVertical() const
	{
		return vFront + vSync + vBack + vRes * vClones;
	}

	int blankHorizontal() const
	{
		return hFront + hSync + hBack;
	}

	int blankVertical() const
	{
		return vFront + vSync + vBack;
	}
};

#endif //MODE_h