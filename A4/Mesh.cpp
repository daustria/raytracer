#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"



static std::string getAssetFilePath(const std::string &fname) 
{

	// Only put the assets part if the filename does not already include it 
	std::string assets_dir{"Assets"};
	if (fname.length() > assets_dir.length()) {
		if (fname.substr(0,assets_dir.length()) == assets_dir) {
			return fname;
		}
	}

	return "Assets/"+fname;
}

// TODO:
// For some reason, m_vertices and m_faces are turning out as zero vectors with nothing inside them.
// Need to check if this code even works.
Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	//std::ifstream ifs( fname.c_str() );
	std::ifstream ifs( getAssetFilePath(fname).c_str() );

	// Note: if we want to put textures (which I definitely do) we will
	// have to modify this code.. and probably other places as well
	
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	m_primitiveType = PrimitiveType::Mesh;
}

void Mesh::hit_base(HitRecord &hr, const Ray &r, float t_0, float t_1) const
{
	// We follow the same logic as intersecting a group of surfaces,
	// since a mesh can be thought of as a gorup of triangles.

	hr.miss = true;

	for (const Triangle &face : m_faces) 
	{
		HitRecord rec;

		hitTriangle(rec, r, t_0, t_1, face);

		if (!rec.miss) {
			hr = rec;
			t_1 = rec.t;
		}	
	}

	hr.p = this;
	
}

void Mesh::hitTriangle(HitRecord &hr, const Ray &r, float t_0, float t_1, const Triangle &tri) const
{
	const glm::vec3 &a_tri = m_vertices[tri.v1];
	const glm::vec3 &b_tri = m_vertices[tri.v2];
	const glm::vec3 &c_tri = m_vertices[tri.v3];

	// We use cramer's rule to solve the triangle ray intersection problem, as in 4.5 of Shirley's book.
	// This amounts to solving the linear system Ax = y, where x = (beta, gamma, t) has the unknowns 
	// which tell us everything we need to know about the ray intersection on the triangle (including if it intersected).
	
	glm::mat3 A;
	glm::vec3 y;

	A[0] = a_tri - b_tri;
	A[1] = a_tri - c_tri;
	A[2] = r.d;

	y = a_tri - r.o;

	// Now we use cramer's rule to solve for the unknowns. Of course we store terms that come up a lot..

	// The variable names come from viewing the 3x3 matrix this way: 
	// a b c
	// d e f
	// g h i
	
	// and y as (j,k,l)
	
	float ei_hf = A[1][1] * A[2][2] - A[2][1] * A[1][2];
	float gf_di = A[2][0] * A[1][2] - A[1][0] * A[2][2];
	float dh_eg = A[1][0] * A[2][1] - A[1][1] * A[2][0];
	float ak_jb = A[0][0] * y[1]    - y[0] * A[0][1];
	float jc_al = y[0] * A[0][2]    - A[0][0] * y[2];
	float bl_kc = A[0][1] * y[2]    - y[1] * A[0][2];

	// serves as the denominator for the cramers rule computation
	float M = A[0][0]*ei_hf + A[0][1]*gf_di + A[0][2]*dh_eg;

	float t = A[1][2] * ak_jb + A[1][1] * jc_al + A[1][0] * bl_kc;
	t = t / -M;

	if ((t < t_0) || (t > t_1)) {
		// We intersect the plane containing the triangle, but outside the given interval,
		// so we missed.
		hr.miss = true;
		return;
	}

	float gamma = A[2][2] * ak_jb + A[2][1] * jc_al + A[2][0] * bl_kc;
	gamma = gamma / M;

	if ((gamma < 0) || (gamma > 1)) {
		// Invalid barycentric coordinates if we are here, so we can't have hit
		hr.miss = true;
		return;
	}

	float beta = y[0] * ei_hf + y[1] * gf_di + y[2] * dh_eg;
	beta = beta / M;

	if ((beta < 0) || (beta + gamma > 1)) {
		// Again, we have invalid barycentric coordinate, so we record as a miss
		hr.miss = true;
		return;
	}

	// If we haven't returned yet... then we hit! 

	hr.miss = false;
	hr.t = t;
	hr.n = glm::normalize(glm::cross(b_tri - a_tri, c_tri - a_tri));
	// We'll set the primitive pointer as the mesh outside this helper function
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
	out << "mesh {";

	/*
	   for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
	   const MeshVertex& v = mesh.m_verts[idx];
	   out << glm::to_string( v.m_position );
	   if( mesh.m_have_norm ) {
	   out << " / " << glm::to_string( v.m_normal );
	   }
	   if( mesh.m_have_uv ) {
	   out << " / " << glm::to_string( v.m_uv );
	   }
	   }
	   */

	out << "}";
	return out;
}
