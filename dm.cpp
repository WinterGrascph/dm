#include "common/scummsys.h"
#include "common/system.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/events.h"

#include "dm/dm.h"
#include "gfx.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "champion.h"
#include "loadsave.h"
#include "objectman.h"

namespace DM {

int8 dirIntoStepCountEast[4] = {0 /* North */, 1 /* East */, 0 /* West */, -1 /* South */};
int8 dirIntoStepCountNorth[4] = {-1 /* North */, 0 /* East */, 1 /* West */, 0 /* South */};

void turnDirRight(direction &dir) { dir = (direction)((dir + 1) & 3); }
void turnDirLeft(direction &dir) { dir = (direction)((dir - 1) & 3); }
bool isOrientedWestEast(direction dir) { return dir & 1; }


DMEngine::DMEngine(OSystem *syst) : Engine(syst), _console(nullptr) {
	// Do not load data files
	// Do not initialize graphics here
	// Do not initialize audio devices here
	// Do these from run

	//Specify all default directories
	//const Common::FSNode gameDataDir(ConfMan.get("example"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "example2");
	DebugMan.addDebugChannel(kDMDebugExample, "example", "example desc");

	// register random source
	_rnd = new Common::RandomSource("quux");

	_dungeonMan = nullptr;
	_displayMan = nullptr;
	_eventMan = nullptr;
	_menuMan = nullptr;
	_championMan = nullptr;
	_loadsaveMan = nullptr;
	_objectMan = nullptr;
	_stopWaitingForPlayerInput = false;
	_gameTimeTicking = false;

	debug("DMEngine::DMEngine");
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;
	delete _console;
	delete _displayMan;
	delete _dungeonMan;
	delete _eventMan;
	delete _menuMan;
	delete _championMan;
	delete _loadsaveMan;
	delete _objectMan;

	// clear debug channels
	DebugMan.clearAllDebugChannels();
}

void DMEngine::initializeGame() {
	_displayMan->loadGraphics();
	// DUMMY CODE: next line
	_displayMan->loadPalette(gPalCredits);

	_eventMan->initMouse();

	while (_loadsaveMan->loadgame() != kLoadgameSuccess) {
		warning("TODO: F0441_STARTEND_ProcessEntrance");
	}

	_displayMan->loadFloorSet(kFloorSetStone);
	_displayMan->loadWallSet(kWallSetStone);

	startGame();
	warning("MISSING CODE: F0267_MOVE_GetMoveResult_CPSCE (if newGame)");
	_eventMan->showMouse(true);
	warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
}


void DMEngine::startGame() {
	_pressingEye = false;
	_stopPressingEye = false;
	_pressingMouth = false;
	_stopPressingMouth = false;
	_highlightBoxInversionRequested = false;
	_eventMan->_highlightBoxEnabled = false;
	_championMan->_partyIsSleeping = false;
	_championMan->_actingChampionOrdinal = indexToOrdinal(kChampionNone);
	_menuMan->_actionAreaContainsIcons = true;
	_eventMan->_useChampionIconOrdinalAsMousePointerBitmap = indexToOrdinal(kChampionNone);

	_eventMan->_primaryMouseInput = gPrimaryMouseInput_Interface;
	_eventMan->_secondaryMouseInput = gSecondaryMouseInput_Movement;
	warning("MISSING CODE: set primary/secondary keyboard input");

	processNewPartyMap(_dungeonMan->_currMap._currPartyMapIndex);

	if (!_dungeonMan->_messages._newGame) {
		warning("TODO: loading game");
	} else {
		_displayMan->_useByteBoxCoordinates = false;
		warning("TODO: clear screen");
	}

	warning("TODO: build copper");
	_menuMan->drawMovementArrows();
	_championMan->resetDataToStartGame();
	_gameTimeTicking = true;
}

void DMEngine::processNewPartyMap(uint16 mapIndex) {
	warning("MISSING CODE: F0194_GROUP_RemoveAllActiveGroups");
	_dungeonMan->setCurrentMapAndPartyMap(mapIndex);
	_displayMan->loadCurrentMapGraphics();
	warning("MISSING CODE: F0195_GROUP_AddAllActiveGroups");
	warning("MISSING CODE: F0337_INVENTORY_SetDungeonViewPalette");
}

Common::Error DMEngine::run() {
	// scummvm/engine specific
	initGraphics(320, 200, false);
	_console = new Console(this);
	_displayMan = new DisplayMan(this);
	_dungeonMan = new DungeonMan(this);
	_eventMan = new EventManager(this);
	_menuMan = new MenuMan(this);
	_championMan = new ChampionMan(this);
	_loadsaveMan = new LoadsaveMan(this);
	_objectMan = new ObjectMan(this);
	_displayMan->setUpScreens(320, 200);

	initializeGame(); // @ F0463_START_InitializeGame_CPSADEF
	while (true) {
		gameloop();
		warning("TODO: F0444_STARTEND_Endgame(G0303_B_PartyDead);");
	}

	return Common::kNoError;
}

void DMEngine::gameloop() {
	while (true) {
		_stopWaitingForPlayerInput = false;
		//do {
		_eventMan->processInput();
		_eventMan->processCommandQueue();
		//} while (!_stopWaitingForPlayerInput || !_gameTimeTicking);

		_displayMan->clearScreen(kColorBlack);
		_displayMan->drawDungeon(_dungeonMan->_currMap._partyDir, _dungeonMan->_currMap._partyPosX, _dungeonMan->_currMap._partyPosY);
		// DUMMY CODE: next line
		_menuMan->drawMovementArrows();
		_displayMan->updateScreen();
		_system->delayMillis(10);
	}
}

} // End of namespace DM
