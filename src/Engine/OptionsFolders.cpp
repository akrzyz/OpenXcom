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

#include "OptionsFolders.h"

namespace OpenXcom
{
	/// Creates a blank option folder for game.
	OptionsFolders::OptionsFolders(const std::string &type)
	{

	}

	/// Cleans up the option folder for game.
	OptionsFolders::~OptionsFolders()
	{

	}

	/// Loads the game entry from YAML.
	void OptionsFolders::load(const YAML::Node& node)
	{

	}

	/// Saves the game entry to YAML.
	void OptionsFolders::save(YAML::Emitter& out) const
	{

	}

	/// Gets the list of rulesets to use with current game entry.
	std::vector<std::string> OptionsFolders::getRulesets()
	{
		return _rulesets;
	}
}
