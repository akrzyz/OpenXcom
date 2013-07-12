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
#include "UnitInfoState.h"
#include <sstream>
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/Bar.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Engine/Surface.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Ruleset/RuleCraft.h"
#include "../Ruleset/Armor.h"
#include "../Savegame/Soldier.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Options.h"
#include "BattlescapeGame.h"
#include "BattlescapeState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Unit Info screen.
 * @param game Pointer to the core game.
 * @param unit Pointer to the selected unit.
 */
UnitInfoState::UnitInfoState(Game *game, BattleUnit *unit) : State(game), _unit(unit)
{
	std::string background;
	Uint8 colors[15];

	// Create objects
	_bg = new Surface(320, 200, 0, 0);
	_txtName = new Text(312, 192, 4, 4);

	_txtTimeUnits = new Text(120, 9, 8, 31);
	_numTimeUnits = new Text(18, 9, 150, 31);
	_barTimeUnits = new Bar(170, 5, 170, 32);

	_txtEnergy = new Text(120, 9, 8, 41);
	_numEnergy = new Text(18, 9, 150, 41);
	_barEnergy = new Bar(170, 5, 170, 42);

	_txtHealth = new Text(120, 9, 8, 51);
	_numHealth = new Text(18, 9, 150, 51);
	_barHealth = new Bar(170, 5, 170, 52);

	_txtFatalWounds = new Text(120, 9, 8, 61);
	_numFatalWounds = new Text(18, 9, 150, 61);
	_barFatalWounds = new Bar(170, 5, 170, 62);

	_txtBravery = new Text(120, 9, 8, 71);
	_numBravery = new Text(18, 9, 150, 71);
	_barBravery = new Bar(170, 5, 170, 72);

	_txtMorale = new Text(120, 9, 8, 81);
	_numMorale = new Text(18, 9, 150, 81);
	_barMorale = new Bar(170, 5, 170, 82);

	_txtReactions = new Text(120, 9, 8, 91);
	_numReactions = new Text(18, 9, 150, 91);
	_barReactions = new Bar(170, 5, 170, 92);

	_txtFiring = new Text(120, 9, 8, 101);
	_numFiring = new Text(18, 9, 150, 101);
	_barFiring = new Bar(170, 5, 170, 102);

	_txtThrowing = new Text(120, 9, 8, 111);
	_numThrowing = new Text(18, 9, 150, 111);
	_barThrowing = new Bar(170, 5, 170, 112);

	_txtStrength = new Text(120, 9, 8, 121);
	_numStrength = new Text(18, 9, 150, 121);
	_barStrength = new Bar(170, 5, 170, 122);

	_txtPsiStrength = new Text(120, 9, 8, 131);
	_numPsiStrength = new Text(18, 9, 150, 131);
	_barPsiStrength = new Bar(170, 5, 170, 132);

	_txtPsiSkill = new Text(120, 9, 8, 141);
	_numPsiSkill = new Text(18, 9, 150, 141);
	_barPsiSkill = new Bar(170, 5, 170, 142);

	_txtFrontArmor = new Text(120, 9, 8, 151);
	_numFrontArmor= new Text(18, 9, 150, 151);
	_barFrontArmor = new Bar(170, 5, 170, 152);

	_txtLeftArmor = new Text(120, 9, 8, 161);
	_numLeftArmor = new Text(18, 9, 150, 161);
	_barLeftArmor = new Bar(170, 5, 170, 162);

	_txtRightArmor = new Text(120, 9, 8, 171);
	_numRightArmor = new Text(18, 9, 150, 171);
	_barRightArmor = new Bar(170, 5, 170, 172);

	_txtRearArmor = new Text(120, 9, 8, 181);
	_numRearArmor = new Text(18, 9, 150, 181);
	_barRearArmor = new Bar(170, 5, 170, 182);

	_txtUnderArmor = new Text(120, 9, 8, 191);
	_numUnderArmor = new Text(18, 9, 150, 191);
	_barUnderArmor = new Bar(170, 5, 170, 192);

	add(_bg);
	add(_txtName);

	add(_txtTimeUnits);
	add(_numTimeUnits);
	add(_barTimeUnits);

	add(_txtEnergy);
	add(_numEnergy);
	add(_barEnergy);

	add(_txtHealth);
	add(_numHealth);
	add(_barHealth);

	add(_txtFatalWounds);
	add(_numFatalWounds);
	add(_barFatalWounds);

	add(_txtBravery);
	add(_numBravery);
	add(_barBravery);

	add(_txtMorale);
	add(_numMorale);
	add(_barMorale);

	add(_txtReactions);
	add(_numReactions);
	add(_barReactions);

	add(_txtFiring);
	add(_numFiring);
	add(_barFiring);

	add(_txtThrowing);
	add(_numThrowing);
	add(_barThrowing);

	add(_txtStrength);
	add(_numStrength);
	add(_barStrength);

	add(_txtPsiStrength);
	add(_numPsiStrength);
	add(_barPsiStrength);

	add(_txtPsiSkill);
	add(_numPsiSkill);
	add(_barPsiSkill);

	add(_txtFrontArmor);
	add(_numFrontArmor);
	add(_barFrontArmor);

	add(_txtLeftArmor);
	add(_numLeftArmor);
	add(_barLeftArmor);

	add(_txtRightArmor);
	add(_numRightArmor);
	add(_barRightArmor);

	add(_txtRearArmor);
	add(_numRearArmor);
	add(_barRearArmor);

	add(_txtUnderArmor);
	add(_numUnderArmor);
	add(_barUnderArmor);

	centerAllSurfaces();

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_UNIBORD.PCK";

		colors[0] = colors[1] = Palette::blockOffset(0)+1;
		colors[2] = colors[14] = Palette::blockOffset(6);
		colors[3] = Palette::blockOffset(0)+9;
		colors[4] = Palette::blockOffset(0)+5;
		colors[5] = colors[11] = Palette::blockOffset(2);
		colors[6] = Palette::blockOffset(14);
		colors[7] = Palette::blockOffset(1);
		colors[8] = Palette::blockOffset(8);
		colors[9] = Palette::blockOffset(6);
		colors[10] = colors[12] = Palette::blockOffset(5);
		colors[13] = Palette::blockOffset(10);
	}
	else
	{
		// Basic properties for display in UFO style
		background = "UNIBORD.PCK";

		colors[0] = colors[12] = Palette::blockOffset(4);
		colors[1] = colors[6] = colors[11] = Palette::blockOffset(3);
		colors[2] = colors[7] = colors[13] = colors[14] = Palette::blockOffset(9);
		colors[3] = Palette::blockOffset(2);
		colors[4] = Palette::blockOffset(5)+2;
		colors[5] = Palette::blockOffset(12);
		colors[8] = Palette::blockOffset(8);
		colors[9] = Palette::blockOffset(6);
		colors[10] = Palette::blockOffset(5);

		_txtName->setHighContrast(true);
		_txtTimeUnits->setHighContrast(true);
		_txtEnergy->setHighContrast(true);
		_txtHealth->setHighContrast(true);
		_txtFatalWounds->setHighContrast(true);
		_txtBravery->setHighContrast(true);
		_txtMorale->setHighContrast(true);
		_txtReactions->setHighContrast(true);
		_txtFiring->setHighContrast(true);
		_txtThrowing->setHighContrast(true);
		_txtStrength->setHighContrast(true);
		_txtPsiStrength->setHighContrast(true);
		_txtPsiSkill->setHighContrast(true);
		_txtFrontArmor->setHighContrast(true);
		_txtLeftArmor->setHighContrast(true);
		_txtRightArmor->setHighContrast(true);
		_txtRearArmor->setHighContrast(true);
		_txtUnderArmor->setHighContrast(true);
	}

	// Set up objects
	_game->getResourcePack()->getSurface(background)->blit(_bg);

	_txtName->setAlign(ALIGN_CENTER);
	_txtName->setBig();
	_txtName->setColor(colors[0]);

	_txtTimeUnits->setColor(colors[1]);
	_txtTimeUnits->setText(_game->getLanguage()->getString("STR_TIME_UNITS"));

	_numTimeUnits->setColor(colors[2]);
	_numTimeUnits->setHighContrast(true);

	_barTimeUnits->setColor(colors[12]);
	_barTimeUnits->setScale(1.0);

	_txtEnergy->setColor(colors[1]);
	_txtEnergy->setText(_game->getLanguage()->getString("STR_ENERGY"));

	_numEnergy->setColor(colors[14]);
	_numEnergy->setHighContrast(true);

	_barEnergy->setColor(colors[13]);
	_barEnergy->setScale(1.0);

	_txtHealth->setColor(colors[1]);
	_txtHealth->setText(_game->getLanguage()->getString("STR_HEALTH"));

	_numHealth->setColor(colors[2]);
	_numHealth->setHighContrast(true);

	_barHealth->setColor(colors[3]);
	_barHealth->setColor2(colors[4]);
	_barHealth->setScale(1.0);

	_txtFatalWounds->setColor(colors[1]);
	_txtFatalWounds->setText(_game->getLanguage()->getString("STR_FATAL_WOUNDS"));

	_numFatalWounds->setColor(colors[2]);
	_numFatalWounds->setHighContrast(true);

	_barFatalWounds->setColor(colors[3]);
	_barFatalWounds->setScale(1.0);

	_txtBravery->setColor(colors[1]);
	_txtBravery->setText(_game->getLanguage()->getString("STR_BRAVERY"));

	_numBravery->setColor(colors[2]);
	_numBravery->setHighContrast(true);

	_barBravery->setColor(colors[5]);
	_barBravery->setScale(1.0);

	_txtMorale->setColor(colors[1]);
	_txtMorale->setText(_game->getLanguage()->getString("STR_MORALE"));

	_numMorale->setColor(colors[2]);
	_numMorale->setHighContrast(true);

	_barMorale->setColor(colors[6]);
	_barMorale->setScale(1.0);

	_txtReactions->setColor(colors[1]);
	_txtReactions->setText(_game->getLanguage()->getString("STR_REACTIONS"));

	_numReactions->setColor(colors[2]);
	_numReactions->setHighContrast(true);

	_barReactions->setColor(colors[7]);
	_barReactions->setScale(1.0);

	_txtFiring->setColor(colors[1]);
	_txtFiring->setText(_game->getLanguage()->getString("STR_FIRING_ACCURACY"));

	_numFiring->setColor(colors[2]);
	_numFiring->setHighContrast(true);

	_barFiring->setColor(colors[8]);
	_barFiring->setScale(1.0);

	_txtThrowing->setColor(colors[1]);
	_txtThrowing->setText(_game->getLanguage()->getString("STR_THROWING_ACCURACY"));

	_numThrowing->setColor(colors[2]);
	_numThrowing->setHighContrast(true);

	_barThrowing->setColor(colors[9]);
	_barThrowing->setScale(1.0);

	_txtStrength->setColor(colors[1]);
	_txtStrength->setText(_game->getLanguage()->getString("STR_STRENGTH"));

	_numStrength->setColor(colors[2]);
	_numStrength->setHighContrast(true);

	_barStrength->setColor(colors[11]);
	_barStrength->setScale(1.0);

	_txtPsiStrength->setColor(colors[1]);
	_txtPsiStrength->setText(_game->getLanguage()->getString("STR_PSIONIC_STRENGTH"));

	_numPsiStrength->setColor(colors[2]);
	_numPsiStrength->setHighContrast(true);

	_barPsiStrength->setColor(colors[5]);
	_barPsiStrength->setScale(1.0);

	_txtPsiSkill->setColor(colors[1]);
	_txtPsiSkill->setText(_game->getLanguage()->getString("STR_PSIONIC_SKILL"));

	_numPsiSkill->setColor(colors[2]);
	_numPsiSkill->setHighContrast(true);

	_barPsiSkill->setColor(colors[5]);
	_barPsiSkill->setScale(1.0);

	_txtFrontArmor->setColor(colors[1]);
	_txtFrontArmor->setText(_game->getLanguage()->getString("STR_FRONT_ARMOR_UC"));

	_numFrontArmor->setColor(colors[2]);
	_numFrontArmor->setHighContrast(true);

	_barFrontArmor->setColor(colors[10]);
	_barFrontArmor->setScale(1.0);

	_txtLeftArmor->setColor(colors[1]);
	_txtLeftArmor->setText(_game->getLanguage()->getString("STR_LEFT_ARMOR_UC"));

	_numLeftArmor->setColor(colors[2]);
	_numLeftArmor->setHighContrast(true);

	_barLeftArmor->setColor(colors[10]);
	_barLeftArmor->setScale(1.0);

	_txtRightArmor->setColor(colors[1]);
	_txtRightArmor->setText(_game->getLanguage()->getString("STR_RIGHT_ARMOR_UC"));

	_numRightArmor->setColor(colors[2]);
	_numRightArmor->setHighContrast(true);

	_barRightArmor->setColor(colors[10]);
	_barRightArmor->setScale(1.0);

	_txtRearArmor->setColor(colors[1]);
	_txtRearArmor->setText(_game->getLanguage()->getString("STR_REAR_ARMOR_UC"));

	_numRearArmor->setColor(colors[2]);
	_numRearArmor->setHighContrast(true);

	_barRearArmor->setColor(colors[10]);
	_barRearArmor->setScale(1.0);

	_txtUnderArmor->setColor(colors[1]);
	_txtUnderArmor->setText(_game->getLanguage()->getString("STR_UNDER_ARMOR_UC"));

	_numUnderArmor->setColor(colors[2]);
	_numUnderArmor->setHighContrast(true);

	_barUnderArmor->setColor(colors[10]);
	_barUnderArmor->setScale(1.0);

}

/**
 *
 */
UnitInfoState::~UnitInfoState()
{

}

/**
 * The unit info can change
 * after going into other screens.
 */
void UnitInfoState::init()
{
	std::wstringstream ss;
	ss << _unit->getTimeUnits();
	_numTimeUnits->setText(ss.str());
	_barTimeUnits->setMax(_unit->getStats()->tu);
	_barTimeUnits->setValue(_unit->getTimeUnits());

	ss.str(L"");
	// aliens have their rank in their "name", soldiers don't
	if (_unit->getType() == "SOLDIER")
	{
		ss << _game->getLanguage()->getString(_unit->getRankString());
		ss << " ";
	}
	ss << _unit->getName(_game->getLanguage(), BattlescapeGame::_debugPlay);
	_txtName->setText(ss.str());

	ss.str(L"");
	ss << _unit->getEnergy();
	_numEnergy->setText(ss.str());
	_barEnergy->setMax(_unit->getStats()->stamina);
	_barEnergy->setValue(_unit->getEnergy());

	ss.str(L"");
	ss << _unit->getHealth();
	_numHealth->setText(ss.str());
	_barHealth->setMax(_unit->getStats()->health);
	_barHealth->setValue(_unit->getHealth());
	_barHealth->setValue2(_unit->getStunlevel());

	ss.str(L"");
	ss << _unit->getFatalWounds();
	_numFatalWounds->setText(ss.str());
	_barFatalWounds->setMax(_unit->getFatalWounds());
	_barFatalWounds->setValue(_unit->getFatalWounds());

	ss.str(L"");
	ss << _unit->getStats()->bravery;
	_numBravery->setText(ss.str());
	_barBravery->setMax(_unit->getStats()->bravery);
	_barBravery->setValue(_unit->getStats()->bravery);

	ss.str(L"");
	ss << _unit->getMorale();
	_numMorale->setText(ss.str());
	_barMorale->setMax(100);
	_barMorale->setValue(_unit->getMorale());

	ss.str(L"");
	ss << _unit->getStats()->reactions;
	_numReactions->setText(ss.str());
	_barReactions->setMax(_unit->getStats()->reactions);
	_barReactions->setValue(_unit->getStats()->reactions);

	ss.str(L"");
	ss << (int)(_unit->getStats()->firing * _unit->getAccuracyModifier());
	_numFiring->setText(ss.str());
	_barFiring->setMax(_unit->getStats()->firing);
	_barFiring->setValue(_unit->getStats()->firing * _unit->getAccuracyModifier());

	ss.str(L"");
	ss << (int)(_unit->getStats()->throwing * _unit->getAccuracyModifier());
	_numThrowing->setText(ss.str());
	_barThrowing->setMax(_unit->getStats()->throwing);
	_barThrowing->setValue(_unit->getStats()->throwing * _unit->getAccuracyModifier());

	ss.str(L"");
	ss << _unit->getStats()->strength;
	_numStrength->setText(ss.str());
	_barStrength->setMax(_unit->getStats()->strength);
	_barStrength->setValue(_unit->getStats()->strength);

	if (_unit->getStats()->psiSkill > 0)
	{
		ss.str(L"");
		ss << _unit->getStats()->psiStrength;
		_numPsiStrength->setText(ss.str());
		_barPsiStrength->setMax(_unit->getStats()->psiStrength);
		_barPsiStrength->setValue(_unit->getStats()->psiStrength);

		ss.str(L"");
		ss << _unit->getStats()->psiSkill;
		_numPsiSkill->setText(ss.str());
		_barPsiSkill->setMax(_unit->getStats()->psiSkill);
		_barPsiSkill->setValue(_unit->getStats()->psiSkill);

		_txtPsiStrength->setVisible(true);
		_numPsiStrength->setVisible(true);
		_barPsiStrength->setVisible(true);

		_txtPsiSkill->setVisible(true);
		_numPsiSkill->setVisible(true);
		_barPsiSkill->setVisible(true);
	}
	else
	{
		_txtPsiStrength->setVisible(false);
		_numPsiStrength->setVisible(false);
		_barPsiStrength->setVisible(false);

		_txtPsiSkill->setVisible(false);
		_numPsiSkill->setVisible(false);
		_barPsiSkill->setVisible(false);
	}

	ss.str(L"");
	ss << _unit->getArmor(SIDE_FRONT);
	_numFrontArmor->setText(ss.str());
	_barFrontArmor->setMax(_unit->getArmor()->getFrontArmor());
	_barFrontArmor->setValue(_unit->getArmor(SIDE_FRONT));

	ss.str(L"");
	ss << _unit->getArmor(SIDE_LEFT);
	_numLeftArmor->setText(ss.str());
	_barLeftArmor->setMax(_unit->getArmor()->getSideArmor());
	_barLeftArmor->setValue(_unit->getArmor(SIDE_LEFT));

	ss.str(L"");
	ss << _unit->getArmor(SIDE_RIGHT);
	_numRightArmor->setText(ss.str());
	_barRightArmor->setMax(_unit->getArmor()->getSideArmor());
	_barRightArmor->setValue(_unit->getArmor(SIDE_RIGHT));

	ss.str(L"");
	ss << _unit->getArmor(SIDE_REAR);
	_numRearArmor->setText(ss.str());
	_barRearArmor->setMax(_unit->getArmor()->getRearArmor());
	_barRearArmor->setValue(_unit->getArmor(SIDE_REAR));

	ss.str(L"");
	ss << _unit->getArmor(SIDE_UNDER);
	_numUnderArmor->setText(ss.str());
	_barUnderArmor->setMax(_unit->getArmor()->getUnderArmor());
	_barUnderArmor->setValue(_unit->getArmor(SIDE_UNDER));
}


/**
 * Closes the window on right-click.
 * @param action Pointer to an action.
 */
void UnitInfoState::handle(Action *action)
{
	State::handle(action);
	if (action->getDetails()->type == SDL_MOUSEBUTTONDOWN)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			_game->popState();
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_X1)
		{
			_game->getSavedGame()->getSavedBattle()->getBattleState()->selectNextPlayerUnit(false, false);
			_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			while (_unit->getArmor()->getSize() > 1
					|| _unit->getRankString() == "STR_LIVE_TERRORIST")
			{
				_game->getSavedGame()->getSavedBattle()->getBattleState()->selectNextPlayerUnit(false, false);
				_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			}
			init();
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_X2)
		{
			_game->getSavedGame()->getSavedBattle()->getBattleState()->selectPreviousPlayerUnit(false);
			_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			while (_unit->getArmor()->getSize() >1
					|| _unit->getRankString() == "STR_LIVE_TERRORIST")
			{
				_game->getSavedGame()->getSavedBattle()->getBattleState()->selectPreviousPlayerUnit(false);
				_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			}
			init();
		}
	}
	if (action->getDetails()->type == SDL_KEYDOWN)
	{
		// "tab" - next solider
		if (action->getDetails()->key.keysym.sym == Options::getInt("keyBattleNextUnit"))
		{
			_game->getSavedGame()->getSavedBattle()->getBattleState()->selectNextPlayerUnit(false, false);
			_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			while (_unit->getArmor()->getSize() >1
					|| _unit->getRankString() == "STR_LIVE_TERRORIST")
			{
				_game->getSavedGame()->getSavedBattle()->getBattleState()->selectNextPlayerUnit(false, false);
				_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			}
			init();
		}
		// prev soldier
		else if (action->getDetails()->key.keysym.sym == Options::getInt("keyBattlePrevUnit"))
		{
			_game->getSavedGame()->getSavedBattle()->getBattleState()->selectPreviousPlayerUnit(false);
			_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			while (_unit->getArmor()->getSize() >1
					|| _unit->getRankString() == "STR_LIVE_TERRORIST")
			{
				_game->getSavedGame()->getSavedBattle()->getBattleState()->selectPreviousPlayerUnit(false);
				_unit = _game->getSavedGame()->getSavedBattle()->getSelectedUnit();
			}
			init();
		}
		else if (action->getDetails()->key.keysym.sym == Options::getInt("keyCancel"))
		{
			_game->popState();
		}
	}
}

}
