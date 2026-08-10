//
// GP2X specific code
//
// by Pickle
//

#if defined(GP2X)

#include "gp2x.h"

static bool volume_init = false;
static unsigned int screenshot_count = 0;

#if defined(GP2X_940)
static int volume = 70;
#else
static int volume = 10;
#endif

static int intUp          = 0;
static int intDown        = 0;
static int intLeft        = 0;
static int intRight       = 0;
static int intUpRight     = 0;
static int intUpLeft      = 0;
static int intDownRight   = 0;
static int intDownLeft    = 0;
static int intButtonR     = 0;
static int intButtonL     = 0;
static int intButtonA     = 0;
static int intButtonB     = 0;
static int intButtonX     = 0;
static int intButtonY     = 0;
static int intButtonSel   = 0;
static int intButtonSrt   = 0;
static int intButtonStick = 0;

#if defined(GP2X_940)
void GP2X_Shutdown(void)
{
	YM3812Shutdown();
}

void GP2X_MemoryInit( void )
{
	SDL_GP2X_AllowGfxMemory(NULL,0);
}
#endif

void GP2X_AdjustVolume( int direction )
{
	if( volume <= 10 )
	{
		if( direction == VOLUME_UP )   volume += VOLUME_CHANGE_RATE/2;
		if( direction == VOLUME_DOWN ) volume -= VOLUME_CHANGE_RATE/2;
	}
	else
	{
		if( direction == VOLUME_UP )   volume += VOLUME_CHANGE_RATE;
		if( direction == VOLUME_DOWN ) volume -= VOLUME_CHANGE_RATE;
	}

	if( volume < VOLUME_MIN ) volume = VOLUME_MIN;
	if( volume > VOLUME_MAX ) volume = VOLUME_MAX;

	printf( "Volume Change: %i\n", volume );

	unsigned long soundDev =__open("/dev/mixer", O_RDWR);
	if(soundDev)
	{
		int vol = ((volume << 8) | volume);
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
		close(soundDev);
	}
}

void GP2X_ButtonDown( int button )
{
	if( !volume_init )
	{
		GP2X_AdjustVolume(VOLUME_NOCHG);
		volume_init = 1;
	}

	switch( button )
	{
		case GP2X_BUTTON_UP:        intUp          = 1; break;
		case GP2X_BUTTON_DOWN:      intDown        = 1; break;
		case GP2X_BUTTON_RIGHT:     intRight       = 1; break;
		case GP2X_BUTTON_LEFT:      intLeft        = 1; break;
		case GP2X_BUTTON_UPRIGHT:   intUpRight     = 1; break;
		case GP2X_BUTTON_UPLEFT:    intUpLeft      = 1; break;
		case GP2X_BUTTON_DOWNRIGHT: intDownRight   = 1; break;
		case GP2X_BUTTON_DOWNLEFT:  intDownLeft    = 1; break;
		case GP2X_BUTTON_SELECT:    intButtonSel   = 1; break;
		case GP2X_BUTTON_START:     intButtonSrt   = 1; break;
		case GP2X_BUTTON_X:         intButtonX     = 1; LastASCII = 'x'; break;
		case GP2X_BUTTON_Y:         intButtonY     = 1; LastASCII = 'y'; break;
		case GP2X_BUTTON_A:         intButtonA     = 1; LastASCII = 'a'; break;
		case GP2X_BUTTON_B:         intButtonB     = 1; LastASCII = 'b'; break;
		case GP2X_BUTTON_R:         intButtonR     = 1; break;
		case GP2X_BUTTON_L:         intButtonL     = 1; break;
		case GP2X_BUTTON_VOLUP:     GP2X_AdjustVolume( VOLUME_UP   ); break;
		case GP2X_BUTTON_VOLDOWN:   GP2X_AdjustVolume( VOLUME_DOWN ); break;
		case GP2X_BUTTON_CLICK:     intButtonStick = 1; break;
	}

	if( intButtonL & intButtonR )
	{
		// Status Bar
		SetKeyboard( SDLK_TAB, KEY_DOWN );

		// Music Player (doesnt work, it appears the event's arnt happening soon enough)
		SetKeyboard( sc_M, KEY_DOWN );

		SetKeyboard( SDLK_LALT, KEY_UP );
		SetKeyboard( SDLK_LEFT, KEY_UP );
		SetKeyboard( SDLK_RIGHT, KEY_UP );
	}
	else if( intButtonL & !intButtonR )
	{
		// Strafe Left
		SetKeyboard( SDLK_LALT, KEY_DOWN );
		SetKeyboard( SDLK_LEFT, KEY_DOWN );
	}
	else if( intButtonR & !intButtonL )
	{
		// Strafe Right
		SetKeyboard( SDLK_LALT,  KEY_DOWN );
		SetKeyboard( SDLK_RIGHT, KEY_DOWN );
	}

	// Left Direction
	if( intLeft  | intDownLeft  | intUpLeft   )
	{
		// UNstrafe
		SetKeyboard( SDLK_LALT,  KEY_UP );
		SetKeyboard( SDLK_RIGHT, KEY_UP );
		// Turn
		SetKeyboard( SDLK_LEFT, KEY_DOWN );
	}

	// Right Direction
	if( intRight | intDownRight | intUpRight  )
	{
		// UNstrafe
		SetKeyboard( SDLK_LALT, KEY_UP );
		SetKeyboard( SDLK_LEFT, KEY_UP );
		// Turn
		SetKeyboard( SDLK_RIGHT, KEY_DOWN );
	}

	// Up Direction
	if( intUp    | intUpRight   | intUpLeft   ) {
		SetKeyboard( SDLK_UP, KEY_DOWN );
	}
	// Down Direction
	if( intDown  | intDownRight | intDownLeft ) {
		SetKeyboard( SDLK_DOWN, KEY_DOWN );
	}

	if( intButtonSel & intButtonSrt ) {
		// Pause
		SetKeyboard( SDLK_PAUSE, KEY_DOWN );
	}
	else if( intButtonL & intButtonSel ) {
		fpscounter ^= 1;    // Turn On FPS Counter
	}
	else if( intButtonL & intButtonSrt ) {
		Screenshot();
	}
	else if( intButtonSel & !intButtonSrt ) {
		// Escape
		SetKeyboard( SDLK_ESCAPE, KEY_DOWN );
	}
	else if( !intButtonSel & intButtonSrt ) {
		// Enter
		SetKeyboard( SDLK_RETURN, KEY_DOWN );
	}

	if( intButtonX   ) {
		// Shoot
		SetKeyboard( SDLK_LCTRL,  KEY_DOWN );
	}
	if( intButtonY   ) {
		// Yes
		SetKeyboard( SDLK_y,      KEY_DOWN );

		if( gamestate.chosenweapon == gamestate.bestweapon )
		{
			SetKeyboard( SDLK_1, KEY_DOWN );
		}
		else
		{
			SetKeyboard( SDLK_1 + gamestate.chosenweapon + 1, KEY_DOWN );
		}
	}
	if( intButtonA   ) {
		// Open
		SetKeyboard( SDLK_SPACE,  KEY_DOWN );
	}
	if( intButtonB   ) {
		// No
		SetKeyboard( SDLK_n,      KEY_DOWN );
		// Run
		SetKeyboard( SDLK_LSHIFT, KEY_DOWN );
	}
}

void GP2X_ButtonUp( int button )
{
	switch( button )
	{
		case GP2X_BUTTON_UP:        intUp          = 0; break;
		case GP2X_BUTTON_DOWN:      intDown        = 0; break;
		case GP2X_BUTTON_RIGHT:     intRight       = 0; break;
		case GP2X_BUTTON_LEFT:      intLeft        = 0; break;
		case GP2X_BUTTON_UPRIGHT:   intUpRight     = 0; break;
		case GP2X_BUTTON_UPLEFT:    intUpLeft      = 0; break;
		case GP2X_BUTTON_DOWNRIGHT: intDownRight   = 0; break;
		case GP2X_BUTTON_DOWNLEFT:  intDownLeft    = 0; break;
		case GP2X_BUTTON_SELECT:    intButtonSel   = 0; break;
		case GP2X_BUTTON_START:     intButtonSrt   = 0; break;
		case GP2X_BUTTON_X:         intButtonX     = 0; break;
		case GP2X_BUTTON_Y:         intButtonY     = 0; break;
		case GP2X_BUTTON_A:         intButtonA     = 0; break;
		case GP2X_BUTTON_B:         intButtonB     = 0; break;
		case GP2X_BUTTON_R:         intButtonR     = 0; break;
		case GP2X_BUTTON_L:         intButtonL     = 0; break;
		case GP2X_BUTTON_CLICK:     intButtonStick = 0; break;
	}

	if( !intButtonL | !intButtonR )
	{
		SetKeyboard( SDLK_TAB,  KEY_UP );
		SetKeyboard( sc_M,      KEY_UP );
		SetKeyboard( SDLK_LALT, KEY_UP );
	}

	if( !intLeft & !intDownLeft & !intUpLeft )
	{
		if( !intButtonL )
		{
			SetKeyboard( SDLK_LALT,  KEY_UP );
			SetKeyboard( SDLK_LEFT,  KEY_UP );
		}
		if( intButtonR )
		{
			SetKeyboard( SDLK_LALT,  KEY_DOWN );
			SetKeyboard( SDLK_RIGHT, KEY_DOWN );
		}
	}

	if( !intRight & !intDownRight & !intUpRight )
	{
		if( !intButtonR )
		{
			SetKeyboard( SDLK_LALT,   KEY_UP );
			SetKeyboard( SDLK_RIGHT,  KEY_UP );
		}
		if( intButtonL )
		{
			SetKeyboard( SDLK_LALT, KEY_DOWN );
			SetKeyboard( SDLK_LEFT, KEY_DOWN );
		}
	}

	if( !intUp    & !intUpRight   & !intUpLeft   ) {
		SetKeyboard( SDLK_UP,    KEY_UP );
	}
	if( !intDown  & !intDownRight & !intDownLeft ) {
		SetKeyboard( SDLK_DOWN,  KEY_UP );
	}

	if( !intButtonSel & !intButtonSrt ) {
		SetKeyboard( SDLK_PAUSE, KEY_UP );
	}
	if( !intButtonSel ) {
		SetKeyboard( SDLK_ESCAPE, KEY_UP );
	}
	if( !intButtonSrt ) {
		SetKeyboard( SDLK_RETURN, KEY_UP );
	}

	if( !intButtonX   ) {
		SetKeyboard( SDLK_LCTRL,  KEY_UP );
	}
	if( !intButtonY   ) {
		SetKeyboard( SDLK_y, KEY_UP );
		SetKeyboard( SDLK_1, KEY_UP );
		SetKeyboard( SDLK_2, KEY_UP );
		SetKeyboard( SDLK_3, KEY_UP );
		SetKeyboard( SDLK_4, KEY_UP );
	}
	if( !intButtonA   ) {
		SetKeyboard( SDLK_SPACE,  KEY_UP );
	}
	if( !intButtonB   ) {
		SetKeyboard( SDLK_n,      KEY_UP );
		SetKeyboard( SDLK_LSHIFT, KEY_UP );
	}
}

void Screenshot( void )
{
	char filename[32];

	snprintf( filename, sizeof(filename), "Screenshot_%i.bmp", screenshot_count );
	SDL_SaveBMP(screen, filename );
	screenshot_count++;
}

void SetKeyboard( unsigned int key, int press )
{
	// press = 1 = down, press = 0 = up
	if( press )
	{
		LastScan = key;
		Keyboard[key] = 1;
	}
	else
	{
		Keyboard[key] = 0;
	}
}

#endif // GP2X
