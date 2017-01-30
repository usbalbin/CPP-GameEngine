#pragma once

#include "Containers.hpp"
#include "bullet\BulletCollision\CollisionShapes\btStridingMeshInterface.h"

#include <vector>

class StridingMesh :
	public btStridingMeshInterface
{
public:
	StridingMesh(std::vector<TriangleIndices>& indices, std::vector<Vertex>& vertices);
	~StridingMesh();

	void getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart = 0) {};
	void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride, const unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart = 0) const;

	void unLockVertexBase(int subpart) {};
	void unLockReadOnlyVertexBase(int subpart) const {};
	int	getNumSubParts() const { return 1; };
	void preallocateVertices(int numverts) {};
	void preallocateIndices(int numindices) {};


private:
	std::vector<TriangleIndices>& indices;
	std::vector<Vertex>& vertices;
};

