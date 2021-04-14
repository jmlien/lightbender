//------------------------------------------------------------------------------
//  Copyright 2007-2014 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#ifndef _OBJ_READER_H_
#define _OBJ_READER_H_

#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>
#include <ctype.h>
#include <math.h>

using namespace std;

#include <mathtool/Point.h>
#include <mathtool/Quaternion.h>
#include <mathtool/Vector.h>
using namespace mathtool;

namespace objreader
{
	template<typename T>
	class Vpt
	{
	public:
		Vpt(){ x = y = z = 0; }
		Vpt(const T * v){ x = v[0]; y = v[1]; z = v[2]; }
		T x, y, z;
	};

	template<typename T>
	class V
	{
	public:
		V(){ x = y = z = 0; }
		V(const T * v){ x = v[0]; y = v[1]; z = v[2]; }

		void normalize()
		{
			T norm = (T)sqrt(x*x + y*y + z*z);
			x /= norm;
			y /= norm;
			z /= norm;
		}

		T x, y, z;
	};

	class polygon
	{
	public:
		list<int> pts;
		list<int> textures;
		list<int> normals;
	};

	template<typename T>
	class objModel
	{
	public:
		objModel() { }

		void compute_v_normal()
		{
			//check if normal information is valid from the obj file
			if (normals.empty()){ //compute normal

				normals = vector< V<T> >(pts.size(), V<T>());

				for (auto i = polys.begin(); i != polys.end(); i++)
				{
					//get 3 points, compute normal and assign to all vertices
					auto pi = i->pts.begin();
					vector< Point<T, 3> > v3;
					for (; pi != i->pts.end(); pi++){
						Vpt<T>& pt = pts[*pi];
						Point<T, 3> pos(pt.x, pt.y, pt.z);
						v3.push_back(pos);
						if (v3.size() == 3) break; //we've collected 3 points
					}
					//compute normal
					Vector<T, 3> n = ((v3[1] - v3[0]) % (v3[2] - v3[0]));

					//copy normals
					pi = i->pts.begin();
					for (; pi != i->pts.end(); pi++){

						normals[*pi].x += n[0];
						normals[*pi].y += n[1];
						normals[*pi].z += n[2];

						//normal index is the same as the vertex index
						i->normals.push_back(*pi);

					}//end copying normals

				}//end looping polygons
			}
			else{ // use the information provided
				//do nothing
			}

			//normalize
			for (auto i = normals.begin(); i != normals.end(); i++)
			{
				i->normalize();
			}
		}

		void compute_v_textcoord()
		{
			if (textcoords.empty() == false) return; //nothing to do...already setup
			//get a list of polygons, setup the text coordinate to be all zeros...
			textcoords = vector< V<T> >(1, V<T>());
			for (list<polygon>::iterator i = polys.begin(); i != polys.end(); i++)
			{
				//all texture coord point to this dummy textcoords
				for (list<int>::iterator pi = i->pts.begin(); pi != i->pts.end(); pi++)
				{
					i->textures.push_back(0);
				}
			}//end for i
		}

		vector< Vpt<T> > pts;
		vector< V<T> > normals;
		vector< V<T> > textcoords;
		list<polygon> polys;
	};

	template<typename T>
	class objReader
	{
	public:
		objReader(const string& name){ m_filename = name; }

		bool Read(){
			ifstream in(m_filename.c_str());
			if (!in.good()){
				cerr << "Error: Can't open file " << m_filename << endl;
				return false;
			}
			bool r = Read(in);
			in.close();
			return r;
		}

		const objModel<T>& getModel() const { return data; }
		objModel<T>& getModel() { return data; }

	private:

		bool Read(istream& in)
		{

			string tmp; //used to store temporary data

			//read pts
			while (true)
			{
				if (!(in >> tmp)) break;

				if (tmp == "f") continue;
				if (tmp == "v"){
					Vpt<T> pt;
					in >> pt.x >> pt.y >> pt.z;
					data.pts.push_back(pt);
				}
				else if (tmp == "vn"){
					V<T> pt;
					in >> pt.x >> pt.y >> pt.z;
					data.normals.push_back(pt);
				}
				else if (tmp == "vt"){
					V<T> pt;
					in >> pt.x >> pt.y;
					data.textcoords.push_back(pt);
				}
				getline(in, tmp);
			}

			in.clear();
			in.seekg(0);

			//read faces
			polygon poly;
			while (true)
			{
				//string tmp;
				if (!(in >> tmp)) break;

				if (isdigit(tmp[0])){ //this defines a vetex

					int pos1 = (int)tmp.find('/');
					int pos2 = (int)tmp.rfind('/');

					string field1 = tmp.substr(0, pos1);
					string field2 = tmp.substr(pos1 + 1, pos2 - pos1 - 1);
					string field3 = tmp.substr(pos2 + 1);

					if (pos1 < 0 || pos2 < 0) //has no "/"
					{
						field2.clear();
						field3.clear();
					}
					else if (pos1 == pos2 && pos1 >= 0) //has only on "/"
					{
						field3.clear();
					}

					int id_v = atoi(field1.c_str()) - 1;
					poly.pts.push_back(id_v);


					if (field2.empty() == false)
					{
						int id_t = atoi(field2.c_str()) - 1;
						if (id_t >= 0) poly.textures.push_back(id_t);
					}

					if (field3.empty() == false)
					{
						int id_n = atoi(field3.c_str()) - 1;
						if (id_n >= 0) poly.normals.push_back(id_n);
					}

				}
				else if (tmp == "f")
				{
					if (!poly.pts.empty()) data.polys.push_back(poly);
					poly.pts.clear();
					poly.textures.clear();
					poly.normals.clear();
				}
				else {
					getline(in, tmp);
				}

			}


			data.polys.push_back(poly);
			data.compute_v_normal();
			data.compute_v_textcoord();

			return true;
		}

		string m_filename;
		objModel<T> data;
	};
}

#endif //_OBJ_READER_H_
