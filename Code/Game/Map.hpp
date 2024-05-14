#pragma once

#include "Engine/Math/ConvexHull3.hpp"
#include "Engine/Math/AABB3.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------
class Map
{
public:
	Map(std::vector<AABB3> const& obstacles);


	//----------------------------------------------------------------------------------------------------------
	std::vector<AABB3> m_obstacleAABBs;
	std::vector<ConvexHull3> m_obstacleConvexHulls;

	void CreateConvexHulls();
};