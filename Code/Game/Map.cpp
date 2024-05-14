#include "Map.hpp"


//----------------------------------------------------------------------------------------------------------
Map::Map( std::vector<AABB3> const& obstacles )
	: m_obstacleAABBs( obstacles )
{
	CreateConvexHulls();
}


//----------------------------------------------------------------------------------------------------------
void Map::CreateConvexHulls()
{
	for (int index = 0; index < m_obstacleAABBs.size(); index++)
	{
		ConvexHull3 convexHull = ConvexHull3( m_obstacleAABBs[ index ] );
		m_obstacleConvexHulls.push_back( convexHull );
	}
}
