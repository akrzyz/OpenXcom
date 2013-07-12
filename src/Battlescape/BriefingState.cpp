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
#include "BriefingState.h"
#include "BattlescapeState.h"
#include "../Engine/Game.h"
#include "../Engine/Language.h"
#include "../Engine/Music.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/Window.h"
#include "InventoryState.h"
#include "NextTurnState.h"
#include "../Resource/ResourcePack.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Ufo.h"
#include <sstream>
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Briefing screen.
 * @param game Pointer to the core game.
 * @param craft Pointer to the craft in the mission.
 * @param base Pointer to the base in the mission.
 */
BriefingState::BriefingState(Game *game, Craft *craft, Base *base) : State(game)
{
	std::string background, backpalette, palette;
	Uint8 colors[4];

	_screen = false;
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(120, 18, 100, 164);
	_txtTitle = new Text(300, 16, 16, 24);
	_txtTarget = new Text(300, 16, 16, 40);
	_txtCraft = new Text(300, 16, 16, 56);
	_txtBriefing = new Text(274, 64, 16, 72);

	std::string mission = _game->getSavedGame()->getSavedBattle()->getMissionType();
	
	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		palette = "TFTD_PALETTES.DAT_0";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(7);
		colors[1] = Palette::blockOffset(4);
		colors[2] = colors[3] = Palette::blockOffset(0)+1;

		if (mission == "STR_TERROR_MISSION")
		{
			background = "TFTD_BACK03.SCR";
		}
		else if (mission == "STR_ALIEN_BASE_ASSAULT" || mission == "STR_MARS_THE_FINAL_ASSAULT")
		{
			background = "TFTD_BACK01.SCR";
		}
		else
		{
			background = "TFTD_BACK16.SCR";
		}
	}	
	else
	{
		// Basic properties for display in UFO style
		palette = "PALETTES.DAT_0";
		backpalette = "BACKPALS.DAT";

		colors[0] = Palette::blockOffset(2);
		colors[1] = Palette::blockOffset(0);
		colors[2] = Palette::blockOffset(15)-1;
		colors[3] = Palette::blockOffset(8)+5;

		if (mission == "STR_ALIEN_BASE_ASSAULT" || mission == "STR_MARS_THE_FINAL_ASSAULT")
		{
			background = "BACK01.SCR";
		}
		else
		{
			background = "BACK16.SCR";
		}
	}

	_game->setPalette(_game->getResourcePack()->getPalette(palette)->getColors());

	// Set palette
	if (mission == "STR_TERROR_MISSION" || mission == "STR_BASE_DEFENSE")
	{
		_game->getResourcePack()->getSurface(background)->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);
		_game->getResourcePack()->getMusic("GMENBASE")->play();
	}
	else if (mission == "STR_MARS_CYDONIA_LANDING" || mission == "STR_MARS_THE_FINAL_ASSAULT")
	{
		_game->getResourcePack()->getSurface(background)->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[1]), Palette::backPos, 16);
		_game->getResourcePack()->getMusic("GMNEWMAR")->play();
	}
	else
	{
		_game->getResourcePack()->getSurface(background)->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[1]), Palette::backPos, 16);
		_game->getResourcePack()->getMusic("GMDEFEND")->play();
	}

	add(_window);
	add(_btnOk);
	add(_txtTitle);
	if (mission == "STR_ALIEN_BASE_ASSAULT" || mission == "STR_MARS_CYDONIA_LANDING")
	{
		_txtCraft->setY(40);
		_txtBriefing->setY(56);
	}
	else
	{
		add(_txtTarget);
	}
	if (mission != "STR_MARS_THE_FINAL_ASSAULT")
	{
		add(_txtCraft);
	}
	add(_txtBriefing);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[2]);

	_btnOk->setColor(colors[3]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BriefingState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingState::btnOkClick, (SDLKey)Options::getInt("keyOk"));
	_btnOk->onKeyboardPress((ActionHandler)&BriefingState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));

	_txtTitle->setColor(colors[3]);
	_txtTitle->setBig();

	_txtTarget->setColor(colors[3]);
	_txtTarget->setBig();
	
	_txtCraft->setColor(colors[3]);
	_txtCraft->setBig();
	std::wstringstream ss;
	if (craft)
	{
		if (craft->getDestination())
		{
			_txtTarget->setText(craft->getDestination()->getName(_game->getLanguage()));
		}

		ss << _game->getLanguage()->getString("STR_CRAFT_") << craft->getName(_game->getLanguage());
	}
	else if(base)
	{
		ss << _game->getLanguage()->getString("STR_BASE_UC_") << base->getName();
	}
	_txtCraft->setText(ss.str());

	_txtBriefing->setColor(colors[3]);
	_txtBriefing->setWordWrap(true);

	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_txtTitle->setText(_game->getLanguage()->getString(mission));
	std::stringstream briefingtext;
	briefingtext << mission.c_str() << "_BRIEFING";
	_txtBriefing->setText(_game->getLanguage()->getString(briefingtext.str()));

	if (mission == "STR_BASE_DEFENSE")
	{
		// And make sure the base is unmarked.
		base->setRetaliationTarget(false);
	}
}

/**
 *
 */
BriefingState::~BriefingState()
{

}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void BriefingState::btnOkClick(Action *)
{
	_game->popState();
	BattlescapeState *bs = new BattlescapeState(_game);
	_game->pushState(bs);
	_game->getSavedGame()->getSavedBattle()->setBattleState(bs);
	_game->pushState(new NextTurnState(_game, _game->getSavedGame()->getSavedBattle(), bs));
	_game->pushState(new InventoryState(_game, false, bs));
}

}
