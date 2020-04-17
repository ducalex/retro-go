
#include "PCEngine.h"


// Check if a game was asked
/*
 * \return non zero if a game must be played
 */
int
game_asked()
{
	return ((CD_emulation == 1) || (strcmp(cart_name, "")));
}


int
main(int argc, char *argv[])
{
	// Create a new PC Engine
	PCEngine* engine = new PCEngine();

	if (!engine->isReady()) {
		MESSAGE_ERROR("PC Engine is not ready!\n");
		return 1;
	}

	// Read the command line
	if (parse_commandline(argc, argv) != 0) {
		delete engine;
		return 1;
	}

	MESSAGE_INFO("HuExpress, the multi-platform PCEngine emulator\n");
	MESSAGE_INFO("Version %d.%d.%d\n",
		VERSION_MAJOR, VERSION_MINOR, VERSION_UPDATE);

	if (game_asked()) {
		engine->LoadFile(cart_name);
		engine->Run();
	} else {
		MESSAGE_ERROR("No game specified\n");
	}

	delete engine;
	return 0;
}
