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
OptionsFolders::OptionsFolders(const std::string &type) : _type(type), _vanillaFolder(""), _openxcomFolder("")
{

}

/// Creates basic option folder for game - contains entry ruleset 'name'.
OptionsFolders::OptionsFolders(const std::string &type, const std::string &name) : _type(type), _vanillaFolder(""), _openxcomFolder("")
{
	_rulesets.insert(std::pair<std::string, std::string>(name, ""));
}

/// Cleans up the option folder for game.
OptionsFolders::~OptionsFolders()
{

}

/// Loads the game entry from YAML.
void OptionsFolders::load(const YAML::Node& node)
{
	for (YAML::Iterator i = node.begin(); i != node.end(); ++i)
	{
		std::string key;
		i.first() >> key;
		if (key == "vanilla")
		{
			i.second() >> _vanillaFolder;
		}
		else if (key == "openxcom")
		{
			i.second() >> _openxcomFolder;
		}
		else if (key == "rulesets")
		{
			_rulesets.clear();
			for (YAML::Iterator j = i.second().begin(); j != i.second().end(); ++j)
			{
				std::string name, folder;
				(*j)["name"] >> name;
				(*j)["folder"] >> folder;
				_rulesets.insert(std::pair<std::string, std::string>(name, folder));
			}
		}
	}
}

/// Saves the game entry to YAML.
void OptionsFolders::save(YAML::Emitter& out) const
{
	out << YAML::Key << _type << YAML::Value;
	out << YAML::BeginMap;
	out << YAML::Key << "vanilla" << YAML::Value << _vanillaFolder;
	out << YAML::Key << "openxcom" << YAML::Value << _openxcomFolder;
	out << YAML::Key << "rulesets" << YAML::Value;
	out << YAML::BeginSeq;
	for (std::map<std::string, std::string>::const_iterator i = _rulesets.begin(); i != _rulesets.end(); ++i)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "name" << YAML::Value << i->first;
		out << YAML::Key << "folder" << YAML::Value << i->second;
		out << YAML::EndMap;
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;
}

/// Gets the list of rulesets to use with current game entry.
std::map<std::string, std::string> OptionsFolders::getRulesets()
{
	return _rulesets;
}

/// Gets vanilla data folder from entry.
std::string OptionsFolders::getVanillaFolder()
{
	return _vanillaFolder;
}

/// Gets additional data folder from entry.
std::string OptionsFolders::getOpenxcomFolder()
{
	return _openxcomFolder;
}
}
