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
GlobeLand::GlobeLand(Game *game, int cenX, int cenY, int width, int height, std::vector<double> radius, int x, int y):
	InteractiveSurface(width, height, x, y, 8),
	 _game(game)
{
	if (_game->getResourcePack()->getSurfaceSet("TEXTURE.DAT") != 0)
	{
		_textureLand = new SurfaceSet(*_game->getResourcePack()->getSurfaceSet("TEXTURE.DAT"));
	}

	_clipper = new FastLineClip(x, x+width, y, y+height);

	_earthData.resize(radius.size());

	//filling normal field for each radius
	for(unsigned int r = 0; r<radius.size(); ++r)
	{
		_earthData[r].resize(width * height);
		for(int j=0; j<height; ++j)
			for(int i=0; i<width; ++i)
			{
				_earthData[r][width*j + i] = static_data.circle_norm(width/2, height/2, radius[r], i+.5, j+.5);
			}
	}

	//filling random noise "texture"
	_randomNoiseData.resize(static_data.random_surf_size * static_data.random_surf_size);
	for(unsigned int i=0; i<_randomNoiseData.size(); ++i)
		_randomNoiseData[i] = rand()%4;
}

/**
 * Deletes the contained surfaces.
 */
GlobeLand::~GlobeLand()
{
	delete _textureLand;

	delete _clipper;
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
 * Increases the zoom level on the globe.
 */
void GlobeLand::zoomIn(size_t zoom)
{
	_game->getSavedGame()->setGlobeZoom(zoom);
}

/**
 * Decreases the zoom level on the globe.
 */
void GlobeLand::zoomOut(size_t zoom)
{
	_game->getSavedGame()->setGlobeZoom(zoom);
}

/**
 * Zooms the globe out as far as possible.
 */
void GlobeLand::zoomMin(size_t zoom)
{
	_game->getSavedGame()->setGlobeZoom(zoom);
}

/**
 * Zooms the globe in as close as possible.
 */
void GlobeLand::zoomMax(size_t zoom)
{
	_game->getSavedGame()->setGlobeZoom(zoom);
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
 * Draws the whole globe, part by part.
 */
void GlobeLand::draw(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom, std::list<Polygon*> cacheLand, std::vector<double> radius)
{
	Surface::draw();
	drawOcean(cenX, cenY, zoom, radius);
	drawLand(zoom, cacheLand);
	drawShadow(cenLon, cenLat, cenX, cenY, zoom);
}


/**
 * Renders the ocean, shading it according to the time of day.
 */
void GlobeLand::drawOcean(Sint16 cenX, Sint16 cenY, size_t zoom, std::vector<double> radius)
{
	if (_game->getResourcePack()->getSurfaceSet("TFTD_TEXTURE.DAT") == 0)
	{
		lock();
		drawCircle(cenX+1, cenY, radius[zoom]+20, Palette::blockOffset(12)+0);
//	ShaderDraw<Ocean>(ShaderSurface(this));
		unlock();
	}
}

/**
 * Renders the land, taking all the visible world polygons
 * and texturing and shading them accordingly.
 */
void GlobeLand::drawLand(size_t zoom, std::list<Polygon*> cacheLand)
{
	Sint16 x[4], y[4];

	for (std::list<Polygon*>::iterator i = cacheLand.begin(); i != cacheLand.end(); ++i)
	{
		// Convert coordinates
		for (int j = 0; j < (*i)->getPoints(); ++j)
		{
			x[j] = (*i)->getX(j);
			y[j] = (*i)->getY(j);
		}

		// Apply textures according to zoom and shade
		int zoomInt = (2 - (int)floor(zoom / 2.0)) * NUM_TEXTURES;
		drawTexturedPolygon(x, y, (*i)->getPoints(), _textureLand->getFrame((*i)->getTexture() + zoomInt), 0, 0);
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


void GlobeLand::drawShadow(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom)
{
	ShaderMove<Cord> earth = ShaderMove<Cord>(_earthData[zoom], getWidth(), getHeight());
	ShaderRepeat<Sint16> noise = ShaderRepeat<Sint16>(_randomNoiseData, static_data.random_surf_size, static_data.random_surf_size);
	
	earth.setMove(cenX-getWidth()/2, cenY-getHeight()/2);
	
	lock();
	ShaderDraw<CreateShadow>(ShaderSurface(this), earth, ShaderScalar(getSunDirection(cenLon, cenLat)), noise);
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
 * Blits the globe onto another surface.
 * @param surface Pointer to another surface.
 */
void GlobeLand::blit(Surface *surface)
{
	Surface::blit(surface);
}

/**
 * Get the polygons shade at a given point
 * @param lon Longitude of the point.
 * @param lat Latitude of the point.
 * @param texture pointer to texture ID returns -1 when polygon not found
 * @param shade pointer to shade
 */
void GlobeLand::getPolygonShade(double lon, double lat, int *texture, int *shade) const
{
	///this is shade conversion from 0..31 levels of geoscape to battlescape levels 0..15
	int worldshades[32] = {  0, 0, 0, 0, 1, 1, 2, 2,
							 3, 3, 4, 4, 5, 5, 6, 6,
							 7, 7, 8, 8, 9, 9,10,11,
							11,12,12,13,13,14,15,15};

	*shade = worldshades[ CreateShadow::getShadowValue(0, Cord(0.,0.,1.), getSunDirection(lon, lat), 0) ];
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
