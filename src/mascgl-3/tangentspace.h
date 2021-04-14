#pragma once

#include "model.h"

//per vertex tangent space
inline void compute_tangent_space(model& m, vertex& v)
{
	v.t = Vector3d(0, 0, 0);
	v.b = Vector3d(0, 0, 0);

	for (list<uint>::iterator i = v.m_f.begin(); i != v.m_f.end(); i++)
	{
		triangle& t = m.tris[*i];
		v.t = v.t + t.t;
		v.b = v.b + t.b;
	}
	//make sure that v.t and t.b are perpendicular to v.n
	v.t = (v.t - v.n*(v.t*v.n)).normalize();
	v.b = (v.b - v.n*(v.b*v.n)).normalize();
}

//per triangle tangent space
inline void compute_tangent_space(model& m, triangle& t)
{
	const Point3d& p0 = m.vertices[t.v[0]].p;
	const Point3d& p1 = m.vertices[t.v[1]].p;
	const Point3d& p2 = m.vertices[t.v[2]].p;
	Vector3d p1p0 = p1 - p0;
	Vector3d p2p0 = p2 - p0;

	const Vector2d& t0 = m.vertices[t.v[0]].uv; // t.texcoord[0];
	const Vector2d& t1 = m.vertices[t.v[1]].uv; // t.texcoord[1];
	const Vector2d& t2 = m.vertices[t.v[2]].uv; // t.texcoord[2];
	REAL v2v0 = t2[1] - t0[1];
	REAL v1v0 = t1[1] - t0[1];
	REAL u2u0 = t2[0] - t0[0];
	REAL u1u0 = t1[0] - t0[0];
	t.t = (p1p0*v2v0 - p2p0*v1v0);

	REAL d = v2v0*u1u0 - v1v0*u2u0;
	if (d!=0)
		t.t=t.t/d;
	else 
		t.t = t.t.normalize();

	t.b = (t.n%t.t).normalize(); //(p1p0 - u1u0*t.t).normalize();
}

inline void compute_tangent_space(model& m)
{
	//compute tangent space for all triangles first
	for (uint i = 0; i < m.t_size; i++)
		compute_tangent_space(m, m.tris[i]);

	//compute tangent space for all vertices
	for (uint i = 0; i < m.v_size; i++)
		compute_tangent_space(m, m.vertices[i]);
}