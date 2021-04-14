#pragma once

#include "model.h"

/**
# "tx"    : translate in x axis
# "ty"    : translate in y axis
# "tz"    : translate in z axis
#
# "cr"    : red component of diffuss color (default is 1)
# "cg"    : green component of diffuss color (default is 1)
# "cb"    : blue component of diffuss color (default is 1)
#
# "sr"    : red component of specular color (default is 1)
# "sg"    : green component of specular color (default is 1)
# "sb"    : blue component of specular color (default is 1)
#
# "ar"    : red component of ambient color (default is 0)
# "ag"    : green component of ambient color (default is 0)
# "ab"    : blue component of ambient color (default is 0)
#
# "catt"  : constant attenuation
# "latt"  : linear attenuation
# "qatt"  : quadratic attenuation
#
# "cutoff": spot light cutoff
# "exp"   : spot light exponent
# "sdx"   : spot light direction in x axis
# "sdy"   : spot light direction in y axis
# "sdz"   : spot light direction in z axis
*/

struct light : public object3D
{
	light()
	{
		spot_dir.set(0, 0, -1);
		spot_cutoff = 60;
		spot_exp = 100;
		att_const = 1;
		att_linear = 0;
		att_quad = 0;
		type = DIRECTIONAL_LIGHT;
		znear = 0.1;
		zfar = 10000.0;
	}

	//type of the light directional, spot light or point light
	enum LIGHT_TYPE { DIRECTIONAL_LIGHT, SPOT_LIGHT, POINT_LIGHT };
	LIGHT_TYPE type;

	//location
	mathtool::Point3d pos;

	//look at this point
	mathtool::Point3d lookat;

	//ambient color
	mathtool::Vector3d ambient;

	//spot light stuff
	double spot_cutoff; //between 0 and 90...
	double spot_exp;    //Only values in the range 0 128 are accepted
	mathtool::Vector3d spot_dir;

	//attenuation (The initial attenuation factors are (1, 0, 0), resulting in no attenuation.)
	double att_const;   //constant attenuation
	double att_linear;  //linear attenuation
	double att_quad;    //quadratic attenuation

	//near, far cutoff plane
	double znear, zfar;
};

//output
inline ofstream & operator<<(ofstream & out, const light& l)
{
	out << static_cast<const object3D&>(l);

	out << " type = ";
	if (l.type == light::DIRECTIONAL_LIGHT) out << "directional";
	else if (l.type == light::SPOT_LIGHT) out << "spot";
	else out << "point";

	out << " lookat = (" << l.lookat << ")"
	    << " tx = " << l.pos[0] << " ty = " << l.pos[1] << " tz = " << l.pos[2]
		<< " ar = " << l.ambient[0] << " ag = " << l.ambient[1] << " ab = " << l.ambient[2]
		<< " cutoff = " << l.spot_cutoff << " exp = " << l.spot_exp
		<< " sdx = " << l.spot_dir[0] << " sdy = " << l.spot_dir[1] << " sdz = " << l.spot_dir[2]
		<< " catt = " << l.att_const << " latt = " << l.att_linear << " qatt = " << l.att_quad
		<< " near = " << l.znear << " far = "<<l.zfar
		<< "\n";

	return out;
}
