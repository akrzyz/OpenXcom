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
#ifndef OPENXCOM_OPTIONSFOLDERS_H
#define OPENXCOM_OPTIONSFOLDERS_H

#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

/**
 * Contains a set of data for specific game entry
 * defined in 'settings.cfg'.
 */
class OptionsFolders
{
private:
	std::string _type;
	std::string _vanillaFolder, _openxcomFolder;
	std::vector<std::string> _rulesets;
public:
	/// Creates a blank option folder for game.
	OptionsFolders(const std::string &type);
	/// Creates basic option folder for game - contains entry ruleset 'name'.
	OptionsFolders(const std::string &type,const std::string &name);
	/// Cleans up the option folder for game.
	~OptionsFolders();
	/// Loads the game entry from YAML.
	void load(const YAML::Node& node);
	/// Saves the game entry to YAML.
	void save(YAML::Emitter& out) const;
	/// Gets the list of rulesets to use with current game entry.
	std::vector<std::string> getRulesets();
	/// Gets vanilla data folder from entry.
	std::string getVanillaFolder();
	/// Gets additional data folder from entry.
	std::string getOpenxcomFolder();
};

}
#endif
