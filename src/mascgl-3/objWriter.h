#pragma once

#include "geometry/TriangleMesh.h"

class ObjWriter {

public:

    ObjWriter() {
    }

    ~ObjWriter() {
    }

    void addMesh(const TriangleMesh & mesh) {

	unordered_map<int,int> vn;

	for (int v_idx : mesh.getVertexIndices()) {
	    const Vector3d & v = mesh.getVertexPos(v_idx);
	    m_vertex_pos.push_back(v);
	    vn[v_idx] = m_vertex_pos.size();
	}


	for (int i : mesh.getTriangleIndices()) {
	    const auto & f = mesh.getTriangleVertices(i);
	    vector<int> tri = { vn.at(f[0]) , vn.at(f[1]) , vn.at(f[2]) };
	    m_triangles_vertices.push_back(tri);
	}
    }

    //void addBox(const Vector3d & c, double l, double w, double h) {
//}   


    void print(ostream & out) const {
	const double tiny = 1e-11;

	for (const auto  & v : m_vertex_pos) {
	    out << "v";
	    out << " " << ((fabs(v[0]) < tiny) ? 0 : v[0]);
	    out << " " << ((fabs(v[1]) < tiny) ? 0 : v[1]);
	    out << " " << ((fabs(v[2]) < tiny) ? 0 : v[2]);
	    out << endl;
	}

	for (const auto & f : m_triangles_vertices) {
	    out << "f " << f[0] << " " << f[1] << " " << f[2] << endl;
	}
    }


    void save(const string & path) const {
	ofstream out;
	out.open(path, ofstream::out);
	if (!out.good()) {
	    cerr << "!Error! failed to open " << path << endl;
	    return;
	}
	print(out);
	out.close();
	cerr << " - obj output to " << path << endl;
    }

private:
    
    vector<Vector3d> m_vertex_pos;
    vector< vector<int> > m_triangles_vertices;
};
