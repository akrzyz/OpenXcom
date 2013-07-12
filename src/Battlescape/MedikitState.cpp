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
#include "MedikitState.h"
#include "MedikitView.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/Game.h"
#include "../Engine/Language.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/Action.h"
#include "../Engine/Palette.h"
#include "../Interface/Text.h"
#include "../Engine/Screen.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/BattleUnit.h"
#include "../Ruleset/RuleItem.h"
#include "../Resource/ResourcePack.h"
#include <iostream>
#include <sstream>
#include "../Engine/Options.h"

namespace OpenXcom
{
/**
 * Helper function return a string representation a type(mainly used for number)
 * t the value to stringify
 * @return a string representation of value
 */
template<typename type>
std::wstring toString (type t)
{
	std::wstringstream ss;
	ss << t;
	return ss.str();
}

/**
 * Helper class for medikit title
 */
class MedikitTitle : public Text
{
public:
	/// Create a medikit title
	MedikitTitle(int x, int y, const std::wstring & title);
};

/**
 * Initialize a Medikit title
 * @param y the Title y origin
 * @param title the title
 */
MedikitTitle::MedikitTitle (int x, int y, const std::wstring & title) : Text (60, 16, x, y)
{
	this->setText(title);
	this->setHighContrast(true);
	this->setAlign(ALIGN_CENTER);
}

/**
 * Helper class for medikit value
 */
class MedikitTxt : public Text
{
public:
	/// Create a medikit text
	MedikitTxt(int x, int y);
};

/**
 * Initialize a Medikit text
 * @param y the Text y origin
 */
MedikitTxt::MedikitTxt(int x, int y) : Text(30, 22, x, y)
{
	Uint8 color;

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		color = Palette::blockOffset(5);
	}
	else
	{
		// Basic properties for display in UFO style
		color = Palette::blockOffset(1);

		this->setHighContrast(true);
	}

	// Note: we can't set setBig here. The needed font is only setted when added to State
	this->setColor(color);
	this->setAlign(ALIGN_CENTER);
	this->setVerticalAlign(ALIGN_MIDDLE);
}

/**
 * Helper class for medikit button
 */
class MedikitButton : public InteractiveSurface
{
public:
	/// Create a medikit button
	MedikitButton(int x, int y);
};

/**
 * Initialize a Medikit button
 * @param y the button y origin
 */
MedikitButton::MedikitButton(int x, int y) : InteractiveSurface(30, 20, x, y)
{
}

/**
 * Initialize the Medikit State
 * @param game Pointer to the core game.
 * @param targetUnit The wounded unit.
 * @param action The healing action.
 */
MedikitState::MedikitState (Game * game, BattleUnit * targetUnit, BattleAction *action) : State (game), _targetUnit(targetUnit), _action(action)
{
	std::string background;
	Uint8 color;
	InteractiveSurface *endButton, *stimulantButton, *pkButton, *healButton;

	_unit = action->actor;
	_item = action->weapon;
	_surface = new InteractiveSurface(320, 200);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_MEDIBORD.BDY";

		color = Palette::blockOffset(5);

		_partTxt = new Text(50, 15, 85, 115);
		_woundTxt = new Text(10, 15, 140, 115);
		_medikitView = new MedikitView(52, 58, 90, 55, _game, _targetUnit, _partTxt, _woundTxt);
		endButton = new InteractiveSurface(20, 20, 220, 140);
		stimulantButton = new MedikitButton(187, 80);
		pkButton = new MedikitButton(187, 44);
		healButton = new MedikitButton(187, 116);
		_pkText = new MedikitTxt (215, 46);
		_stimulantTxt = new MedikitTxt (215, 81);
		_healTxt = new MedikitTxt (215, 116);
	}
	else
	{
		// Basic properties for display in UFO style
		background = "MEDIBORD.PCK";

		color = Palette::blockOffset(2);

		_partTxt = new Text(50, 15, 90, 120);
		_woundTxt = new Text(10, 15, 145, 120);
		_medikitView = new MedikitView(52, 58, 95, 60, _game, _targetUnit, _partTxt, _woundTxt);
		endButton = new InteractiveSurface(20, 20, 220, 140);
		stimulantButton = new MedikitButton(190, 84);
		pkButton = new MedikitButton(190, 48);
		healButton = new MedikitButton(190, 120);
		_pkText = new MedikitTxt (220, 50);
		_stimulantTxt = new MedikitTxt (220, 85);
		_healTxt = new MedikitTxt (220, 120);
	}

	if (Screen::getDY() > 50)
	{
		_screen = false;
		SDL_Rect current;
		current.w = 190;
		current.h = 100;
		current.x = 67;
		current.y = 44;
		_surface->drawRect(&current, Palette::blockOffset(15)+15);
	}
	add(_surface);
	add(_medikitView);
	add(endButton);
	add(healButton);
	add(stimulantButton);
	add(pkButton);
	add(_pkText);
	add(_stimulantTxt);
	add(_healTxt);
	add(_partTxt);
	add(_woundTxt);

	centerAllSurfaces();

	if (Options::getString("GUIstyle") == "xcom2")
	{
		add(new MedikitTitle (187, 33, _game->getLanguage()->getString("STR_PAIN_KILLER")));
		add(new MedikitTitle (187, 69, _game->getLanguage()->getString("STR_STIMULANT")));
		add(new MedikitTitle (187, 105, _game->getLanguage()->getString("STR_HEAL")));
	}
	else
	{
		add(new MedikitTitle (192, 37, _game->getLanguage()->getString("STR_PAIN_KILLER")));
		add(new MedikitTitle (192, 73, _game->getLanguage()->getString("STR_STIMULANT")));
		add(new MedikitTitle (192, 109, _game->getLanguage()->getString("STR_HEAL")));
	}

	_game->getResourcePack()->getSurface(background)->blit(_surface);
	_pkText->setBig();
	_stimulantTxt->setBig();
	_healTxt->setBig();
	_partTxt->setColor(color);
	_partTxt->setHighContrast(true);
	_woundTxt->setColor(color);
	_woundTxt->setHighContrast(true);
	endButton->onMouseClick((ActionHandler)&MedikitState::onEndClick);
	endButton->onKeyboardPress((ActionHandler)&MedikitState::onEndClick, (SDLKey)Options::getInt("keyCancel"));
	healButton->onMouseClick((ActionHandler)&MedikitState::onHealClick);
	stimulantButton->onMouseClick((ActionHandler)&MedikitState::onStimulantClick);
	pkButton->onMouseClick((ActionHandler)&MedikitState::onPainKillerClick);
	update();
}

/**
 * Closes the window on right-click.
 * @param action Pointer to an action.
 */
void MedikitState::handle(Action *action)
{
	State::handle(action);
	if (action->getDetails()->type == SDL_MOUSEBUTTONDOWN && action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->popState();
	}
}

/**
 * return to the previous screen
 * @param action Pointer to an action.
 */
void MedikitState::onEndClick(Action *)
{
	_game->popState();
}

/**
 * Handler for clicking on the heal button
 * @param action Pointer to an action.
 */
void MedikitState::onHealClick(Action *)
{
	int heal = _item->getHealQuantity();
	RuleItem *rule = _item->getRules();
	if (heal == 0)
	{
		return;
	}
	if (_unit->spendTimeUnits (rule->getTUUse()))
	{
		_targetUnit->heal(_medikitView->getSelectedPart(), rule->getHealAmount(), rule->getHealthAmount());
		_item->setHealQuantity(--heal);
		_medikitView->invalidate();
		update();
	}
	else
	{
		_action->result = "STR_NOT_ENOUGH_TIME_UNITS";
		_game->popState();
	}
}

/**
 * Handler for clicking on the stimulant button
 * @param action Pointer to an action.
 */
void MedikitState::onStimulantClick(Action *)
{
	int stimulant = _item->getStimulantQuantity();
	RuleItem *rule = _item->getRules();
	if (stimulant == 0)
	{
		return;
	}
	if (_unit->spendTimeUnits (rule->getTUUse()))
	{
		_targetUnit->stimulant(rule->getEnergy(), rule->getStun());
		_item->setStimulantQuantity(--stimulant);
		update();

		// if the unit has revived we quit this screen automatically
		if (_targetUnit->getStatus() == STATUS_UNCONSCIOUS && _targetUnit->getStunlevel() < _targetUnit->getHealth() && _targetUnit->getHealth() > 0)
		{
			_game->popState();
		}
	}
	else
	{
		_action->result = "STR_NOT_ENOUGH_TIME_UNITS";
		_game->popState();
	}
}

/**
 * Handler for clicking on the pain killer button
 * @param action Pointer to an action.
 */
void MedikitState::onPainKillerClick(Action *)
{
	int pk = _item->getPainKillerQuantity();
	RuleItem *rule = _item->getRules();
	if (pk == 0)
	{
		return;
	}
	if (_unit->spendTimeUnits (rule->getTUUse()))
	{
		_item->setPainKillerQuantity(--pk);
		update();
	}
	else
	{
		_action->result = "STR_NOT_ENOUGH_TIME_UNITS";
		_game->popState();
	}
}

/**
 * Update medikit state
 */
void MedikitState::update()
{
	_pkText->setText(toString(_item->getPainKillerQuantity()));
	_stimulantTxt->setText(toString(_item->getStimulantQuantity()));
	_healTxt->setText(toString(_item->getHealQuantity()));
}

}
