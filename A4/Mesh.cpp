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

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	  , m_faces()
	  , m_boundingBox(glm::vec3(), 1.0f)
{
	std::string code;

	static const float EPSILON_MESH(0.01f);

	bool first_vertex(true);

	//std::ifstream ifs( fname.c_str() );
	std::ifstream ifs( getAssetFilePath(fname).c_str() );
	
	while( ifs >> code ) {

		if( code == "v" ) {

			double vx, vy, vz;

			ifs >> vx >> vy >> vz;
			glm::vec3 v{vx, vy, vz};
			m_vertices.push_back(v);

			if (first_vertex) {
				// Initialize the min and max vertices for bounding box
				m_bmin = v - glm::vec3(EPSILON_MESH);
				m_bmax = v + glm::vec3(EPSILON_MESH);
				first_vertex = false;
			} else {
				// Update the min and max vertices of the bounding box

				for(int i = 0; i < 3; ++i) {
					if (v[i] < m_bmin[i]) {
						m_bmin[i] = v[i];
					}

					if (v[i] > m_bmax[i]) {
						m_bmax[i] = v[i];
					}
				}
			}

		} else if ( code == "vn" ) {

			double n1, n2, n3;
			ifs >> n1 >> n2 >> n3;
			glm::vec3 n{n1, n2, n3};

			m_normals.push_back(n);

		} else if (code == "vt" ) {

			double u, v;
			ifs >> u >> v;

			glm::dvec2 t{u, v};

			m_textureCoordinates.push_back(t);

		} else if( code == "f" ) {

			std::string currentLine;
			std::getline(ifs, currentLine);

			// The code for this function is simple but a bit long and tedious so I just 
			// moved it to this helper function
			readFaceIndices(currentLine);

		} else {

			printf(" %s | ERROR : Unknown code in reading Mesh file\n", __func__);

			// Advance past the current line and continue as normal
			std::string currentLine;
			std::getline(ifs, currentLine);

			continue;
		}

	}

	m_boundingBox = NonhierBox(m_bmin, m_bmax - m_bmin);
	m_primitiveType = PrimitiveType::Mesh;
}

void Mesh::readFaceIndices(const std::string & currentLine)
{
	int positionIndexA, positionIndexB, positionIndexC;
	int normalIndexA, normalIndexB, normalIndexC;
	int uvCoordIndexA, uvCoordIndexB, uvCoordIndexC;

	int index;

	// sscanf will return the number of matched index values it found
	// from the pattern.

	int matches_type_a = sscanf(currentLine.c_str(), " %d/%d/%d", &index, &index, &index);
	int matches_type_b = sscanf(currentLine.c_str(), " %d//%d", &index, &index);
	int matches_type_c = sscanf(currentLine.c_str(), " %d %d %d", &index, &index, &index);

	if ( matches_type_a == 3 ) {

		// Line contains indices of the pattern vertex/uv-cord/normal.
		sscanf(currentLine.c_str(), " %d/%d/%d %d/%d/%d %d/%d/%d",
				&positionIndexA, &uvCoordIndexA, &normalIndexA,
				&positionIndexB, &uvCoordIndexB, &normalIndexB,
				&positionIndexC, &uvCoordIndexC, &normalIndexC);

		// .obj file uses indices that start at 1, so subtract 1 so they start at 0.
		positionIndexA--;
		positionIndexB--;
		positionIndexC--;
		uvCoordIndexA--;
		uvCoordIndexB--;
		uvCoordIndexC--;
		normalIndexA--;
		normalIndexB--;
		normalIndexC--;

		m_faces.push_back( Triangle( positionIndexA, positionIndexB, positionIndexC, 
					uvCoordIndexA, uvCoordIndexB, uvCoordIndexC, 
					normalIndexA, normalIndexB, normalIndexC ) );

	} else if ( matches_type_b == 2) {

		// Line contains indices of the pattern vertex//normal.
		sscanf(currentLine.c_str(), " %d//%d %d//%d %d//%d",
				&positionIndexA, &normalIndexA,
				&positionIndexB, &normalIndexB,
				&positionIndexC, &normalIndexC);

		positionIndexA--;
		positionIndexB--;
		positionIndexC--;
		normalIndexA--;
		normalIndexB--;
		normalIndexC--;

		m_faces.push_back( Triangle( positionIndexA, positionIndexB, positionIndexC,
					normalIndexA, normalIndexB, normalIndexC ) );


	} else if ( matches_type_c == 3 ) {

		// Regular pattern of 'vertex vertex vertex', no normals and texture coordinates

		sscanf(currentLine.c_str(), " %d %d %d", &positionIndexA, &positionIndexB, &positionIndexC);

		positionIndexA--;
		positionIndexB--;
		positionIndexC--;

		m_faces.push_back( Triangle( positionIndexA, positionIndexB, positionIndexC ) );
	}
}

void Mesh::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const
{
	// First we check if it hits our bounding box, to avoid 
	// doing unnecessary work of intersecting with each triangle
	
	m_boundingBox.hitBase(hr, r, t_0, t_1);

	if (hr.miss) {
		return;
	}
		
	// Now we follow the same logic as intersecting a group of surfaces,
	// since a mesh can be thought of as a gorup of triangles.

	hr.miss = true;

	const Triangle *hit_face = nullptr;

	for (const Triangle &face : m_faces) 
	{
		HitRecord rec;

		hitTriangle(rec, r, t_0, t_1, face);

		if (!rec.miss) {
			hr = rec;
			t_1 = rec.t;
			hit_face = &face;
		}	
	}

	// If we hit a triangle, update the texture coordinates
	if (!hr.miss && hit_face) {
		updateTextureCoordinatesTriangle(hr, *hit_face);
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
	hr.hit_point = r.evaluate(t);
	hr.n = glm::normalize(glm::cross(b_tri - a_tri, c_tri - a_tri));
	// We'll set the primitive pointer as the mesh outside this helper function
}
void Mesh::updateTextureCoordinates(HitRecord &hr) const 
{
	// Do nothing. We update texture coordinates already in our hitBase routine
}
void Mesh::updateTextureCoordinatesTriangle(HitRecord &hr, const Triangle &t) const
{
	// Do nothing, for now, since we haven't tried loading UV coordinates for meshes yet
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
