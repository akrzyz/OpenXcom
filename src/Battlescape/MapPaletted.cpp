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
#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include "MapPaletted.h"
#include "UnitSprite.h"
#include "Position.h"
#include "Pathfinding.h"
#include "TileEngine.h"
#include "Projectile.h"
#include "Explosion.h"
#include "BattlescapeState.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Action.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Engine/RNG.h"
#include "../Engine/Game.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Tile.h"
#include "../Savegame/BattleUnit.h"
#include "../Ruleset/MapDataSet.h"
#include "../Ruleset/MapData.h"
#include "../Ruleset/Armor.h"
#include "BattlescapeMessage.h"
#include "../Savegame/SavedGame.h"
#include "../Interface/Cursor.h"
#include "../Engine/Options.h"
#include "../Interface/NumberText.h"


/*
  1) Map origin is top corner.
  2) X axis goes downright. (width of the map)
  3) Y axis goes downleft. (length of the map
  4) Z axis goes up (height of the map)

           0,0
			/\
	    y+ /  \ x+
		   \  /
		    \/
*/

namespace OpenXcom
{

/**
 * Sets up a map with the specified size and position.
 * @param game Pointer to the core game.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 * @param visibleMapHeight Current visible map height.
 */
MapPaletted::MapPaletted(Game *game, int width, int height, int x, int y, int visibleMapHeight) : InteractiveSurface(width, height, x, y), _game(game), _animFrame(0), _launch(false), _visibleMapHeight(visibleMapHeight), _unitDying(false)
{
	_previewSetting = Options::getInt("battleNewPreviewPath");
	if (Options::getBool("TraceAI"))
	{
		// turn everything on because we want to see the markers.
		_previewSetting = 3;
	}
	_res = _game->getResourcePack();
	_save = _game->getSavedGame()->getSavedBattle();
}

/**
 * Deletes the map.
 */
MapPaletted::~MapPaletted()
{
}

/**
 * Replaces a certain amount of colors in the surface's palette.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void MapPaletted::setPalette(SDL_Color *colors, int firstcolor, int ncolors)
{
	Surface::setPalette(colors, firstcolor, ncolors);
}

/**
 * Put a projectile sprite on the map
 * @param projectile
 */
void MapPaletted::setProjectile(Projectile *projectile)
{
	_projectile = projectile;
	_launch = true;
}

/**
 * Get the current projectile sprite on the map
 * @return 0 if there is no projectile sprite on the map.
 */
Projectile *MapPaletted::getProjectile() const
{
	return _projectile;
}

/**
 * Get a list of explosion sprites on the map.
 * @return a list of explosion sprites.
 */
std::set<Explosion*> *MapPaletted::getExplosions()
{
	return &_explosions;
}

/**
 * Get a list of waypoints on the map.
 * @return a list of waypoints
 */
std::vector<Position> *MapPaletted::getWaypoints()
{
	return &_waypoints;
}

void MapPaletted::setUnitDying(bool flag)
{
	_unitDying = flag;
}

}
