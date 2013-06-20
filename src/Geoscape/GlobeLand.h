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
class Timer;
class Target;
class LocalizedText;

/**
 * Interactive globe view of the world.
 * Takes a flat world map made out of land polygons with
 * polar coordinates and renders it as a 3D-looking globe
 * with cartesian coordinates that the player can interact with.
 */
class GlobeLand : public InteractiveSurface
{
private:
	static const int NUM_TEXTURES = 13;
	static const int NUM_LANDSHADES = 48;
	static const int NUM_SEASHADES = 72;
	static const double QUAD_LONGITUDE;
	static const double QUAD_LATITUDE;

	SurfaceSet *_textureLand;
	Game *_game;
	FastLineClip *_clipper;
	///normal of each pixel in earth globe per zoom level
	std::vector<std::vector<Cord> > _earthData;
	///data sample used for noise in shading
	std::vector<Sint16> _randomNoiseData;


	/// Get position of sun relative to given position in polar cords and date.
	Cord getSunDirection(double lon, double lat) const;
public:
	/// Creates a new globe at the specified position and size.
	GlobeLand(Game *game, int cenX, int cenY, int width, int height, std::vector<double> radius, int x = 0, int y = 0);
	/// Cleans up the globe.
	~GlobeLand();
	/// Loads a set of polygons from a DAT file.
	static void loadDat(const std::string &filename, std::list<Polygon*> *polygons);
	/// Converts cartesian coordinates to polar coordinates.
	void cartToPolar(Sint16 x, Sint16 y, double *lon, double *lat) const;
	/// Sets the texture set for the globe's land polygons.
	void setTextureLand(SurfaceSet *textureLand);
	/// Zooms the globe in.
	void zoomIn(size_t zoom);
	/// Zooms the globe out.
	void zoomOut(size_t zoom);
	/// Zooms the globe minimum.
	void zoomMin(size_t zoom);
	/// Zooms the globe maximum.
	void zoomMax(size_t zoom);
	/// Turns on/off the globe detail.
	void toggleDetail();
	/// Sets the palette of the globe.
	void setPalette(SDL_Color *colors, int firstcolor = 0, int ncolors = 256);
	/// Draws the whole globe.
	void draw(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom, std::list<Polygon*> cacheLand, std::vector<double> radius);
	/// Draws the ocean of the globe.
	void drawOcean(Sint16 cenX, Sint16 cenY, size_t zoom, std::vector<double> radius);
	/// Draws the land of the globe.
	void drawLand(size_t zoom, std::list<Polygon*> cacheLand);
	/// Draws the shadow.
	void drawShadow(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom);
	/// Blits the globe onto another surface.
	void blit(Surface *surface);
	/// Get the polygons shade at the given point.
	void getPolygonShade(double lon, double lat, int *texture, int *shade) const;
	/// Get the localized text.
	const LocalizedText &tr(const std::string &id) const;
	/// Get the localized text.
	LocalizedText tr(const std::string &id, unsigned n) const;
	/// Draw globe range circle.
	void drawGlobeCircle(double lat, double lon, double radius, int segments);
	/// Special "transparent" line.
	void XuLine(Surface* surface, Surface* src, double x1, double y1, double x2, double y2, Sint16 Color);
};

}

#endif
