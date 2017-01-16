#pragma once

#include "Containers.hpp"
#include "StridingMesh.hpp"
#include "BulletCollision\CollisionShapes\btOptimizedBvh.h"

#include <vector>

class BvhTree :
	public btOptimizedBvh
{
public:
	BvhTree(std::vector<TriangleIndices>& indices, std::vector<Vertex>& vertices);
	~BvhTree();

	NodeArray& getNodes() { return m_contiguousNodes; }

	void appendNodesToVector(std::vector<btOptimizedBvhNode>& vect);

	int getCurNodeIndex();

private:
	StridingMesh stridingMesh;
};

