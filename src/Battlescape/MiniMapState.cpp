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
#include "MiniMapState.h"
#include <iostream>
#include <sstream>
#include "../Engine/Game.h"
#include "../Engine/Screen.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/SurfaceSet.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Palette.h"
#include "../Engine/CrossPlatform.h"
#include "../Savegame/SavedGame.h"
#include "../Ruleset/MapDataSet.h"
#include "../Interface/Text.h"
#include "../Interface/ImageButton.h"
#include "MiniMapView.h"
#include "Map.h"
#include "Camera.h"
#include "../Engine/Timer.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the MiniMapState screen.
 * @param game Pointer to the core game.
 * @param camera The Battlescape camera.
 * @param battleGame The Battlescape save.
*/
MiniMapState::MiniMapState (Game * game, Camera * camera, SavedBattleGame * battleGame, SDL_Color *palette) : State(game)
{
	std::string background;
	std::wstringstream s;
	Uint8 colors[2];
	int mapHeight, mapX, mapY, textX, textY;

	InteractiveSurface *btnLvlUp, *btnLvlDwn, *btnOk;
	ImageButton *btnLvlUpTFTD, *btnLvlDwnTFTD, *btnOkTFTD;

	_surface = new InteractiveSurface(320, 200);

	if (Screen::getDY() > 50)
	{
		_screen = false;
		SDL_Rect current;
		current.w = 223;
		current.h = 151;
		current.x = 46;
		current.y = 14;
		_surface->drawRect(&current, colors[0]);
	}

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_SCANBORD.BDY";

		btnLvlUpTFTD = new ImageButton(24, 12, 280, 98);
		btnLvlDwnTFTD = new ImageButton(24, 12, 280, 115);
		btnOkTFTD = new ImageButton(24, 12, 280, 144);

		colors[0] = Palette::blockOffset(15)+15;
		colors[1] = Palette::blockOffset(5);

		mapHeight = 148;
		mapX = 47;
		mapY = 16;
		textX = 282;
		textY = 28;

		s << "L";
	}
	else
	{
		// Basic properties for display in UFO style
		background = "SCANBORD.PCK";

		colors[0] = Palette::blockOffset(15)+15;
		colors[1] = Palette::blockOffset(4);

		mapHeight = 150;
		mapX = 49;
		mapY = 15;
		textX = 281;
		textY = 75;

		btnLvlUp = new InteractiveSurface(18, 20, 24, 62);
		btnLvlDwn = new InteractiveSurface(18, 20, 24, 88);
		btnOk = new InteractiveSurface(32, 32, 275, 145);

		add(btnLvlUp);
		add(btnLvlDwn);
		add(btnOk);

		btnLvlUp->onMouseClick((ActionHandler)&MiniMapState::btnLevelUpClick);
		btnLvlDwn->onMouseClick((ActionHandler)&MiniMapState::btnLevelDownClick);
		btnOk->onMouseClick((ActionHandler)&MiniMapState::btnOkClick);
		btnOk->onKeyboardPress((ActionHandler)&MiniMapState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));
	}

	_miniMapView = new MiniMapView(222, mapHeight, mapX, mapY, 32, game, camera, battleGame, palette);
	_txtLevel = new Text (20, 25, textX, textY);

	add(_surface);
	add(_miniMapView);
	add(_txtLevel);

	centerAllSurfaces();

	_txtLevel->setBig();
	_txtLevel->setColor(colors[1]);
	s << camera->getViewLevel();
	_txtLevel->setText(s.str());
	_timerAnimate = new Timer(125);
	_timerAnimate->onTimer((StateHandler)&MiniMapState::animate);
	_timerAnimate->start();
	_miniMapView->draw();
	_game->getResourcePack()->getSurface(background)->blit(_surface);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		add(btnLvlUpTFTD);
		add(btnLvlDwnTFTD);
		add(btnOkTFTD);

		btnLvlUpTFTD->copy(_game->getResourcePack()->getSurface("TFTD_SCANBORD.BDY"));
		btnLvlUpTFTD->setColor(Palette::blockOffset(0)+6);
		btnLvlDwnTFTD->copy(_game->getResourcePack()->getSurface("TFTD_SCANBORD.BDY"));
		btnLvlDwnTFTD->setColor(Palette::blockOffset(0)+6);
		btnOkTFTD->copy(_game->getResourcePack()->getSurface("TFTD_SCANBORD.BDY"));
		btnOkTFTD->setColor(Palette::blockOffset(0)+6);

		btnLvlUpTFTD->onMouseClick((ActionHandler)&MiniMapState::btnLevelUpClick);
		btnLvlDwnTFTD->onMouseClick((ActionHandler)&MiniMapState::btnLevelDownClick);
		btnOkTFTD->onMouseClick((ActionHandler)&MiniMapState::btnOkClick);
		btnOkTFTD->onKeyboardPress((ActionHandler)&MiniMapState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));
	}
	else
	{
		_txtLevel->setHighContrast(true);
	}
}

/**
 *
 */
MiniMapState::~MiniMapState()
{
	delete _timerAnimate;
}

/**
 * Handles mouse-wheeling.
 * @param action Pointer to an action.
 */
void MiniMapState::handle(Action *action)
{
	State::handle(action);
	if (action->getDetails()->type == SDL_MOUSEBUTTONDOWN)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
		{
			btnLevelUpClick(action);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
		{
			btnLevelDownClick(action);
		}
	}
}

/**
 * return to the previous screen
 * @param action Pointer to an action.
 */
void MiniMapState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Change the currently displayed minimap level
 * @param action Pointer to an action.
 */
void MiniMapState::btnLevelUpClick(Action *)
{
	std::wstringstream s;
	if (Options::getString("GUIstyle") == "xcom2")
		s << "L";
	s << _miniMapView->up();
	_txtLevel->setText(s.str());
}

/**
 * Change the currently displayed minimap level
 * @param action Pointer to an action.
 */
void MiniMapState::btnLevelDownClick(Action *)
{
	std::wstringstream s;
	if (Options::getString("GUIstyle") == "xcom2")
		s << "L";
	s << _miniMapView->down ();
	_txtLevel->setText(s.str());
}

/**
 * Animation handler. Update the minimap view animation
*/
void MiniMapState::animate()
{
	_miniMapView->animate();
}

/**
 * Handle timers
*/
void MiniMapState::think ()
{
	State::think();
	_timerAnimate->think(this, 0);
}
}
