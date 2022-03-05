typedef int_least32_t blargg_long;
typedef uint_least32_t blargg_ulong;

#define GET_LE16( addr )        (*(uint16_t*) (addr))
#define GET_LE32( addr )        (*(uint32_t*) (addr))
#define SET_LE16( addr, data )  (void) (*(uint16_t*) (addr) = (data))
#define SET_LE32( addr, data )  (void) (*(uint32_t*) (addr) = (data))

#define GET_LE16SA( addr )      ((int16_t) GET_LE16( addr ))
#define GET_LE16A( addr )       GET_LE16( addr )
#define SET_LE16A( addr, data ) SET_LE16( addr, data )

class SMP
{
public:
    unsigned frequency;
    int32 clock;
	static const uint8 iplrom[64];
	uint32 registers[4];
	uint8 *apuram;
	uint8 *stack;

	SMP();
	~SMP();

	void load_state(uint8 **);
	void save_state(uint8 **);

	void execute(int cycles = 0);
	void power();
	void reset();

private:
	struct Flags
	{
		bool n, v, p, b, h, i, z, c;

		inline operator unsigned() const
		{
			return (n << 7) | (v << 6) | (p << 5) | (b << 4) | (h << 3) | (i << 2) | (z << 1) | (c << 0);
		};

		inline unsigned operator=(unsigned data)
		{
			n = data & 0x80; v = data & 0x40; p = data & 0x20; b = data & 0x10;
			h = data & 0x08; i = data & 0x04; z = data & 0x02; c = data & 0x01;
			return data;
		}

		inline unsigned operator|=(unsigned data) { return operator=(operator unsigned() | data); }
		inline unsigned operator^=(unsigned data) { return operator=(operator unsigned() ^ data); }
		inline unsigned operator&=(unsigned data) { return operator=(operator unsigned() & data); }
	};

	uint32 opcode_cycle;
	uint8 opcode_number;

	uint32 ticks;

	uint16 rd, wr, dp, sp, ya, bit;

	struct Regs
	{
		uint32 pc;
		uint8 sp;
		union
		{
			uint16 ya;
#ifndef __BIG_ENDIAN__
			struct { uint8 a, y; } B;
#else
			struct { uint8 y, a; } B;
#endif
		};
		uint8 x;
		Flags p;
	} regs;

	struct Status
	{
		bool iplrom_enable; // $00f1
		unsigned dsp_addr;	// $00f2
		unsigned ram00f8;	// $00f8
		unsigned ram00f9;	// $00f9
	} status;

	template <unsigned frequency>
	struct Timer
	{
		bool enable;
		unsigned target;
		unsigned stage1_ticks;
		unsigned stage2_ticks;
		unsigned stage3_ticks;

		inline void tick();
		inline void tick(unsigned clocks);
	};

	Timer<128> timer0;
	Timer<128> timer1;
	Timer<16> timer2;

	inline void tick();
	inline void tick(unsigned clocks);

	unsigned op_read(unsigned addr);
	void op_write(unsigned addr, uint8 data);

	inline uint8 op_adc(uint8 x, uint8 y);
	inline uint16 op_addw(uint16 x, uint16 y);
	inline uint8 op_and(uint8 x, uint8 y);
	inline uint8 op_cmp(uint8 x, uint8 y);
	inline uint16 op_cmpw(uint16 x, uint16 y);
	inline uint8 op_eor(uint8 x, uint8 y);
	inline uint8 op_inc(uint8 x);
	inline uint8 op_dec(uint8 x);
	inline uint8 op_or(uint8 x, uint8 y);
	inline uint8 op_sbc(uint8 x, uint8 y);
	inline uint16 op_subw(uint16 x, uint16 y);
	inline uint8 op_asl(uint8 x);
	inline uint8 op_lsr(uint8 x);
	inline uint8 op_rol(uint8 x);
	inline uint8 op_ror(uint8 x);
};

extern SMP smp;
