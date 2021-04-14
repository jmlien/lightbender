#pragma once

#include "objReader.h"
#include "model.h"
#include "light.h"
#include "camera.h"

#include <list>
#include <float.h>
#include <algorithm>
#include <sstream>
using namespace std;

extern Camera camera; //defined in draw_basics.cpp

struct bounding_box
{
	bounding_box()
	{
		for (int i = 0; i < 6; i++)
		{
			sides[i] = NULL;
			dim[i] = 0;
			visible[i] = true;
			add2mesh[i]=true;
		}
		//end for i
	}

	//0:left, 1:right, 2:bottom, 3:top, 4:back, 5:front
	bool visible[6];
	bool add2mesh[6];
	REAL dim[6];     //dimension
	model * sides[6];  //geometry for each side...
};

//-----------------------------------------------------------------------------
// INPUTS
struct mascgl_workspace
{
	string env_filename; //an environment file defines models and their transformation and material properties

	//JML: we might need to get rid of these...
	string vertex_shader_name;
	string fragment_shader_name;

	//-----------------------------------------------------------------------------
	// Intermediate data
	list<object3D *> models;
	list<light*>     lights;
	bounding_box bbox; //bounding box
	map<string, model*> name2model; //this maps filename to the first model loaded from this file

	//some default textures
	Texture texture_white;
	Texture texture_black;

	//
	REAL R = 0;       //radius
	mathtool::Point3d COM;       //center of mass

	unsigned int image_w = 100, image_h = 100; //image size
	unsigned int n_ray = 10;            //n rays per pixel
	unsigned int n_light_sample = 10;   //n samples per light objects
	unsigned int n_ray_AO = 0;          //n rays per ambient occlusion query
	unsigned int max_depth = 10;        //max ray depth
	REAL AO_radius = 10;              //ambient occulision radius

	//-----------------------------------------------------------------------------
	// lightbender related methods

	//input image that we try to reproduce
	string input_image_filename;

	//lightbender structure paramters
	float structure_width, structure_height; //dimension of the structure
	float gap;         //size of the gap between cells
	int n_row, n_col;  //number of rows and cols

	//-----------------------------------------------------------------------------
	//
	//
	//
	//  parsing objects
	//
	//
	//-----------------------------------------------------------------------------

	//convert a string to a list of tokens
	list<string> tokenize(char * tmp, const char * ignore)
	{
		list<string> tokens;
		char * tok = strtok(tmp, ignore);
		while (tok != NULL)
		{
			tokens.push_back(tok);
			tok = strtok(NULL, ignore);
		}
		return tokens;
	}

	list<string> tokenize(istream& in)
	{
		const int size = 1024;
		char * tmp = new char[size];
		in.getline(tmp, size);//read lines from a file into strings
		list<string> tok = tokenize(tmp, " =\t[]()<>,");
		delete tmp;
		return tok;
	}

	string & next(list<string>::iterator & i, list<string>& tok)
	{
		if (i == tok.end())
		{
			stringstream ss;
			ss << "! Error: parsing next error. Reaches the end of list.";
			throw ss.str();
		}

		return *i;
	}

	bool findImageSize(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			if (t == "width") image_w = (unsigned int)atoi(next(++i,tok).c_str());
			else if (t == "height") image_h = (unsigned int)atoi(next(++i, tok).c_str());
			else if (t == "max_depth") max_depth = (unsigned int)atoi(next(++i, tok).c_str());
			else if (t == "n") n_ray = (unsigned int)atoi(next(++i, tok).c_str());
			else if (t == "n_light_sample") n_light_sample = (unsigned int)atoi(next(++i, tok).c_str());
			else if (t == "n_ambient") n_ray_AO = (unsigned int)atoi(next(++i, tok).c_str());
			else if (t == "ambient_radius") AO_radius = (REAL)atof(next(++i, tok).c_str());
			else if (t == "vertex_shader") vertex_shader_name = next(++i, tok);
			else if (t == "fragment_shader") fragment_shader_name = next(++i, tok);
		}

		return true;
	}

	void parseObject(object3D * m, list<string>& tok)
	{
		list<string> unknown;
		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			//difuss color
			else if (t == "cr") m->mat_color[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "cg") m->mat_color[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "cb") m->mat_color[2] = (REAL)atof(next(++i, tok).c_str());
			//specular color
			else if (t == "sr") m->mat_specular[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "sg") m->mat_specular[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "sb") m->mat_specular[2] = (REAL)atof(next(++i, tok).c_str());
			//emission color
			else if (t == "er") m->mat_emission[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "eg") m->mat_emission[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "eb") m->mat_emission[2] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "shininess") m->mat_shininess = (REAL)atof(next(++i, tok).c_str());
			//texture
			else if (t == "text_c")  m->color_texture_filename = next(++i, tok);
			else if (t == "text_n") m->normalmap_texture_filename = next(++i, tok);
			//name
			else if (t == "name") m->name = next(++i, tok);
			//unknown...
			else unknown.push_back(t);
		}

		tok.swap(unknown);
	}

	bool createLight(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		light * li = new light();

		assert(li);
		parseObject(li, tok);

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			//light position
			if (t == "tx") li->pos[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "ty") li->pos[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "tz") li->pos[2] = (REAL)atof(next(++i, tok).c_str());
			//light  type
			else if (t == "type")
			{
				string name = next(++i, tok);
				if (tolower(name) == "spot") li->type = light::SPOT_LIGHT;
				else if (tolower(name) == "directional") li->type = light::DIRECTIONAL_LIGHT;
				else if (tolower(name) == "point") li->type = light::POINT_LIGHT;
				else cerr << "! Warning: Unknown light type: " << name << ". ignore" << endl;;
			}
			//look-at location
			else if (t == "lookat")
			{
				for (int d = 0; d < 3; d++) li->lookat[d] = (REAL)atof(next(++i, tok).c_str());
			}
			//ambient color
			else if (t == "ar") li->ambient[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "ag") li->ambient[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "ab") li->ambient[2] = (REAL)atof(next(++i, tok).c_str());
			//attenuation
			else if (t == "catt") li->att_const = (REAL)atof(next(++i, tok).c_str());
			else if (t == "latt") li->att_linear = (REAL)atof(next(++i, tok).c_str());
			else if (t == "qatt") li->att_quad = (REAL)atof(next(++i, tok).c_str());
			//spot light stuff
			else if (t == "cutoff") li->spot_cutoff = (REAL)atof(next(++i, tok).c_str());
			else if (t == "exp") li->spot_exp = (REAL)atof(next(++i, tok).c_str());
			else if (t == "sdx") li->spot_dir[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "sdy") li->spot_dir[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "sdz") li->spot_dir[2] = (REAL)atof(next(++i, tok).c_str());
			//near/far cut-off
			else if (t == "near") li->znear = (REAL)atof(next(++i, tok).c_str());
			else if (t == "far") li->zfar = (REAL)atof(next(++i, tok).c_str());
		}

		lights.push_back(li);

		return true;
	}


	bool setupCamera(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			//camera position
			if (t == "tx") camera.setCameraPosX((REAL)atof(next(++i, tok).c_str()));
			else if (t == "ty") camera.setCameraPosY((REAL)atof(next(++i, tok).c_str()));
			else if (t == "tz") camera.setCameraPosZ((REAL)atof(next(++i, tok).c_str()));
			else if (t == "azim") camera.setAzim((REAL)atof(next(++i, tok).c_str()));
			else if (t == "elev") camera.setElev((REAL)atof(next(++i, tok).c_str()));
		}

		return true;
	}

	bool createLightBender(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			//camera position
			if (t == "img") input_image_filename=next(++i, tok);
			else if (t == "width") structure_width=(REAL)atof(next(++i, tok).c_str());
			else if (t == "heigh") structure_height=(REAL)atof(next(++i, tok).c_str());
			else if (t == "gap") gap=(REAL)atof(next(++i, tok).c_str());
			else if (t == "row") n_row=atoi(next(++i, tok).c_str());
			else if (t == "col") n_col=atoi(next(++i, tok).c_str());
		}

		//image must be setup
		if(input_image_filename.empty()) return false;
		return true;
	}

	bool createModel(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		mathtool::Vector3d local_rot(0, 0, 0);
		mathtool::Vector3d pos(0, 0, 0);
		REAL s = 1; //scale
		string filename;

		model * m = new model();
		assert(m);
		parseObject(m, tok);

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			if (t == "tx") pos[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "ty") pos[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "tz") pos[2] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "rx") local_rot[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "ry") local_rot[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "rz") local_rot[2] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "scale") s = (REAL)atof(next(++i, tok).c_str());
			else filename = t;
		}

		//check if this model has been build before...
		//if not we read from file. Otherwise we copy from an existing model...

		if (name2model.find(filename) == name2model.end())
		{
			if (!m->build(filename)) return false;
			name2model[filename] = m;
			cout << "- Read model " << filename << endl;
		}
		else
		{
			m->copy(name2model[filename]);
			//cout << "COM="<<m->getCOM() << " R=" << m->getR() << endl;
		}


		// updated to use
		mathtool::Quaternion<REAL> rot(local_rot.get()); //rz*ry*rx;
		//mathtool::Matrix3x3 rotM = rot.getMatrix();
		m->setCurrentTransform(pos, rot, s);
		models.push_back(m);

		return true;
	}

	bool createSphere(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		sphere * m = new sphere();
		assert(m);

		parseObject(m, tok);

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++){
			string& t = *i;
			if (t[0] == '#') break;
			if (t == "tx") m->center[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "ty") m->center[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "tz") m->center[2] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "r") m->radius = (REAL)atof(next(++i, tok).c_str());
			//transparency
			else if (t == "Tr") m->transparency[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "Tg") m->transparency[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "Tb") m->transparency[2] = (REAL)atof(next(++i, tok).c_str());
			//relectiveness (how mirrow-like this object is)
			else if (t == "Rr") m->reflectiveness[0] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "Rg") m->reflectiveness[1] = (REAL)atof(next(++i, tok).c_str());
			else if (t == "Rb") m->reflectiveness[2] = (REAL)atof(next(++i, tok).c_str());
			//refractive index
			else if (t == "RI") m->refractive_index = (REAL)atof(next(++i, tok).c_str());
		}

		models.push_back(m);

		return true;
	}

	//this function creates one side of the box... not the whole box
	bool createBbox(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		int side = -1;
		REAL offset = 0;
		bool visible = true;
		bool buildmesh = false;

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++)
		{
			string t = tolower(*i);
			if (t[0] == '#') break;

			if (t == "left") side = 0;
			else if (t == "right") side = 1;
			else if (t == "bottom") side = 2;
			else if (t == "top") side = 3;
			else if (t == "back") side = 4;
			else if (t == "front") side = 5;
			else if (t == "offset") offset = (REAL)atof(next(++i,tok).c_str());
			else if (t == "invisible") visible = false;
		}

		if (side<0 || side>6) return false; // not a valid side

		model * wall=bbox.sides[side] = new model();
		assert(bbox.sides[side]);
		bbox.dim[side] = offset;
		bbox.visible[side] = visible;
		parseObject(wall, tok);

		if (visible)
			models.push_back(bbox.sides[side]);

		return true;
	}

	bool parseMethod(istream& in)
	{
		list<string> tok = tokenize(in);
		if (tok.size() == 0) return false;			//empty line
		if (tok.front()[0] == '#') return false; 	//commented out

		for (list<string>::iterator i = tok.begin(); i != tok.end(); i++)
		{
			string t = tolower(*i);
			if (t[0] == '#') break;
			//dymsum_method_params.push_back(t);
		}

		return true;
	}

	bool initialize(istream& in)
	{
		while (!in.eof())
		{
			list<string> tok = tokenize(in);
			if (tok.empty()) continue;

			string label = tok.front();
			tok.pop_front();
			if (label[0] == '#') continue;

			label = tolower(label);

			//initialize image
			if (label == "image")
			{
				if (findImageSize(in) == false)  return false;
			}//end if obst

			//initialize lights
			else if (label == "light")
			{
				int m_size = atoi(tok.front().c_str());
				for (int i = 0; i<m_size; i++)
				{
					if (createLight(in) == false)  return false;
				}//end outer for
			}//end if light

			//initialize camera
			else if (label == "camera")
			{
				if (setupCamera(in) == false)  return false;
			}//end if camera

			//initialize lightbender
			else if (label == "lightbender")
			{
				if (createLightBender(in) == false)  return false;
			}//end if lightbender

			//initialize meshes
			else if (label == "mesh")
			{
				int m_size = atoi(tok.front().c_str());
				for (int i = 0; i<m_size; i++)
				{
					if (createModel(in) == false)  return false;
				}//end outer for
			}//end if obst

			//initialize sphere
			else if (label == "sphere")
			{
				int sphere_size = atoi(tok.front().c_str());
				for (int i = 0; i<sphere_size; i++)
				{
					if (createSphere(in) == false)  return false;
				}//end outer for
			}//end if obst

			//initialize bounding box
			else if (label == "bbox")
			{
				int m_size = atoi(tok.front().c_str());
				for (int i = 0; i < m_size; i++) //one for each side...
				{
					if (createBbox(in) == false) return false;
				}
			}

			//parse method related parameters
			else if (label == "method")
			{
				if (parseMethod(in) == false) return false;
			}

			//Unknown
			else{
				cerr << "! Warning: Unknown value: (" << label << ")" << endl;
				continue;
			}
		}

		return true;
	}

	bool initialize(const string& filename)
	{
		env_filename = filename;
		ifstream fin(env_filename.c_str());

		if (fin.good() == false)
		{
			cerr << "! Error: Failed to open file " << env_filename << endl;
			return false;
		}

		try
		{
			if (initialize(fin) == false) return false;
		}
		catch (string& error)
		{
			cerr << "! Error: " << error << endl;
			fin.close();
			return false;
		}

		fin.close();

		//
		computeCOM_R();

		return true;
	}

	//-----------------------------------------------------------------------------

	void computeCOM_R()
	{
		//compute a bbox
		REAL box[6] = { FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX };
		//-------------------------------------------------------------------------
		for (list<object3D*>::iterator i = models.begin(); i != models.end(); i++)
		{
			object3D* obj = *i;

			//this obj is a mesh
			if (dynamic_cast<model*>(obj))
			{
				model* mesh = dynamic_cast<model*>(obj);
				for (unsigned int j = 0; j < mesh->v_size; j++){ //this is a sphere
					mathtool::Point3d& p = mesh->vertices[j].p;
					if (p[0]<box[0]) box[0] = p[0];
					if (p[0]>box[1]) box[1] = p[0];
					if (p[1]<box[2]) box[2] = p[1];
					if (p[1]>box[3]) box[3] = p[1];
					if (p[2]<box[4]) box[4] = p[2];
					if (p[2]>box[5]) box[5] = p[2];
				}//j
			}
			else if (dynamic_cast<sphere*>(obj)) //this is a ball
			{
				sphere* ball = dynamic_cast<sphere*>(obj);
				REAL sbox[6] = { ball->center[0] - ball->radius, ball->center[0] + ball->radius,
				                   ball->center[1] - ball->radius, ball->center[1] + ball->radius,
					               ball->center[2] - ball->radius, ball->center[2] + ball->radius };

				if (sbox[0] < box[0]) box[0] = sbox[0];
				if (sbox[1] > box[1]) box[1] = sbox[1];
				if (sbox[2] < box[2]) box[2] = sbox[2];
				if (sbox[3] > box[3]) box[3] = sbox[3];
				if (sbox[4] < box[4]) box[4] = sbox[4];
				if (sbox[5] > box[5]) box[5] = sbox[5];
			}
		}//i

		//apply offsetand
		for (int i = 0; i < 6; i++)
		{
			bbox.dim[i] += box[i];
			box[i] = bbox.dim[i];
		}

		//for (int i = 0; i < 6; i++) cout << box[i] << ", ";
		//cout << endl;

		//update COM
		COM.set((box[1] + box[0]) / 2, (box[3] + box[2]) / 2, (box[5] + box[4]) / 2);

		//-------------------------------------------------------------------------
		R = (REAL)(COM - mathtool::Point3d(box[0], box[2], box[4])).norm();

		//compute geometry for each side...
		buildBoxModel();
	}


	//create a model with the front open...
	void buildBoxModel()
	{
		REAL * box = bbox.dim;

		mathtool::Point3d llb(box[0], box[2], box[4]), lrb(box[1], box[2], box[4]), urb(box[1], box[3], box[4]), ulb(box[0], box[3], box[4]),
			              llf(box[0], box[2], box[5]), lrf(box[1], box[2], box[5]), urf(box[1], box[3], box[5]), ulf(box[0], box[3], box[5]);

		//cout << "llf=" << llf << " , lrf=" << lrf << ", lrb=" << lrb << ", llb=" << llb << endl;

		//build left wall
		if (bbox.sides[0] != NULL) buildBoxWall(bbox.sides[0], llf, llb, ulb, ulf, mathtool::Vector3d(1, 0, 0));

		//build right wall
		if (bbox.sides[1] != NULL) buildBoxWall(bbox.sides[1], lrf, urf, urb, lrb, mathtool::Vector3d(-1, 0, 0));

		//build bottom floor
		if (bbox.sides[2] != NULL) buildBoxWall(bbox.sides[2], llf, lrf, lrb, llb, mathtool::Vector3d(0, 1, 0));

		//build top ceiling
		if (bbox.sides[3] != NULL) buildBoxWall(bbox.sides[3], ulf, ulb, urb, urf, mathtool::Vector3d(0, -1, 0));

		//build back wall
		if (bbox.sides[4] != NULL) buildBoxWall(bbox.sides[4], llb, lrb, urb, ulb, mathtool::Vector3d(0, 0, 1));

		//build front wall
		if (bbox.sides[5] != NULL) buildBoxWall(bbox.sides[5], llf, lrf, urf, ulf, mathtool::Vector3d(0, 0, -1));

	}

	inline void buildBoxWall
		(model * m, const mathtool::Point3d& a, const mathtool::Point3d& b, const mathtool::Point3d& c,
		            const mathtool::Point3d& d, const mathtool::Vector3d& n)
	{
		//build left wall
		m->v_size = 4;
		m->vertices = new vertex[4];
		m->vertices[0].p = a;
		m->vertices[0].n = n;
		m->vertices[0].uv = mathtool::Vector2d(0, 0);
		m->vertices[1].p = b;
		m->vertices[1].n = n;
		m->vertices[1].uv = mathtool::Vector2d(1, 0);
		m->vertices[2].p = c;
		m->vertices[2].n = n;
		m->vertices[2].uv = mathtool::Vector2d(1, 1);
		m->vertices[3].p = d;
		m->vertices[3].n = n;
		m->vertices[3].uv = mathtool::Vector2d(0, 1);

		m->e_size = 5;
		m->edges = new edge[5];
		m->edges[0].vid[0] = 0;
		m->edges[0].vid[1] = 1;
		m->edges[1].vid[0] = 1;
		m->edges[1].vid[1] = 2;
		m->edges[2].vid[0] = 2;
		m->edges[2].vid[1] = 3;
		m->edges[3].vid[0] = 3;
		m->edges[3].vid[1] = 0;
		m->edges[4].vid[0] = 2;
		m->edges[4].vid[1] = 0;

		m->t_size = 2;
		m->tris = new triangle[2];
		m->tris[0].v[0] = 0;
		m->tris[0].v[1] = 1;
		m->tris[0].v[2] = 2;
		m->tris[0].n = n;
		m->tris[1].v[0] = 2;
		m->tris[1].v[1] = 3;
		m->tris[1].v[2] = 0;
		m->tris[1].n = n;
	}

	void createDefaultTextures()
	{
		//create a 1X1 white texture
		unsigned char white[] = { 255, 255, 255 };
		texture_white.createTexture(GL_RGB,1,1,white);

		//create a 1X1 black texture
		unsigned char black[] = { 0, 0, 0 };
		texture_black.createTexture(GL_RGB, 1, 1, black);
	}

	string tolower(const string& s)
	{
		string lower;
		for (string::const_iterator i = s.begin(); i != s.end(); i++)
		{
			char c = ::tolower(*i);
			lower.push_back(c);
		}
		return lower;
	}

	//check if a model is wall
	inline bool is_wall(model* m)
	{
		for (int i = 0; i < 6; i++)
		if (bbox.sides[i] == m) return true;
		return false;
	}

	bool save(bool verbose=false)
	{
		//save the results to file
		string::size_type rpos = env_filename.find(".r");
		string output_filename = env_filename.substr(0, rpos) + "-output.r";

		if (verbose) cout << "- Save result to " << output_filename << endl;

		ofstream fout(output_filename.c_str());
		if (fout.good())
		{
			//image
			fout << "# create from " << env_filename << endl;
			fout << "image\n n = " << n_ray << " width = " << image_w << " height = " << image_h << " vertex_shader = " << vertex_shader_name << " fragment_shader = " << fragment_shader_name << "\n";
			fout << "\n";

			//light
			light * mylight = lights.front();
			fout << "light 1\n";
			fout << (*mylight) << "\n";

			//count
			int mesh_count = 0;
			for (auto& obj : models)
			{
				if (dynamic_cast<model*>(obj)) //this object is a mesh
				{
					model* mesh = dynamic_cast<model*>(obj);
					if (is_wall(mesh) == false && mesh->cast_shadow) mesh_count++;
				}
			}

			fout << "bbox 6\n";
			//0:left, 1:right, 2:bottom, 3:top, 4:back, 5:front
			const string wall_names[] = { "left", "right", "bottom", "top", "back", "front" };
			for (int i = 0; i < 6; i++)
			{
				fout << wall_names[i] << " offset = " << bbox.dim[i] << " ";
				if (bbox.visible[i] == false)
				{
					fout << "invisible\n";
					continue;
				}

				if (bbox.sides[i] == NULL) continue;
				fout << dynamic_cast<object3D&>(*bbox.sides[i]) << "\n";
			}
			fout << "\n";

			//	left   offset = -100 shininess = 1
			//	right  offset = 1000  shininess = 1
			//	front invisible  offset = 500 shininess = 1
			//	back  offset = -500  shininess = 1 text_c = texture / mickey.jpg
			//	top  invisible offset = 600
			fout << "mesh " << mesh_count << "\n";
			for (auto& obj : models)
			{
				if (dynamic_cast<model*>(obj)) //this object is a mesh
				{
					model* mesh = dynamic_cast<model*>(obj);
					if (is_wall(mesh) || mesh->cast_shadow==false ) continue;
					fout << *mesh << "\n";
				}
			}
		}
		else
		{
			if (verbose) cout << "- Failed to open file " << output_filename << endl;
			return false;
		}

		fout.close();

		return true;
	}

};
