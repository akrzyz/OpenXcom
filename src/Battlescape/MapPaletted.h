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
#ifndef OPENXCOM_MAPPALETTED_H
#define OPENXCOM_MAPPALETTED_H

#include "../Engine/InteractiveSurface.h"
#include <set>
#include <vector>

namespace OpenXcom
{

class ResourcePack;
class SavedBattleGame;
class Surface;
class MapData;
class Position;
class Tile;
class BattleUnit;
class Projectile;
class Explosion;

/**
 * Interactive map of the battlescape
 */
class MapPaletted : public InteractiveSurface
{
private:
//	static const int SCROLL_INTERVAL = 20;
	static const int BULLET_SPRITES = 37;
	Game *_game;
	SavedBattleGame *_save;
	ResourcePack *_res;
	int _animFrame;
	Projectile *_projectile;
	bool projectileInFOV;
	std::set<Explosion *> _explosions;
	bool explosionInFOV;
	bool _launch;
	int _visibleMapHeight;
	std::vector<Position> _waypoints;
	bool _unitDying;
	int _previewSetting;
public:
	/// Creates a new map at the specified position and size.
	MapPaletted(Game *game, int width, int height, int x, int y, int visibleMapHeight);
	/// Cleans up the map.
	~MapPaletted();
	/// Sets the palette.
	void setPalette(SDL_Color *colors, int firstcolor = 0, int ncolors = 256);
	/// rotate the tileframes 0-7
	void animate(bool redraw);
	/// Set projectile
	void setProjectile(Projectile *projectile);
	/// Get projectile
	Projectile *getProjectile() const;
	/// Get explosion set
	std::set<Explosion*> *getExplosions();
	/// Get waypoints vector
	std::vector<Position> *getWaypoints();
	void setUnitDying(bool flag);
};

}

#endif
