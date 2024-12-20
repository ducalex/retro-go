
#ifndef PINCONFIG_H
#define PINCONFIG_H
class PinConfig;
class PinConfig
{
	public:
		static const PinConfig VGAPurple;
	public:
		int r[5];
		int g[6];
		int b[5];
		int hSync, vSync;

	PinConfig() {};

	PinConfig(
		int r0, int r1, int r2, int r3, int r4, 
		int g0, int g1, int g2, int g3, int g4, int g5, 
		int b0, int b1, int b2, int b3, int b4,
		int hSync, int vSync)
		{
			r[0] = r0; r[1] = r1; r[2] = r2; r[3] = r3; r[4] = r4;
			g[0] = g0; g[1] = g1; g[2] = g2; g[3] = g3; g[4] = g4; g[5] = g5;
			b[0] = b0; b[1] = b1; b[2] = b2; b[3] = b3; b[4] = b4;
			this->hSync = hSync;
			this->vSync = vSync;
		}
};
#endif //PINCONFIG_H