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
#include "CannotReequipState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Cannot Reequip screen.
 * @param game Pointer to the core game.
 * @param missingItems List of items still needed for reequip.
 */
CannotReequipState::CannotReequipState(Game *game, std::vector<ReequipStat> missingItems) : State(game)
{
	std::string background, backpalette;
	Uint8 colors[4];

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(120, 18, 100, 174);
	_txtTitle = new Text(220, 32, 50, 8);
	_txtItem = new Text(180, 9, 16, 50);
	_txtQuantity = new Text(60, 9, 130, 50);
	_txtCraft = new Text(50, 9, 200, 50);
	_lstItems = new TextList(288, 112, 8, 58);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		backpalette = "TFTD_BACKPALS.DAT";
		colors[0] = Palette::blockOffset(4);
		colors[1] = colors[2] = colors[3] = Palette::blockOffset(0)+1;
		background = "TFTD_BACK01.SCR";
	}
	else
	{
		// Basic properties for display in UFO style
		backpalette = "BACKPALS.DAT";
		colors[0] = Palette::blockOffset(0);
		colors[1] = Palette::blockOffset(15)-1;
		colors[2] = Palette::blockOffset(8)+5;
		colors[3] = Palette::blockOffset(8)+10;
		background = "BACK01.SCR";
	}

	// Set palette
	_game->getResourcePack()->getSurface(background)->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);
	add(_window);
	add(_btnOk);
	add(_txtTitle);
	add(_txtItem);
	add(_txtQuantity);
	add(_txtCraft);
	add(_lstItems);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnOk->setColor(colors[2]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CannotReequipState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CannotReequipState::btnOkClick, (SDLKey)Options::getInt("keyOk"));
	_btnOk->onKeyboardPress((ActionHandler)&CannotReequipState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));

	_txtTitle->setColor(colors[2]);
	_txtTitle->setText(_game->getLanguage()->getString("STR_NOT_ENOUGH_EQUIPMENT_TO_FULLY_RE_EQUIP_SQUAD"));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);

	_txtItem->setColor(colors[1]);
	_txtItem->setText(_game->getLanguage()->getString("STR_ITEM"));

	_txtQuantity->setColor(colors[1]);
	_txtQuantity->setText(_game->getLanguage()->getString("STR_QUANTITY_UC"));

	_txtCraft->setColor(colors[1]);
	_txtCraft->setText(_game->getLanguage()->getString("STR_CRAFT"));

	_lstItems->setColor(colors[3]);
	_lstItems->setColumns(3, 114, 70, 104);
	_lstItems->setSelectable(true);
	_lstItems->setBackground(_window);
	_lstItems->setMargin(8);

	for (std::vector<ReequipStat>::iterator i = missingItems.begin(); i != missingItems.end(); ++i)
	{
		std::wstringstream ss;
		ss << i->qty;
		_lstItems->addRow(3, _game->getLanguage()->getString(i->item).c_str(), ss.str().c_str(), i->craft.c_str());
	}
}

/**
 *
 */
CannotReequipState::~CannotReequipState()
{
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CannotReequipState::btnOkClick(Action *)
{
	_game->popState();
}

}
