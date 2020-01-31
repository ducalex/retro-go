#if 0
/*
 * new sound core for 1.1.x
 */



enum sevcode
{
	SEV_S1E,
	SEV_S2E,
	SEV_S3E,
	SEV_S4E,
	SEV_S1L,
	SEV_S2L,
	SEV_S3L,
	SEV_S4L,
	SEV_SW,
	SEV_WAV
};


struct sev
{
	int prev, next;
	int time;
};



static struct sev *sevs;



void sound_mix(int cycles)
{

}


void sound_update(int force)
{
	int now = 0;

	for (;;)
	{
		if (sevs->time > cpu.snd) break;

	}
}
#endif
