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
#ifndef OPENXCOM_GLOBELAND_H
#define OPENXCOM_GLOBELAND_H

#include <vector>
#include <list>
#include "../Engine/InteractiveSurface.h"
#include "../Engine/FastLineClip.h"
#include "Cord.h"

namespace OpenXcom
{

class Game;
class Polygon;
class SurfaceSet;

/**
 * Land elements of the globe.
 */
class GlobeLand : public InteractiveSurface
{
private:
	static const int NUM_TEXTURES = 13;
	static const int NUM_LANDSHADES = 48;
	static const int NUM_SEASHADES = 72;

	SurfaceSet *_texture;
	Game *_game;
	FastLineClip *_clipper;
	///normal of each pixel in earth globe per zoom level
	std::vector<std::vector<Cord> > _earthData;
	///data sample used for noise in shading
	std::vector<Sint16> _randomNoiseData;


	/// Get position of sun relative to given position in polar cords and date.
	Cord getSunDirection(double lon, double lat) const;
public:
	/// Creates a new land globe at the specified position and size.
	GlobeLand(Game *game, int cenX, int cenY, int width, int height, std::vector<double> radius, int x = 0, int y = 0);
	/// Cleans up the land.
	~GlobeLand();
	/// Sets the palette of the land.
	void setPalette(SDL_Color *colors, int firstcolor = 0, int ncolors = 256);
	/// Draws the whole land.
	void draw(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom, std::list<Polygon*> cacheLand, std::vector<double> radius);
	/// Draws the ocean of the globe (if no water textures are given).
	void drawOcean(Sint16 cenX, Sint16 cenY, size_t zoom, std::vector<double> radius);
	/// Draws the land of the globe.
	void drawLand(size_t zoom, std::list<Polygon*> cacheLand);
	/// Draws the shadow to land.
	void drawShadow(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom);
	/// Blits the land onto another surface.
	void blit(Surface *surface);
	/// Get the polygons shade at the given point.
	void getPolygonShade(double lon, double lat, int *shade) const;
};

}

#endif
