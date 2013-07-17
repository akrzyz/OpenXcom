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
#include "NextTurnState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Engine/Options.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Engine/Action.h"
#include "../Savegame/SavedBattleGame.h"
#include "DebriefingState.h"
#include "../Interface/Cursor.h"
#include "../Interface/FpsCounter.h"
#include "BattlescapeState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Next Turn screen.
 * @param game Pointer to the core game.
 * @param battleGame Pointer to the saved game.
 * @param state Pointer to the Battlescape state.
 */
NextTurnState::NextTurnState(Game *game, SavedBattleGame *battleGame, BattlescapeState *state) : State(game), _battleGame(battleGame), _state(state)
{
	std::string background;
	Uint8 color;

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(320, 16, 0, 68);
	_txtTurn = new Text(320, 16, 0, 92);
	_txtSide = new Text(320, 16, 0, 108);
	_txtMessage = new Text(320, 16, 0, 132);

	add(_window);
	add(_txtTitle);
	add(_txtTurn);
	add(_txtSide);
	add(_txtMessage);

	centerAllSurfaces();

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_TAC00.SCR";

		color = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "TAC00.SCR";

		color = Palette::blockOffset(0);

		_window->setHighContrast(true);
		_txtTitle->setHighContrast(true);
		_txtTurn->setHighContrast(true);
		_txtSide->setHighContrast(true);
		_txtMessage->setHighContrast(true);
	}

	// Set up objects
	_window->setColor(color);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_txtTitle->setColor(color);
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(_game->getLanguage()->getString("STR_OPENXCOM"));

	_txtTurn->setColor(color);
	_txtTurn->setBig();
	_txtTurn->setAlign(ALIGN_CENTER);
	std::wstringstream ss;
	ss << _game->getLanguage()->getString("STR_TURN") << L" " << _battleGame->getTurn();
	_txtTurn->setText(ss.str());

	_txtSide->setColor(color);
	_txtSide->setBig();
	_txtSide->setAlign(ALIGN_CENTER);
	ss.str(L"");
	ss << _game->getLanguage()->getString("STR_SIDE") << _game->getLanguage()->getString((_battleGame->getSide() == FACTION_PLAYER?"STR_XCOM":"STR_ALIENS"));
	_txtSide->setText(ss.str());

	_txtMessage->setColor(color);
	_txtMessage->setBig();
	_txtMessage->setAlign(ALIGN_CENTER);
	_txtMessage->setText(_game->getLanguage()->getString("STR_PRESS_BUTTON_TO_CONTINUE"));

	_state->clearMouseScrollingState();
}

/**
 *
 */
NextTurnState::~NextTurnState()
{

}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void NextTurnState::handle(Action *action)
{
	State::handle(action);

	if (action->getDetails()->type == SDL_KEYDOWN || action->getDetails()->type == SDL_MOUSEBUTTONDOWN)
	{
		_game->popState();

		int liveAliens = 0;
		int liveSoldiers = 0;
		_state->getBattleGame()->tallyUnits(liveAliens, liveSoldiers, false);
		if (liveAliens == 0 || liveSoldiers == 0)
		{
			_state->finishBattle(false, liveSoldiers);
		}
		else
		{
			_state->btnCenterClick(0);
		}
	}
}

}
