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
#ifndef OPENXCOM_EXTRASOUNDS_H
#define OPENXCOM_EXTRASOUNDS_H

#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

class ExtraSounds
{
private:
	std::map<int, std::string> _sounds;
	std::string _folder;
	int _modIndex;
public:
	/// Creates a blank external sound set.
	ExtraSounds(const std::string &folder);
	/// Cleans up the external sound set.
	virtual ~ExtraSounds();
	/// Loads the data from yaml
	void load(const YAML::Node &node, int modIndex);
	/// Gets the list of sounds defined by this mod
	std::map<int, std::string> *getSounds();
	/// Gets main directory with data for this external sound set.
	std::string getFolder();
	/// get the mod index for this external sound set.
	int getModIndex();
};

}

#endif
