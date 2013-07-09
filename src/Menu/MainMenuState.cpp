/*
 * Copyright 2010-2013 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MainMenuState.h"
#include <sstream>
#include "../version.h"
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Font.h"
#include "../Engine/Palette.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Engine/Music.h"
#include "NewGameState.h"
#include "NewBattleState.h"
#include "LoadState.h"
#include "OptionsState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Main Menu window.
 * @param game Pointer to the core game.
 */
MainMenuState::MainMenuState(Game *game) : State(game)
{
	std::string palette, backpalette, background;
	Uint8 colors[3];

	// Create objects
	_window = new Window(this, 256, 160, 32, 20, POPUP_BOTH);
	_btnNewGame = new TextButton(92, 20, 64, 90);
	_btnNewBattle = new TextButton(92, 20, 164, 90);
	_btnLoad = new TextButton(92, 20, 64, 118);
	_btnOptions = new TextButton(92, 20, 164, 118);
	_btnQuit = new TextButton(192, 20, 64, 146);
	_txtTitle = new Text(256, 30, 32, 45);

	if ( ( (_game->getResourcePack()->getPalette("TFTD_PALETTES.DAT_0") != 0) && (_game->getResourcePack()->getPalette("PALETTES.DAT_0") == 0) ) || ( (_game->getResourcePack()->getPalette("TFTD_PALETTES.DAT_0") != 0) && (Options::getString("GUIstyle") == "xcom2") ) )
	{
		// Basic properties for display in TFTD style
		Options::setString("GUIstyle", "xcom2");
		palette = "TFTD_PALETTES.DAT_0";
		backpalette = "TFTD_BACKPALS.DAT";
		colors[0] = colors[1] = Palette::blockOffset(0)+1;
		colors[2] = Palette::blockOffset(4);
		background = "TFTD_BACK01.SCR";
	}
	else
	{
		// Basic properties for display in UFO style
		Options::setString("GUIstyle", "xcom1");
		palette = "PALETTES.DAT_0";
		backpalette = "BACKPALS.DAT";
		colors[0] = Palette::blockOffset(8)+5;
		colors[1] = Palette::blockOffset(8)+10;
		colors[2] = Palette::blockOffset(0);
		background = "BACK01.SCR";
	}

	// Set palette
	_game->setPalette(_game->getResourcePack()->getPalette(palette)->getColors());
	_game->getResourcePack()->getSurface(background)->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[2]), Palette::backPos, 16);

	add(_window);
	add(_btnNewGame);
	add(_btnNewBattle);
	add(_btnLoad);
	add(_btnOptions);
	add(_btnQuit);
	add(_txtTitle);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[0]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnNewGame->setColor(colors[0]);
	_btnNewGame->setText(_game->getLanguage()->getString("STR_NEW_GAME"));
	_btnNewGame->onMouseClick((ActionHandler)&MainMenuState::btnNewGameClick);

	_btnNewBattle->setColor(colors[0]);
	_btnNewBattle->setText(_game->getLanguage()->getString("STR_NEW_BATTLE"));
	_btnNewBattle->onMouseClick((ActionHandler)&MainMenuState::btnNewBattleClick);

	_btnLoad->setColor(colors[0]);
	_btnLoad->setText(_game->getLanguage()->getString("STR_LOAD_SAVED_GAME"));
	_btnLoad->onMouseClick((ActionHandler)&MainMenuState::btnLoadClick);

	_btnOptions->setColor(colors[0]);
	_btnOptions->setText(_game->getLanguage()->getString("STR_OPTIONS"));
	_btnOptions->onMouseClick((ActionHandler)&MainMenuState::btnOptionsClick);

	_btnQuit->setColor(colors[0]);
	_btnQuit->setText(_game->getLanguage()->getString("STR_QUIT"));
	_btnQuit->onMouseClick((ActionHandler)&MainMenuState::btnQuitClick);

	_txtTitle->setColor(colors[1]);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	std::wstringstream title;
	title << _game->getLanguage()->getString("STR_OPENXCOM") << L"\x02";
	title << Language::utf8ToWstr(OPENXCOM_VERSION_SHORT) << Language::utf8ToWstr(OPENXCOM_VERSION_GIT);
	_txtTitle->setText(title.str());

	// Set music
	_game->getResourcePack()->getMusic("GMSTORY")->play();
}

/**
 *
 */
MainMenuState::~MainMenuState()
{

}

/**
 * Resets the palette
 * since it's bound to change on other screens.
 */
void MainMenuState::init()
{
	// Set palette
	if (_game->getResourcePack()->getPalette("BACKPALS.DAT"))
		_game->setPalette(_game->getResourcePack()->getPalette("BACKPALS.DAT")->getColors(Palette::blockOffset(0)), Palette::backPos, 16);
	else
		_game->setPalette(_game->getResourcePack()->getPalette("TFTD_BACKPALS.DAT")->getColors(Palette::blockOffset(0)), Palette::backPos, 16);
}

/**
 * Opens the New Game window.
 * @param action Pointer to an action.
 */
void MainMenuState::btnNewGameClick(Action *)
{
	_game->pushState(new NewGameState(_game));
}

/**
 * Opens the New Battle screen.
 * @param action Pointer to an action.
 */
void MainMenuState::btnNewBattleClick(Action *)
{
	_game->pushState(new NewBattleState(_game));
}

/**
 * Opens the Load Game screen.
 * @param action Pointer to an action.
 */
void MainMenuState::btnLoadClick(Action *)
{
	_game->pushState(new LoadState(_game, true));
}

/**
 * Opens the Options screen.
 * @param action Pointer to an action.
 */
void MainMenuState::btnOptionsClick(Action *)
{
	_game->pushState(new OptionsState(_game));
}

/**
 * Quits the game.
 * @param action Pointer to an action.
 */
void MainMenuState::btnQuitClick(Action *)
{
	_game->quit();
}

}
