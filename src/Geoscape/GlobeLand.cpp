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
#include "GlobeLand.h"
#include <cmath>
#include <fstream>
#include "../aresame.h"
#include "../Engine/Action.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Timer.h"
#include "../Resource/ResourcePack.h"
#include "Polygon.h"
#include "Polyline.h"
#include "../Engine/FastLineClip.h"
#include "../Engine/Palette.h"
#include "../Engine/Game.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/Base.h"
#include "../Savegame/Country.h"
#include "../Ruleset/RuleCountry.h"
#include "../Interface/Text.h"
#include "../Engine/Font.h"
#include "../Engine/Language.h"
#include "../Engine/Exception.h"
#include "../Ruleset/RuleRegion.h"
#include "../Savegame/Region.h"
#include "../Ruleset/City.h"
#include "../Savegame/Target.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Waypoint.h"
#include "../Engine/ShaderMove.h"
#include "../Engine/ShaderRepeat.h"
#include "../Engine/Options.h"
#include "../Savegame/TerrorSite.h"
#include "../Savegame/AlienBase.h"
#include "../Engine/LocalizedText.h"
#include "../Savegame/BaseFacility.h"
#include "../Ruleset/RuleBaseFacility.h"
#include "../Ruleset/RuleCraft.h"
#include "../Ruleset/Ruleset.h"

namespace OpenXcom
{

const double GlobeLand::QUAD_LONGITUDE = 0.05;
const double GlobeLand::QUAD_LATITUDE = 0.2;
const double GlobeLand::ROTATE_LONGITUDE = 0.25;
const double GlobeLand::ROTATE_LATITUDE = 0.15;

namespace
{
	
///helper class for `Globe` for drawing earth globe with shadows
struct GlobeStaticData
{
	///array of shading gradient
	Sint16 shade_gradient[240];
	///size of x & y of noise surface
	const int random_surf_size;
	
	/**
	 * Function returning normal vector of sphere surface
     * @param ox x cord of sphere center
     * @param oy y cord of sphere center
     * @param r radius of sphere
     * @param x cord of point where we getting this vector
     * @param y cord of point where we getting this vector
     * @return normal vector of sphere surface 
     */
	inline Cord circle_norm(double ox, double oy, double r, double x, double y)
	{
		const double limit = r*r;
		const double norm = 1./r;
		Cord ret;
		ret.x = (x-ox);
		ret.y = (y-oy);
		const double temp = (ret.x)*(ret.x) + (ret.y)*(ret.y);
		if(limit > temp)
		{
			ret.x *= norm;
			ret.y *= norm;
			ret.z = sqrt(limit - temp)*norm;
			return ret;
		}
		else
		{
			ret.x = 0.;
			ret.y = 0.;
			ret.z = 0.;
			return ret;
		}
	}
	
	//initialization	
	GlobeStaticData() : random_surf_size(60)
	{
		//filling terminator gradient LUT
		for (int i=0; i<240; ++i)
		{
			int j = i - 120;

			if (j<-66) j=-16;
			else
			if (j<-48) j=-15;
			else
			if (j<-33) j=-14;
			else
			if (j<-22) j=-13;
			else
			if (j<-15) j=-12;
			else
			if (j<-11) j=-11;
			else
			if (j<-9) j=-10;

			if (j>120) j=19;
			else
			if (j>98) j=18;
			else
			if (j>86) j=17;
			else
			if (j>74) j=16;
			else
			if (j>54) j=15;
			else
			if (j>38) j=14;
			else
			if (j>26) j=13;
			else
			if (j>18) j=12;
			else
			if (j>13) j=11;
			else
			if (j>10) j=10;
			else
			if (j>8) j=9;

			shade_gradient[i]= j+16;
		}

	}
};

GlobeStaticData static_data;

struct Ocean
{
	static inline void func(Uint8& dest, const int&, const int&, const int&, const int&)
	{
		dest = Palette::blockOffset(12) + 0;
	}
};

struct CreateShadow
{
	static inline Uint8 getShadowValue(const Uint8& dest, const Cord& earth, const Cord& sun, const Sint16& noise)
	{
		Cord temp = earth;
		//diff
		temp -= sun;
		//norm
		temp.x *= temp.x;
		temp.y *= temp.y;
		temp.z *= temp.z;
		temp.x += temp.z + temp.y;
		//we have norm of distance between 2 vectors, now stored in `x`

		temp.x -= 2;
		temp.x *= 125.;

		if (temp.x < -110)
			temp.x = -31;
		else if (temp.x > 120)
			temp.x = 50;
		else
			temp.x = static_data.shade_gradient[(Sint16)temp.x + 120];

		temp.x -= noise;

		if(temp.x > 0.)
		{
			const Sint16 val = (temp.x> 31)? 31 : (Sint16)temp.x;
			const int d = dest & helper::ColorGroup;
			if(d ==  Palette::blockOffset(12) || d ==  Palette::blockOffset(13))
			{
				//this pixel is ocean
				return Palette::blockOffset(12) + val;
			}
			else
			{
				//this pixel is land
				if (dest==0) return val;
				const int s = val / 3;
				const int e = dest+s;
				if(e > d + helper::ColorShade)
					return d + helper::ColorShade;
				return e;
			}
		}
		else
		{
			const int d = dest & helper::ColorGroup;
			if(d ==  Palette::blockOffset(12) || d ==  Palette::blockOffset(13))
			{
				//this pixel is ocean
				return Palette::blockOffset(12);
			}
			else
			{
				//this pixel is land
				return dest;
			}
		}
	}
	
	static inline void func(Uint8& dest, const Cord& earth, const Cord& sun, const Sint16& noise, const int&)
	{
		if(dest && earth.z)
			dest = getShadowValue(dest, earth, sun, noise);
		else
			dest = 0;
	}
};

}//namespace


/**
 * Sets up a globe with the specified size and position.
 * @param game Pointer to core game.
 * @param cenX X position of the center of the globe.
 * @param cenY Y position of the center of the globe.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 */
GlobeLand::GlobeLand(Game *game, int cenX, int cenY, int width, int height, int x, int y):
	InteractiveSurface(width, height, x, y, 8),
	_rotLon(0.0), _rotLat(0.0),
	_cenX(cenX), _cenY(cenY), _game(game),
	_blink(true), _hover(false), _cacheLand()
{
	if (_game->getResourcePack()->getSurfaceSet("TEXTURE.DAT") != 0)
	{
		_textureLand = new SurfaceSet(*_game->getResourcePack()->getSurfaceSet("TEXTURE.DAT"));
	}

	_clipper = new FastLineClip(x, x+width, y, y+height);

	// Animation timers
	_rotTimer = new Timer(50);
	_rotTimer->onTimer((SurfaceHandler)&GlobeLand::rotate);

	_cenLon = _game->getSavedGame()->getGlobeLongitude();
	_cenLat = _game->getSavedGame()->getGlobeLatitude();
	_zoom = _game->getSavedGame()->getGlobeZoom();

	_radius.push_back(0.45*height);
	_radius.push_back(0.60*height);
	_radius.push_back(0.90*height);
	_radius.push_back(1.40*height);
	_radius.push_back(2.25*height);
	_radius.push_back(3.60*height);
	_earthData.resize(_radius.size());

	//filling normal field for each radius
	for(unsigned int r = 0; r<_radius.size(); ++r)
	{
		_earthData[r].resize(width * height);
		for(int j=0; j<height; ++j)
			for(int i=0; i<width; ++i)
			{
				_earthData[r][width*j + i] = static_data.circle_norm(width/2, height/2, _radius[r], i+.5, j+.5);
			}
	}

	//filling random noise "texture"
	_randomNoiseData.resize(static_data.random_surf_size * static_data.random_surf_size);
	for(unsigned int i=0; i<_randomNoiseData.size(); ++i)
		_randomNoiseData[i] = rand()%4;

	cachePolygons();
}

/**
 * Deletes the contained surfaces.
 */
GlobeLand::~GlobeLand()
{
	delete _textureLand;

	delete _rotTimer;
	delete _clipper;

	for (std::list<Polygon*>::iterator i = _cacheLand.begin(); i != _cacheLand.end(); ++i)
	{
		delete *i;
	}
}

/**
 * Converts a polar point into a cartesian point for
 * mapping a polygon onto the 3D-looking globe.
 * @param lon Longitude of the polar point.
 * @param lat Latitude of the polar point.
 * @param x Pointer to the output X position.
 * @param y Pointer to the output Y position.
 */
void GlobeLand::polarToCart(double lon, double lat, Sint16 *x, Sint16 *y) const
{
	// Orthographic projection
	*x = _cenX + (Sint16)floor(_radius[_zoom] * cos(lat) * sin(lon - _cenLon));
	*y = _cenY + (Sint16)floor(_radius[_zoom] * (cos(_cenLat) * sin(lat) - sin(_cenLat) * cos(lat) * cos(lon - _cenLon)));
}

void GlobeLand::polarToCart(double lon, double lat, double *x, double *y) const
{
	// Orthographic projection
	*x = _cenX + _radius[_zoom] * cos(lat) * sin(lon - _cenLon);
	*y = _cenY + _radius[_zoom] * (cos(_cenLat) * sin(lat) - sin(_cenLat) * cos(lat) * cos(lon - _cenLon));
}


/**
 * Converts a cartesian point into a polar point for
 * mapping a globe click onto the flat world map.
 * @param x X position of the cartesian point.
 * @param y Y position of the cartesian point.
 * @param lon Pointer to the output longitude.
 * @param lat Pointer to the output latitude.
 */
void GlobeLand::cartToPolar(Sint16 x, Sint16 y, double *lon, double *lat) const
{
	// Orthographic projection
	x -= _cenX;
	y -= _cenY;

	double rho = sqrt((double)(x*x + y*y));
	double c = asin(rho / _radius[_zoom]);
	if ( AreSame(rho, 0.0) )
	{
		*lat = _cenLat;
		*lon = _cenLon;

	}
	else
	{
		*lat = asin((y * sin(c) * cos(_cenLat)) / rho + cos(c) * sin(_cenLat));
		*lon = atan2(x * sin(c),(rho * cos(_cenLat) * cos(c) - y * sin(_cenLat) * sin(c))) + _cenLon;
	}

	// Keep between 0 and 2xPI
	while (*lon < 0)
		*lon += 2 * M_PI;
	while (*lon >= 2 * M_PI)
		*lon -= 2 * M_PI;
}

/**
 * Checks if a polar point is on the back-half of the globe,
 * invisible to the player.
 * @param lon Longitude of the point.
 * @param lat Latitude of the point.
 * @return True if it's on the back, False if it's on the front.
 */
bool GlobeLand::pointBack(double lon, double lat) const
{
	double c = cos(_cenLat) * cos(lat) * cos(lon - _cenLon) + sin(_cenLat) * sin(lat);

	return c < 0;
}


/** Return latitude of last visible to player point on given longitude.
 * @param lon Longitude of the point.
 * @return Longitude of last visible point.
 */
double GlobeLand::lastVisibleLat(double lon) const
{
//	double c = cos(_cenLat) * cos(lat) * cos(lon - _cenLon) + sin(_cenLat) * sin(lat);
//		tan(lat) = -cos(_cenLat) * cos(lon - _cenLon)/sin(_cenLat) ;
	return atan(-cos(_cenLat) * cos(lon - _cenLon)/sin(_cenLat));
}

/**
 * Checks if a polar point is inside a certain polygon.
 * @param lon Longitude of the point.
 * @param lat Latitude of the point.
 * @param poly Pointer to the polygon.
 * @return True if it's inside, False if it's outside.
 */
bool GlobeLand::insidePolygon(double lon, double lat, Polygon *poly) const
{
	bool backFace = true;
	for (int i = 0; i < poly->getPoints(); ++i)
	{
		backFace = backFace && pointBack(poly->getLongitude(i), poly->getLatitude(i));
	}
	if (backFace != pointBack(lon, lat))
		return false;

	bool odd = false;
	for (int i = 0; i < poly->getPoints(); ++i)
	{
		int j = (i + 1) % poly->getPoints();

		/*double x = lon, y = lat,
			   x_i = poly->getLongitude(i), y_i = poly->getLatitude(i),
			   x_j = poly->getLongitude(j), y_j = poly->getLatitude(j);*/

		double x, y, x_i, x_j, y_i, y_j;
		polarToCart(poly->getLongitude(i), poly->getLatitude(i), &x_i, &y_i);
		polarToCart(poly->getLongitude(j), poly->getLatitude(j), &x_j, &y_j);
		polarToCart(lon, lat, &x, &y);

		if (((y_i < y && y_j >= y) || (y_j < y && y_i >= y)) && (x_i <= x || x_j <= x))
		{
			odd ^= (x_i + (y - y_i) / (y_j - y_i) * (x_j - x_i) < x);
		}
	}
	return odd;
}

/**
 * Loads a series of map polar coordinates in X-Com format,
 * converts them and stores them in a set of polygons.
 * @param filename Filename of the DAT file.
 * @param polygons Pointer to the polygon set.
 * @sa http://www.ufopaedia.org/index.php?title=WORLD.DAT
 */
void GlobeLand::loadDat(const std::string &filename, std::list<Polygon*> *polygons)
{
	// Load file
	std::ifstream mapFile (filename.c_str(), std::ios::in | std::ios::binary);
	if (!mapFile)
	{
		throw Exception(filename + " not found");
	}

	short value[10];

	while (mapFile.read((char*)&value, sizeof(value)))
	{
		Polygon* poly;
		int points;

		if (value[6] != -1)
		{
			points = 4;
		}
		else
		{
			points = 3;
		}
		poly = new Polygon(points);

		for (int i = 0, j = 0; i < points; ++i)
		{
			// Correct X-Com degrees and convert to radians
			double lonRad = value[j++] * 0.125f * M_PI / 180;
			double latRad = value[j++] * 0.125f * M_PI / 180;

			poly->setLongitude(i, lonRad);
			poly->setLatitude(i, latRad);
		}
		poly->setTexture(value[8]);

		polygons->push_back(poly);
	}

	if (!mapFile.eof())
	{
		throw Exception("Invalid globe map");
	}

	mapFile.close();
}

/**
 * Sets a leftwards rotation speed and starts the timer.
 */
void GlobeLand::rotateLeft()
{
	_rotLon = -ROTATE_LONGITUDE;
	_rotTimer->start();
}

/**
 * Sets a rightwards rotation speed and starts the timer.
 */
void GlobeLand::rotateRight()
{
	_rotLon = ROTATE_LONGITUDE;
	_rotTimer->start();
}

/**
 * Sets a upwards rotation speed and starts the timer.
 */
void GlobeLand::rotateUp()
{
	_rotLat = -ROTATE_LATITUDE;
	_rotTimer->start();
}

/**
 * Sets a downwards rotation speed and starts the timer.
 */
void GlobeLand::rotateDown()
{
	_rotLat = ROTATE_LATITUDE;
	_rotTimer->start();
}

/**
 * Resets the rotation speed and timer.
 */
void GlobeLand::rotateStop()
{
	_rotLon = 0.0;
	_rotLat = 0.0;
	_rotTimer->stop();
}

/**
 * Increases the zoom level on the globe.
 */
void GlobeLand::zoomIn()
{
	if (_zoom < _radius.size() - 1)
	{
		_zoom++;
		_game->getSavedGame()->setGlobeZoom(_zoom);
		cachePolygons();
	}
}

/**
 * Decreases the zoom level on the globe.
 */
void GlobeLand::zoomOut()
{
	if (_zoom > 0)
	{
		_zoom--;
		_game->getSavedGame()->setGlobeZoom(_zoom);
		cachePolygons();
	}
}

/**
 * Zooms the globe out as far as possible.
 */
void GlobeLand::zoomMin()
{
	_zoom = 0;
	_game->getSavedGame()->setGlobeZoom(_zoom);
	cachePolygons();
}

/**
 * Zooms the globe in as close as possible.
 */
void GlobeLand::zoomMax()
{
	_zoom = _radius.size() - 1;
	_game->getSavedGame()->setGlobeZoom(_zoom);
	cachePolygons();
}

/**
 * Rotates the globe to center on a certain
 * polar point on the world map.
 * @param lon Longitude of the point.
 * @param lat Latitude of the point.
 */
void GlobeLand::center(double lon, double lat)
{
	_cenLon = lon;
	_cenLat = lat;
	_game->getSavedGame()->setGlobeLongitude(_cenLon);
	_game->getSavedGame()->setGlobeLatitude(_cenLat);
	cachePolygons();
}

/**
 * Checks if a polar point is inside the globe's landmass.
 * @param lon Longitude of the point.
 * @param lat Latitude of the point.
 * @return True if it's inside, False if it's outside.
 */
bool GlobeLand::insideLand(double lon, double lat) const
{
	bool inside = false;
	// We're only temporarily changing cenLon/cenLat so the "const" is actually preserved
	GlobeLand* const globe = const_cast<GlobeLand* const>(this); // WARNING: BAD CODING PRACTICE
	double oldLon = _cenLon, oldLat = _cenLat;
	globe->_cenLon = lon;
	globe->_cenLat = lat;
	for (std::list<Polygon*>::iterator i = _game->getResourcePack()->getPolygonsLand()->begin(); i != _game->getResourcePack()->getPolygonsLand()->end() && !inside; ++i)
	{
		inside = insidePolygon(lon, lat, *i);
	}
	globe->_cenLon = oldLon;
	globe->_cenLat = oldLat;
	return inside;
}

/**
 * Checks if a certain target is near a certain cartesian point
 * (within a circled area around it) over the globe.
 * @param target Pointer to target.
 * @param x X coordinate of point.
 * @param y Y coordinate of point.
 * @return True if it's near, false otherwise.
 */
bool GlobeLand::targetNear(Target* target, int x, int y) const
{
	Sint16 tx, ty;
	if (pointBack(target->getLongitude(), target->getLatitude()))
		return false;
	polarToCart(target->getLongitude(), target->getLatitude(), &tx, &ty);

	int dx = x - tx;
	int dy = y - ty;
	return (dx * dx + dy * dy <= NEAR_RADIUS);
}

/**
 * Returns a list of all the targets currently near a certain
 * cartesian point over the globe.
 * @param x X coordinate of point.
 * @param y Y coordinate of point.
 * @param craft Only get craft targets.
 * @return List of pointers to targets.
 */
std::vector<Target*> GlobeLand::getTargets(int x, int y, bool craft) const
{
	std::vector<Target*> v;
	if (!craft)
	{
		for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
		{
			if ((*i)->getLongitude() == 0.0 && (*i)->getLatitude() == 0.0)
				continue;

			if (targetNear((*i), x, y))
			{
				v.push_back(*i);
			}

			for (std::vector<Craft*>::iterator j = (*i)->getCrafts()->begin(); j != (*i)->getCrafts()->end(); ++j)
			{
				if ((*j)->getLongitude() == (*i)->getLongitude() && (*j)->getLatitude() == (*i)->getLatitude() && (*j)->getDestination() == 0)
					continue;

				if (targetNear((*j), x, y))
				{
					v.push_back(*j);
				}
			}
		}
	}
	for (std::vector<Ufo*>::iterator i = _game->getSavedGame()->getUfos()->begin(); i != _game->getSavedGame()->getUfos()->end(); ++i)
	{
		if (!(*i)->getDetected())
			continue;

		if (targetNear((*i), x, y))
		{
			v.push_back(*i);
		}
	}
	for (std::vector<Waypoint*>::iterator i = _game->getSavedGame()->getWaypoints()->begin(); i != _game->getSavedGame()->getWaypoints()->end(); ++i)
	{
		if (targetNear((*i), x, y))
		{
			v.push_back(*i);
		}
	}
	for (std::vector<TerrorSite*>::iterator i = _game->getSavedGame()->getTerrorSites()->begin(); i != _game->getSavedGame()->getTerrorSites()->end(); ++i)
	{
		if (targetNear((*i), x, y))
		{
			v.push_back(*i);
		}
	}
	for (std::vector<AlienBase*>::iterator i = _game->getSavedGame()->getAlienBases()->begin(); i != _game->getSavedGame()->getAlienBases()->end(); ++i)
 	{
		if (!(*i)->isDiscovered())
		{
			continue;
		}
		if (targetNear((*i), x, y))
		{
			v.push_back(*i);
		}
	}
	return v;
}

/**
 * Takes care of pre-calculating all the polygons currently visible
 * on the globe and caching them so they only need to be recalculated
 * when the globe is actually moved.
 */
void GlobeLand::cachePolygons()
{
	cache(_game->getResourcePack()->getPolygonsLand(), &_cacheLand);
	_redraw = true;
}

/**
 * Caches a set of polygons.
 * @param polygons Pointer to list of polygons.
 * @param cache Pointer to cache.
 */
void GlobeLand::cache(std::list<Polygon*> *polygons, std::list<Polygon*> *cache)
{
	// Clear existing cache
	for (std::list<Polygon*>::iterator i = cache->begin(); i != cache->end(); ++i)
	{
		delete *i;
	}
	cache->clear();

	// Pre-calculate values to cache
	for (std::list<Polygon*>::iterator i = polygons->begin(); i != polygons->end(); ++i)
	{
		// Is quad on the back face?
		bool backFace = true;
		for (int j = 0; j < (*i)->getPoints(); ++j)
		{
			backFace = backFace && pointBack((*i)->getLongitude(j), (*i)->getLatitude(j));
		}
		if (backFace)
			continue;

		Polygon* p = new Polygon(**i);

		// Convert coordinates
		for (int j = 0; j < p->getPoints(); ++j)
		{
			Sint16 x, y;
			polarToCart(p->getLongitude(j), p->getLatitude(j), &x, &y);
			p->setX(j, x);
			p->setY(j, y);
		}

		cache->push_back(p);
	}
}

/**
 * Replaces a certain amount of colors in the palette of the globe.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void GlobeLand::setPalette(SDL_Color *colors, int firstcolor, int ncolors)
{
	Surface::setPalette(colors, firstcolor, ncolors); 

	_textureLand->setPalette(colors, firstcolor, ncolors);
}

/**
 * Keeps the animation timers running.
 */
void GlobeLand::think()
{
	_rotTimer->think(0, this);
}

/**
 * Rotates the globe by a set amount. Necessary
 * since the globe keeps rotating while a button
 * is pressed down.
 */
void GlobeLand::rotate()
{
	_cenLon += _rotLon;
	_cenLat += _rotLat;
	_game->getSavedGame()->setGlobeLongitude(_cenLon);
	_game->getSavedGame()->setGlobeLatitude(_cenLat);
	cachePolygons();
}

/**
 * Draws the whole globe, part by part.
 */
void GlobeLand::draw()
{
	Surface::draw();
	drawOcean();
	drawLand();
	drawShadow();
}


/**
 * Renders the ocean, shading it according to the time of day.
 */
void GlobeLand::drawOcean()
{
	if (_game->getResourcePack()->getSurfaceSet("TFTD_TEXTURE.DAT") == 0)
	{
		lock();
		drawCircle(_cenX+1, _cenY, _radius[_zoom]+20, Palette::blockOffset(12)+0);
//	ShaderDraw<Ocean>(ShaderSurface(this));
		unlock();
	}
}

/**
 * Renders the land, taking all the visible world polygons
 * and texturing and shading them accordingly.
 */
void GlobeLand::drawLand()
{
	Sint16 x[4], y[4];

	for (std::list<Polygon*>::iterator i = _cacheLand.begin(); i != _cacheLand.end(); ++i)
	{
		// Convert coordinates
		for (int j = 0; j < (*i)->getPoints(); ++j)
		{
			x[j] = (*i)->getX(j);
			y[j] = (*i)->getY(j);
		}

		// Apply textures according to zoom and shade
		int zoom = (2 - (int)floor(_zoom / 2.0)) * NUM_TEXTURES;
		drawTexturedPolygon(x, y, (*i)->getPoints(), _textureLand->getFrame((*i)->getTexture() + zoom), 0, 0);
	}
}

/**
 * Get position of sun from point on globe
 * @param lon lontidue of position
 * @param lat latitude of position 
 * @return position of sun
 */
Cord GlobeLand::getSunDirection(double lon, double lat) const
{
	const double curTime = _game->getSavedGame()->getTime()->getDaylight();
	const double rot = curTime * 2*M_PI;
	double sun;

	if (Options::getBool("globeSeasons"))
	{
		const int MonthDays1[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
		const int MonthDays2[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

		int year=_game->getSavedGame()->getTime()->getYear();
		int month=_game->getSavedGame()->getTime()->getMonth()-1;
		int day=_game->getSavedGame()->getTime()->getDay()-1;

		double tm = (double)(( _game->getSavedGame()->getTime()->getHour() * 60
			+ _game->getSavedGame()->getTime()->getMinute() ) * 60
			+ _game->getSavedGame()->getTime()->getSecond() ) / 86400; //day fraction is also taken into account

		double CurDay;
		if (year%4 == 0 && !(year%100 == 0 && year%400 != 0))
			CurDay = (MonthDays2[month] + day + tm )/366 - 0.219; //spring equinox (start of astronomic year)
		else
			CurDay = (MonthDays1[month] + day + tm )/365 - 0.219;
		if (CurDay<0) CurDay += 1.;

		sun = -0.261 * sin(CurDay*2*M_PI);
	}
	else
		sun = 0;

	Cord sun_direction(cos(rot+lon), sin(rot+lon)*-sin(lat), sin(rot+lon)*cos(lat));

	Cord pole(0, cos(lat), sin(lat));

	if (sun>0)
		 sun_direction *= 1. - sun;
	else
		 sun_direction *= 1. + sun;

	pole *= sun;
	sun_direction += pole;
	double norm = sun_direction.norm();
	//norm should be always greater than 0
	norm = 1./norm;
	sun_direction *= norm;
	return sun_direction;
}


void GlobeLand::drawShadow()
{
	ShaderMove<Cord> earth = ShaderMove<Cord>(_earthData[_zoom], getWidth(), getHeight());
	ShaderRepeat<Sint16> noise = ShaderRepeat<Sint16>(_randomNoiseData, static_data.random_surf_size, static_data.random_surf_size);
	
	earth.setMove(_cenX-getWidth()/2, _cenY-getHeight()/2);
	
	lock();
	ShaderDraw<CreateShadow>(ShaderSurface(this), earth, ShaderScalar(getSunDirection(_cenLon, _cenLat)), noise);
	unlock();
		
}


void GlobeLand::XuLine(Surface* surface, Surface* src, double x1, double y1, double x2, double y2, Sint16)
{
	if (_clipper->LineClip(&x1,&y1,&x2,&y2) != 1) return; //empty line
	x1+=0.5;
	y1+=0.5;
	x2+=0.5;
	y2+=0.5;
	double deltax = x2-x1, deltay = y2-y1;
	bool inv;
	Sint16 tcol;
	double len,x0,y0,SX,SY;
	if (abs((int)y2-(int)y1) > abs((int)x2-(int)x1)) 
	{
		len=abs((int)y2-(int)y1);
		inv=false;
	}
	else
	{
		len=abs((int)x2-(int)x1);
		inv=true;
	}

	if (y2<y1) { 
    SY=-1;
  } else if ( AreSame(deltay, 0.0) ) {
    SY=0;
  } else {
    SY=1;
  }

	if (x2<x1) {
    SX=-1;
  } else if ( AreSame(deltax, 0.0) ) {
    SX=0;
  } else {
    SX=1;
  }

	x0=x1;  y0=y1;
	if (inv)
		SY=(deltay/len);
	else
		SX=(deltax/len);

	while(len>0)
	{
		if (x0>0 && y0>0 && x0<surface->getWidth() && y0<surface->getHeight())
		{
			tcol=src->getPixel((int)x0,(int)y0);
			const int d = tcol & helper::ColorGroup;
			if(d ==  Palette::blockOffset(12) || d ==  Palette::blockOffset(13))
			{
				//this pixel is ocean
				tcol = Palette::blockOffset(12) + 12;
			}
			else
			{
				const int e = tcol+4;
				if(e > d + helper::ColorShade)
					tcol = d + helper::ColorShade;
				else tcol = e;
			}
			surface->setPixel((int)x0,(int)y0,tcol);
		}
		x0+=SX;
		y0+=SY;
		len-=1.0;
	}
}

/**
 *	Draw globe range circle
 */
void GlobeLand::drawGlobeCircle(double lat, double lon, double radius, int segments)
{
	double x, y, x2 = 0, y2 = 0;
	double lat1, lon1;
	double seg = M_PI / (static_cast<double>(segments) / 2);
	for (double az = 0; az <= M_PI*2+0.01; az+=seg) //48 circle segments
	{
		//calculating sphere-projected circle
		lat1 = asin(sin(lat) * cos(radius) + cos(lat) * sin(radius) * cos(az));
		lon1 = lon + atan2(sin(az) * sin(radius) * cos(lat), cos(radius) - sin(lat) * sin(lat1));
		polarToCart(lon1, lat1, &x, &y);
		if ( AreSame(az, 0.0) ) //first vertex is for initialization only
		{
			x2=x;
			y2=y;
			continue;
		}
		x2=x; y2=y;
	}
}

/**
 * Blits the globe onto another surface.
 * @param surface Pointer to another surface.
 */
void GlobeLand::blit(Surface *surface)
{
	Surface::blit(surface);
}

/**
 * Ignores any mouse clicks that are outside the globe.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void GlobeLand::mousePress(Action *action, State *state)
{
	double lon, lat;
	cartToPolar((Sint16)floor(action->getAbsoluteXMouse()), (Sint16)floor(action->getAbsoluteYMouse()), &lon, &lat);

	// Check for errors
	if (lat == lat && lon == lon)
		InteractiveSurface::mousePress(action, state);
}

/**
 * Ignores any mouse clicks that are outside the globe.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void GlobeLand::mouseRelease(Action *action, State *state)
{
	double lon, lat;
	cartToPolar((Sint16)floor(action->getAbsoluteXMouse()), (Sint16)floor(action->getAbsoluteYMouse()), &lon, &lat);

	// Check for errors
	if (lat == lat && lon == lon)
		InteractiveSurface::mouseRelease(action, state);
}

/**
 * Ignores any mouse clicks that are outside the globe
 * and handles globe rotation and zooming.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void GlobeLand::mouseClick(Action *action, State *state)
{
	double lon, lat;
	cartToPolar((Sint16)floor(action->getAbsoluteXMouse()), (Sint16)floor(action->getAbsoluteYMouse()), &lon, &lat);

	// Check for errors
	if (lat == lat && lon == lon)
	{
		InteractiveSurface::mouseClick(action, state);

		// Handle globe control
		if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			center(lon, lat);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
		{
			zoomIn();
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
		{
			zoomOut();
		}
	}
}

/**
 * Handles globe keyboard shortcuts.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void GlobeLand::keyboardPress(Action *action, State *state)
{
	InteractiveSurface::keyboardPress(action, state);
}

/**
 * Get the polygons texture at a given point
 * @param lon Longitude of the point.
 * @param lat Latitude of the point.
 * @param texture pointer to texture ID returns -1 when polygon not found
 * @param shade pointer to shade
 */
void GlobeLand::getPolygonTextureAndShade(double lon, double lat, int *texture, int *shade) const
{
	///this is shade conversion from 0..31 levels of geoscape to battlescape levels 0..15
	int worldshades[32] = {  0, 0, 0, 0, 1, 1, 2, 2,
							 3, 3, 4, 4, 5, 5, 6, 6,
							 7, 7, 8, 8, 9, 9,10,11,
							11,12,12,13,13,14,15,15};

	*texture = -1;
	*shade = worldshades[ CreateShadow::getShadowValue(0, Cord(0.,0.,1.), getSunDirection(lon, lat), 0) ];

	// We're only temporarily changing cenLon/cenLat so the "const" is actually preserved
	GlobeLand* const globe = const_cast<GlobeLand* const>(this); // WARNING: BAD CODING PRACTICE
	double oldLon = _cenLon, oldLat = _cenLat;
	globe->_cenLon = lon;
	globe->_cenLat = lat;
	std::list<Polygon*> *polygons = _game->getResourcePack()->getPolygonsLand();
	if (!polygons->empty())
		for (std::list<Polygon*>::iterator i = polygons->begin(); i != polygons->end(); ++i)
		{
			if (insidePolygon(lon, lat, *i))
			{
				*texture = (*i)->getTexture();
				break;
			}
		}
	polygons = _game->getResourcePack()->getPolygonsWater();
	if (!polygons->empty())
		for (std::list<Polygon*>::iterator i = polygons->begin(); i != polygons->end(); ++i)
		{
			if (insidePolygon(lon, lat, *i))
			{
				*texture = (*i)->getTexture();
				break;
			}
		}
	globe->_cenLon = oldLon;
	globe->_cenLat = oldLat;
}

/**
 * Checks if the globe is zoomed in to it's maximum.
 * @return Returns true if globe is at max zoom, otherwise returns false.
 */
bool GlobeLand::isZoomedInToMax() const
{
	if(_zoom == _radius.size() - 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Checks if the globe is zoomed out to it's maximum.
 * @return Returns true if globe is at max zoom, otherwise returns false.
 */
bool GlobeLand::isZoomedOutToMax() const
{
	if(_zoom == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Get the localized text for dictionary key @a id.
 * This function forwards the call to Language::getString(const std::string &).
 * @param id The dictionary key to search for.
 * @return A reference to the localized text.
 */
const LocalizedText &GlobeLand::tr(const std::string &id) const
{
	return _game->getLanguage()->getString(id);
}

/**
 * Get a modifiable copy of the localized text for dictionary key @a id.
 * This function forwards the call to Language::getString(const std::string &, unsigned).
 * @param id The dictionary key to search for.
 * @param n The number to use for the proper version.
 * @return A copy of the localized text.
 */
LocalizedText GlobeLand::tr(const std::string &id, unsigned n) const
{
	return _game->getLanguage()->getString(id, n);
}

}//namespace OpenXcom
