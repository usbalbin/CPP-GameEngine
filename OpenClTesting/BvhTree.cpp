#include "stdafx.h"
#include "BvhTree.hpp"


BvhTree::BvhTree(std::vector<TriangleIndices>& indices, std::vector<Vertex>& vertices) : stridingMesh(indices, vertices)
{
	btVector3 notUsed;
	build(&stridingMesh, false, notUsed, notUsed);
}

BvhTree::~BvhTree()
{
}

void BvhTree::appendNodesToVector(std::vector<btOptimizedBvhNode>& vect)
{
	for (int i = 0; i < m_contiguousNodes.size(); i++) {
		vect.push_back(m_contiguousNodes[i]);
	}
}

int BvhTree::getCurNodeIndex() {
	return m_curNodeIndex;
}