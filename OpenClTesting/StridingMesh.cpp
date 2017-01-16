#include "stdafx.h"
#include "StridingMesh.hpp"

#include "Containers.hpp"

#include <vector>


StridingMesh::StridingMesh(std::vector<TriangleIndices>& indices, std::vector<Vertex>& vertices) : indices(indices), vertices(vertices)
{

}

StridingMesh::~StridingMesh()
{
}

void StridingMesh::getLockedReadOnlyVertexIndexBase(const unsigned char ** vertexbase, int & numverts, PHY_ScalarType & type, int & stride, const unsigned char ** indexbase, int & indexstride, int & numfaces, PHY_ScalarType & indicestype, int subpart) const
{
	*vertexbase = (const unsigned char *)&(vertices[0].position.x);
	numverts = vertices.size();
	type = PHY_ScalarType::PHY_FLOAT;
	stride = sizeof(Vertex);

	*indexbase = (const unsigned char *)indices.data();
	numfaces = indices.size();
	indicestype = PHY_ScalarType::PHY_INTEGER;
	indexstride = sizeof(TriangleIndices);
}
