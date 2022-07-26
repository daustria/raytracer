#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"

// Use this #define to selectively compile your code to render the
// bounding boxes around your mesh objects. Uncomment this option
// to turn it on.
//#define RENDER_BOUNDING_VOLUMES

struct Triangle
{
	// These are indices. We get the vertex in constant time
	// by doing something like m_vertices[v1].
	size_t v1; 
	size_t v2;
	size_t v3;

	size_t n1;
	size_t n2;
	size_t n3;

	size_t u1;
	size_t u2;
	size_t u3;

	// Texture coordinates of triangle

	Triangle( size_t pv1, size_t pv2, size_t pv3 , size_t n1 = 0, size_t n2 = 0, size_t n3 = 0, size_t u1 = 0, size_t u2 = 0, size_t u3 = 0 )
		: v1( pv1 )
		  , v2( pv2 )
		  , v3( pv3 )
		  , n1( n1 )
		  , n2( n2 )
		  , n3( n3 )
		  , u1( u1 )
		  , u2( u2 )
		  , u3( u3 )
	{}

};

// A polygonal mesh.
class Mesh : public Primitive {
public:
	Mesh( const std::string& fname );

	virtual void hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const override;
	virtual void updateTextureCoordinates(HitRecord &hr) const override;
private:
	void hitTriangle(HitRecord &hr, const Ray &r, float t_0, float t_1, const Triangle &tri) const;
	void updateTextureCoordinatesTriangle(HitRecord &hr, const Triangle &t) const;

	void readFaceIndices(const std::string & data);

	friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);

	std::vector<glm::vec3> m_vertices;
	std::vector<glm::dvec2> m_textureCoordinates;
	std::vector<glm::vec3> m_normals;
	std::vector<Triangle> m_faces;

	// Parameters for the bounding box of this mesh..
	glm::vec3 m_bmin;
	glm::vec3 m_bmax;
	NonhierBox m_boundingBox;


};
