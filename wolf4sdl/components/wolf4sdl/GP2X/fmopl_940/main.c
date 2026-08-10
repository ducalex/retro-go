extern void fmopl_Init();
extern int  fmopl_core_control();

void Main940();

void code940(void) __attribute__((naked))
{
	asm ("b .DzzBegin");    // reset, interrupt table
	asm ("b .DzzBegin");
	asm ("b .DzzBegin");
	asm ("b .DzzBegin");
	asm ("b .DzzBegin");
	asm ("b .DzzBegin");
	asm ("b .DzzBegin");
	asm ("b .DzzBegin");
	asm (".DzzBegin:");
	asm ("mov sp, #0x100000");    // set the stack top (1M)
	asm ("sub sp, sp, #4");       // minus 4

	// set up memory region 0 -- the whole 4GB address space
	asm ("mov r0, #63");  // region data
	asm ("mcr p15, 0, r0, c6, c0, 0");
	asm ("mcr p15, 0, r0, c6, c0, 1");
	// set up region 1 which is the first 2 megabytes.
	asm ("mov r0, #0x00000029");  // region data
	asm ("mcr p15, 0, r0, c6, c1, 0");
	asm ("mcr p15, 0, r0, c6, c1, 1");
	// set region 1 to be cacheable (so the first 2M will be cacheable)
	asm ("mov r0, #2");
	asm ("mcr p15, 0, r0, c2, c0, 0");
	asm ("mcr p15, 0, r0, c2, c0, 1");
	// set region 1 to be bufferable too (only data)
	asm ("mcr p15, 0, r0, c3, c0, 0");
	// set protection on for all regions
	asm ("mov r0, #15");
	asm ("mcr p15, 0, r0, c5, c0, 0");
	asm ("mcr p15, 0, r0, c5, c0, 1");

	asm ("mrc p15, 0, r0, c1, c0, 0"); // fetch current control reg
	asm ("orr r0, r0, #1"); // 0x00000001: enable protection unit
	asm ("orr r0, r0, #4"); // 0x00000004: enable D cache
	asm ("orr r0, r0, #0x1000"); // 0x00001000: enable I cache
	asm ("orr r0, r0, #0xC0000000"); //  0xC0000000: async+fastbus
	asm ("mcr p15, 0, r0, c1, c0, 0"); // set control reg

	Main940();
}

void Main940(void)
{
	fmopl_Init();

	while(fmopl_core_control()) {};
}
