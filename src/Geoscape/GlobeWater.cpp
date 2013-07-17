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
#include "GlobeWater.h"
#include <fstream>
#include "../aresame.h"
#include "../Engine/SurfaceSet.h"
#include "../Resource/ResourcePack.h"
#include "Polygon.h"
#include "../Engine/FastLineClip.h"
#include "../Engine/Palette.h"
#include "../Engine/Game.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GameTime.h"
#include "../Engine/ShaderMove.h"
#include "../Engine/ShaderRepeat.h"
#include "../Engine/Options.h"

namespace OpenXcom
{


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
			if(d ==  Palette::blockOffset(1) || d ==  Palette::blockOffset(2))
			{
				//this pixel is land
				return Palette::blockOffset(1) + val;
			}
			else
			{
				//this pixel is ocean
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
			if(d ==  Palette::blockOffset(1) || d ==  Palette::blockOffset(2))
			{
				//this pixel is ocean
				return Palette::blockOffset(1);
			}
			else
			{
				//this pixel is land
				return dest;
			}
		}
	}
	
	static inline void func(Uint8& dest, const Cord& earth, const Cord& sun, const Sint16& noise)
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
GlobeWater::GlobeWater(Game *game, int cenX, int cenY, int width, int height, std::vector<double> radius, int x, int y):
	InteractiveSurface(width, height, x, y, 8, "TFTD_PALETTES.DAT_0"),
	 _game(game)
{
	_texture = new SurfaceSet(*_game->getResourcePack()->getSurfaceSet("TFTD_TEXTURE.DAT"));

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
GlobeWater::~GlobeWater()
{
	delete _texture;

	delete _clipper;
}

/**
 * Replaces a certain amount of colors in the palette of the globe.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void GlobeWater::setPalette(SDL_Color *colors, int firstcolor, int ncolors)
{
	Surface::setPalette(colors, firstcolor, ncolors); 

	_texture->setPalette(colors, firstcolor, ncolors);
}

/**
 * Draws the whole globe, part by part.
 */
void GlobeWater::draw(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom, std::list<Polygon*> cacheWater, std::vector<double> radius)
{
	Surface::draw();
	drawLand(cenX, cenY, zoom, radius);
	drawOcean(zoom, cacheWater);
	drawShadow(cenLon, cenLat, cenX, cenY, zoom);
}


/**
 * Renders the water, taking all the visible world polygons
 * and texturing and shading them accordingly.
 */
void GlobeWater::drawOcean(size_t zoom, std::list<Polygon*> cacheWater)
{
	Sint16 x[4], y[4];

	for (std::list<Polygon*>::iterator i = cacheWater.begin(); i != cacheWater.end(); ++i)
	{
		// Convert coordinates
		for (int j = 0; j < (*i)->getPoints(); ++j)
		{
			x[j] = (*i)->getX(j);
			y[j] = (*i)->getY(j);
		}

		// Apply textures according to zoom and shade
		int zoomInt = (2 - (int)floor(zoom / 2.0)) * NUM_TEXTURES;
		drawTexturedPolygon(x, y, (*i)->getPoints(), _texture->getFrame((*i)->getTexture() + zoomInt), 0, 0);
	}
}

/**
 * Renders the land, shading it according to the time of day (if no land textures are given).
 */
void GlobeWater::drawLand(Sint16 cenX, Sint16 cenY, size_t zoom, std::vector<double> radius)
{
	if (_game->getResourcePack()->getSurfaceSet("TEXTURE.DAT") == 0)
	{
		lock();
		drawCircle(cenX+1, cenY, radius[zoom]+20, Palette::blockOffset(1)+0);
//	ShaderDraw<Ocean>(ShaderSurface(this));
		unlock();
	}
}

/**
 * Get position of sun from point on globe
 * @param lon lontidue of position
 * @param lat latitude of position 
 * @return position of sun
 */
Cord GlobeWater::getSunDirection(double lon, double lat) const
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


void GlobeWater::drawShadow(double cenLon, double cenLat, Sint16 cenX, Sint16 cenY, size_t zoom)
{
	ShaderMove<Cord> earth = ShaderMove<Cord>(_earthData[zoom], getWidth(), getHeight());
	ShaderRepeat<Sint16> noise = ShaderRepeat<Sint16>(_randomNoiseData, static_data.random_surf_size, static_data.random_surf_size);
	
	earth.setMove(cenX-getWidth()/2, cenY-getHeight()/2);
	
	lock();
	ShaderDraw<CreateShadow>(ShaderSurface(this), earth, ShaderScalar(getSunDirection(cenLon, cenLat)), noise);
	unlock();
		
}

/**
 * Blits the globe onto another surface.
 * @param surface Pointer to another surface.
 */
void GlobeWater::blit(Surface *surface)
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
void GlobeWater::getPolygonShade(double lon, double lat, int *shade) const
{
	///this is shade conversion from 0..31 levels of geoscape to battlescape levels 0..15
	int worldshades[32] = {  0, 0, 0, 0, 1, 1, 2, 2,
							 3, 3, 4, 4, 5, 5, 6, 6,
							 7, 7, 8, 8, 9, 9,10,11,
							11,12,12,13,13,14,15,15};

	*shade = worldshades[ CreateShadow::getShadowValue(0, Cord(0.,0.,1.), getSunDirection(lon, lat), 0) ];
}

}//namespace OpenXcom
