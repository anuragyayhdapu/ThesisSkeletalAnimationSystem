#include "Game/GameFinalShowcase.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Game/GameVaultingTest.hpp"
#include "Game/AnimationState.hpp"
#include "Game/GameBasicMovement.hpp"
#include "Game/MovementState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/ThirdPersonController.hpp"
#include "Game/Character.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimCrossFadeController.hpp"
#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"


//----------------------------------------------------------------------------------------------------------
Vec3 Character::Physics::GetForward() const
{
	Vec3 iForward = m_orientation * Vec3( 1.f, 0.f, 0.f );
	return iForward;
}


//----------------------------------------------------------------------------------------------------------
Character::Mesh::~Mesh()
{
	delete m_gpuVerts;
}


//----------------------------------------------------------------------------------------------------------
Character::Character()
{
	m_physics.m_position	= Vec3( 9.f, -13.f, 0.f );

	m_physics.m_orientation = Quaternion( 0.f, 0.f, 0.713f, 0.702f );
	m_physics.m_orientation.Normalize();
}

Character::~Character()
{
}


//----------------------------------------------------------------------------------------------------------
void Character::Startup()
{
	InitSpriteLitShader();
	InitMovementState();
	InitRaycast();
	InitDebugRootMotionTranslation();
	CreateRootMotionTranslationGraphVerts();
	LoadMeshData();
}


//----------------------------------------------------------------------------------------------------------
void Character::CreateVerts()
{
	// debug wire frame sphere
	AddVertsForSphere3D( m_mesh.m_cpuVerts, Vec3::ZERO, m_physics.m_radius );
	m_mesh.m_gpuVerts	  = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	size_t vertexDataSize = sizeof( Vertex_PCU ) * m_mesh.m_cpuVerts.size();
	g_theRenderer->CopyCPUToGPU( m_mesh.m_cpuVerts.data(), vertexDataSize, m_mesh.m_gpuVerts );

	// debug position sphere
	AddVertsForSphere3D( m_debugPositionMesh.m_cpuVerts, Vec3::ZERO, 0.1f );
	m_debugPositionMesh.m_gpuVerts = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( m_debugPositionMesh.m_cpuVerts.data(), sizeof( Vertex_PCU ) * m_debugPositionMesh.m_cpuVerts.size(), m_debugPositionMesh.m_gpuVerts );

	// unit vector mesh
	AddVertsForCylinder3D( m_debugUnitVectorMesh.m_cpuVerts, Vec3::ZERO, Vec3( 1.f, 0.f, 0.f ), 0.03f );
	m_debugUnitVectorMesh.m_gpuVerts = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( m_debugUnitVectorMesh.m_cpuVerts.data(), sizeof( Vertex_PCU ) * m_debugUnitVectorMesh.m_cpuVerts.size(), m_debugUnitVectorMesh.m_gpuVerts );
}


//----------------------------------------------------------------------------------------------------------
void Character::Update()
{
	UpdateMovementState();
	UpdateAnimations();
	UpdateFootRaycast();
	UpdateHeadRaycast();
	UpdateShoulderRaycasts();
	UpdateGroundRaycast();
	ToggleMeshRender();

	UpdateDebugRootMotionTranslation();
}


//----------------------------------------------------------------------------------------------------------
void Character::Render() const
{
	RenderAnimations();
	DebugRender();
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRender() const
{
	if ( !m_showDebugRender )
		return;

	// DebugRenderPhysicsSphere();
	// DebugRenderBasisVectors();
	DebugRenderCurrentMovementState();
	DebugRenderRaycast();
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRenderUI() const
{
	if ( !g_theApp->CanDebugDraw2D() )
	{
		return;
	}

	AABB2 screenBounds = g_theGame->m_screenCamera.GetOrthographicBounds();
	DebugRenderGraphVerts();

	AABB2 zBounds = screenBounds.GetBoxAtUVs( AABB2( 0.7f, 0.f, 1.f, 0.3f ) );
	DebugrenderZRootMotionTranslationUI( zBounds );

	AABB2 yBounds = screenBounds.GetBoxAtUVs( AABB2( 0.7f, 0.3f, 1.f, 0.6f ) );
	DebugrenderYRootMotionTranslationUI( yBounds );

	AABB2 xBounds = screenBounds.GetBoxAtUVs( AABB2( 0.7f, 0.6f, 1.f, 0.9f ) );
	DebugrenderXRootMotionTranslationUI( xBounds );
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRenderPhysicsSphere() const
{
	if ( !g_theApp->CanDebugDraw3D() )
	{
		return;
	}

	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );

	Mat44 transform;
	transform.SetTranslation3D( m_physics.m_position );
	g_theRenderer->SetModelConstants( transform, Rgba8::WHITE );

	// render a sphere at the position
	g_theRenderer->DrawVertexBuffer( m_mesh.m_gpuVerts, ( int ) m_mesh.m_cpuVerts.size(), 0 );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexBuffer( m_debugPositionMesh.m_gpuVerts, ( int ) m_debugPositionMesh.m_cpuVerts.size(), 0 );
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRenderBasisVectors() const
{
	if ( !g_theApp->CanDebugDraw3D() )
	{
		return;
	}

	Vec3  iForword = m_physics.m_orientation * Vec3( 1.f, 0.f, 0.f );
	Vec3  jLeft	   = m_physics.m_orientation * Vec3( 0.f, 1.f, 0.f );
	Vec3  kUp	   = m_physics.m_orientation * Vec3( 0.f, 0.f, 1.f );

	Mat44 transform;
	transform.SetTranslation3D( m_physics.m_position );
	transform.SetIJK3D( iForword, jLeft, kUp );
	g_theRenderer->SetModelConstants( transform, Rgba8::RED );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexBuffer( m_debugUnitVectorMesh.m_gpuVerts, ( int ) m_debugUnitVectorMesh.m_cpuVerts.size(), 0 );
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateAnimations()
{
	g_theAnimationController->Update();
}


//----------------------------------------------------------------------------------------------------------
void Character::RenderAnimations() const
{
	AnimPose const& sampledPose = g_theAnimationController->GetSampledPose();
	RenderPose( sampledPose, Rgba8::WHITE );
	RenderMeshData( sampledPose );
}


//----------------------------------------------------------------------------------------------------------
void Character::RenderPose( AnimPose const& pose, Rgba8 color ) const
{
	if ( m_renderMesh )
		return;

	std::vector<Vertex_PCU> verts;

	// for each transform in the pose, debug draw line from parent to child
	for ( int index = 0; index < pose.GetNumberOfJoints(); index++ )
	{
		Transform const& self = pose.GetGlobalTransformOfJoint( index );

		if ( g_theCharacter->m_movementState->GetStateName() == "hangDrop" )
		{
			if ( index == 0 )
			{
				DebuggerPrintf( "zValue: %.3f\n", self.m_position.z );
			}
		}

		int parentIndex = pose.GetParentOfJoint( index );
		if ( parentIndex < 0 )
		{
			AddVertsForSphere3D( verts, self.m_position, 0.04f, Rgba8::CYAN );

			continue;
		}
		Transform const& parent = pose.GetGlobalTransformOfJoint( parentIndex );

		// AddVertsForCylinder3D( verts, self.m_position, parent.m_position, 0.05f );
		AddVertsForCone3D( verts, parent.m_position, self.m_position, 0.03f );
		// AddVertsForSphere3D( verts, self.m_position, 0.04f, Rgba8::LIGHT_CORAL );
		//  DebugAddWorldLine( self.m_position, parent.m_position, 1.f, 0.f );
	}

	Mat44 modelTransformMatrix;
	modelTransformMatrix.AppendTranslation3D( m_physics.m_position );
	modelTransformMatrix.Append( m_physics.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp() );

	g_theRenderer->SetModelConstants( modelTransformMatrix, color );
	// g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	if ( g_theApp->CanDebugDraw3D() )
	{
		verts.clear();
		AddVertsForSphere3D( verts, m_physics.m_position, 0.1f, Rgba8::LIGHT_CORAL );

		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( verts );
	}
}


//----------------------------------------------------------------------------------------------------------
void Character::InitMovementState()
{
	m_movementState = new IdleMovementState();
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateMovementState()
{
	MovementState* newState = m_movementState->UpdateTransition();
	if ( newState )
	{
		// swap mpvement state
		delete m_movementState;
		m_movementState = newState;

		//  request animation change
		g_theAnimationController->m_transitionRequested = true;
		g_theAnimationController->m_nextStateName		= m_movementState->GetStateName();

		CreateRootMotionTranslationGraphVerts();
	}
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRenderCurrentMovementState() const
{
	if ( !g_theApp->CanDebugDraw2D() )
	{
		return;
	}

	Camera const& screenCamera = g_theGame->m_screenCamera;
	float		  fontSize	   = 15.f;
	AABB2		  cameraBounds( screenCamera.GetOrthographicBottomLeft(), screenCamera.GetOrthographicTopRight() );
	float		  textMinX = cameraBounds.m_mins.x;
	float		  textMinY = cameraBounds.m_maxs.y * 0.9f;
	Vec2		  topLeftLinePosition( textMinX, textMinY );
	Vec2		  topLeftAlignment	  = Vec2( 0.f, 1.f );
	float		  duration			  = 0.f; // one frame
	std::string	  currentMoveStateStr = m_movementState->GetStateName();

	DebugAddScreenText( "Movement State", topLeftLinePosition, fontSize, topLeftAlignment, duration, Rgba8::WHITE );

	topLeftLinePosition.y -= fontSize - 2.f;
	DebugAddScreenText( currentMoveStateStr, topLeftLinePosition, fontSize, topLeftAlignment, duration, Rgba8::RED );
}


//----------------------------------------------------------------------------------------------------------
void Character::InitRaycast()
{
	if ( m_map )
		m_obstacles = m_map->m_obstacleConvexHulls;
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateFootRaycast()
{
	Vec3  rayStartPos	   = m_physics.m_position + Vec3( 0.f, 0.f, 0.1f );
	Vec3  rayForwardnormal = m_physics.m_orientation.GetVector_XFwd();
	float rayLength		   = m_footRaycastMaxDistance;

	for ( int index = 0; index < m_obstacles.size(); index++ )
	{
		ConvexHull3 const& obstacleHull = m_obstacles[ index ];
		m_footRaycastResult				= RaycastVsConvexHull3D( rayStartPos, rayForwardnormal, rayLength, obstacleHull );

		if ( m_footRaycastResult.m_didImpact )
		{
			break;
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRenderRaycast() const
{
	if ( !g_theApp->CanDebugDraw3D() )
	{
		return;
	}

	std::vector<Vertex_PCU> verts;

	// foot raycast
	Vec3  rayStartpos = m_footRaycastResult.m_rayStartPos;
	Vec3  rayEndpos	  = rayStartpos + ( m_footRaycastResult.m_rayFwdNormal * m_footRaycastResult.m_rayMaxLength );
	Rgba8 color		  = m_footRaycastResult.m_didImpact ? Rgba8::GREEN : Rgba8::RED;
	AddVertsForArrow3D( verts, rayStartpos, rayEndpos, 0.05f, color );

	// head raycast
	rayStartpos = m_headRaycast.m_rayStartPos;
	rayEndpos	= rayStartpos + ( m_headRaycast.m_rayFwdNormal * m_headRaycast.m_rayMaxLength );
	color		= m_headRaycast.m_didImpact ? Rgba8::GREEN : Rgba8::RED;
	AddVertsForArrow3D( verts, rayStartpos, rayEndpos, 0.05f, color );

	// shoulder raycasts
	rayStartpos = m_rightShoulderRaycastResult.m_rayStartPos;
	rayEndpos	= rayStartpos + ( m_rightShoulderRaycastResult.m_rayFwdNormal * m_rightShoulderRaycastResult.m_rayMaxLength );
	color		= m_rightShoulderRaycastResult.m_didImpact ? Rgba8::GREEN : Rgba8::RED;
	AddVertsForArrow3D( verts, rayStartpos, rayEndpos, 0.05f, color );
	rayStartpos = m_leftShoulderRaycastResult.m_rayStartPos;
	rayEndpos	= rayStartpos + ( m_leftShoulderRaycastResult.m_rayFwdNormal * m_leftShoulderRaycastResult.m_rayMaxLength );
	color		= m_leftShoulderRaycastResult.m_didImpact ? Rgba8::GREEN : Rgba8::RED;
	AddVertsForArrow3D( verts, rayStartpos, rayEndpos, 0.05f, color );

	// ground raycast
	rayStartpos = m_groundRaycastResult.m_rayStartPos;
	rayEndpos	= rayStartpos + ( m_groundRaycastResult.m_rayFwdNormal * m_groundRaycastResult.m_rayMaxLength );
	color		= m_groundRaycastResult.m_didImpact ? Rgba8::GREEN : Rgba8::RED;
	AddVertsForArrow3D( verts, rayStartpos, rayEndpos, 0.05f, color );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsFootInFrontOfAnObstacle( float obstacleDistance ) const
{
	if ( m_footRaycastResult.m_didImpact == true )
	{
		if ( m_footRaycastResult.m_impactDist <= obstacleDistance )
		{
			return true;
		}
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsFootInRangeOfAnObstacle( FloatRange range ) const
{
	if ( range.IsOnRange( m_footRaycastResult.m_impactDist ) )
	{
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
bool Character::PressedActionButton() const
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateHeadRaycast()
{
	Vec3  rayStartPos	   = m_physics.m_position + Vec3( 0.f, 0.f, m_physics.m_height );
	Vec3  rayForwardnormal = m_physics.m_orientation.GetVector_XFwd();
	float rayLength		   = m_headraycastMaxDistance;

	for ( int index = 0; index < m_obstacles.size(); index++ )
	{
		ConvexHull3 const& obstacleHull = m_obstacles[ index ];
		m_headRaycast					= RaycastVsConvexHull3D( rayStartPos, rayForwardnormal, rayLength, obstacleHull );

		if ( m_headRaycast.m_didImpact )
		{
			// calculate the height of obstacle
			for (int planeIndex = 0; planeIndex < obstacleHull.m_boundingPlanes.size(); planeIndex++)
			{
				Plane3 plane = obstacleHull.m_boundingPlanes[ planeIndex ];
				Vec3   planeNormal = plane.m_normal;
				Vec3   skyward	   = Vec3( 0.f, 0.f, 1.f );
				float dotProduct = DotProduct3D( planeNormal, skyward );
				if (dotProduct >= 0.99f)
				{
					Vec3 pointOnPlane = planeNormal * plane.m_distanceFromOrigin;
					m_latestObstacleHeight = pointOnPlane.z;
					DebuggerPrintf( "plane height: %f\n", pointOnPlane.z );
					break;
				}
			}
			

			break;
		}
	}
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsHeadBlockedByAnObstacle() const
{
	if ( m_headRaycast.m_didImpact )
	{
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsHeadInRangeOfAnObstacle( FloatRange range ) const
{
	if ( m_headRaycast.m_didImpact )
	{
		float impactDistance = m_headRaycast.m_impactDist;
		if ( range.IsOnRange( impactDistance ) )
		{
			return true;
		}
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateShoulderRaycasts()
{
	Quaternion characterOrientation	   = m_physics.m_orientation;
	Vec3	   characterRightDirection = characterOrientation * Vec3( 0.f, -1.f, 0.f );
	Vec3	   characterLeftDirection  = characterOrientation.GetVector_YLeft();
	Vec3	   rightRaycastStartPos	   = m_physics.m_position + Vec3( characterRightDirection.x, characterRightDirection.y, m_physics.m_height * 0.5f );
	Vec3	   leftRaycastStartPos	   = m_physics.m_position + Vec3( characterLeftDirection.x, characterLeftDirection.y, m_physics.m_height * 0.5f );
	Vec3	   rayForwardnormal		   = m_physics.m_orientation.GetVector_XFwd();
	float	   rayLength			   = m_rightShoulderRaycastMaxDistance;

	// check for right shoulder raycast
	for ( int index = 0; index < m_obstacles.size(); index++ )
	{
		ConvexHull3 const& obstacleHull = m_obstacles[ index ];
		m_rightShoulderRaycastResult	= RaycastVsConvexHull3D( rightRaycastStartPos, rayForwardnormal, rayLength, obstacleHull );
		if ( m_rightShoulderRaycastResult.m_didImpact )
		{
			break;
		}
	}

	// check for left shoulder raycast
	for ( int index = 0; index < m_obstacles.size(); index++ )
	{
		ConvexHull3 const& obstacleHull = m_obstacles[ index ];
		m_leftShoulderRaycastResult		= RaycastVsConvexHull3D( leftRaycastStartPos, rayForwardnormal, rayLength, obstacleHull );
		if ( m_leftShoulderRaycastResult.m_didImpact )
		{
			break;
		}
	}
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsRightShoulderRaycastHittinSomething() const
{
	bool isRaycastHittingSomething = m_rightShoulderRaycastResult.m_didImpact;
	return isRaycastHittingSomething;
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsLeftShoulderRaycastHittinSomething() const
{
	bool isRaycastHittingSomething = m_leftShoulderRaycastResult.m_didImpact;
	return isRaycastHittingSomething;
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateGroundRaycast()
{
	if ( g_theCharacter->m_movementState->GetStateName() == "walk" )
	{
		m_groundRaycastStartOffset = Vec3( 0.45f, 0.f, -0.2f );
	}
	else if ( g_theCharacter->m_movementState->GetStateName() == "run" )
	{
		m_groundRaycastStartOffset = Vec3( 2.f, 0.f, -0.2f );
	}

	Vec3	   groundRaycastStartPos  = m_groundRaycastStartOffset;
	Quaternion characterOrientation	  = g_theThirdPersonController->GetCharacterOrientation();
	groundRaycastStartPos			  = characterOrientation * groundRaycastStartPos;
	Vec3 characterPos				  = g_theThirdPersonController->GetCharacterPosition();
	groundRaycastStartPos			 += characterPos;
	Vec3  groundRaycastFwdNormal	  = -1.f * characterOrientation.GetVector_ZUp();
	float raycastLegth				  = 1.5f;

	for ( int index = 0; index < m_obstacles.size(); index++ )
	{
		ConvexHull3 const& convexHull = m_obstacles[ index ];
		m_groundRaycastResult		  = RaycastVsConvexHull3D( groundRaycastStartPos, groundRaycastFwdNormal, raycastLegth, convexHull );
		if ( m_groundRaycastResult.m_didImpact )
		{
			break;
		}
	}
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsGroundRaycastHittingSomething() const
{
	if ( m_groundRaycastResult.m_didImpact )
	{
		return true;
	}
	else
	{
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------
bool Character::IsCharacterOnTheGround() const
{
	if ( m_groundRaycastResult.m_rayStartPos.z <= 0.f )
	{
		return true;
	}
	else
	{
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------
void Character::InitDebugRootMotionTranslation()
{
}


//----------------------------------------------------------------------------------------------------------
void Character::UpdateDebugRootMotionTranslation()
{
	/*if (m_movementState->m_applyRootMotionTranslation)
	{*/
	m_debugRootMotionTraslation.m_positionCurve	  = m_movementState->m_rootMotionTranslationCurve;
	m_debugRootMotionTraslation.m_sampleTime	  = m_movementState->m_sampleTimeMs;
	m_debugRootMotionTraslation.m_sampledPosition = m_movementState->m_debugSampledUnrotatedtranslation;
	//}
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugrenderZRootMotionTranslationUI( AABB2 const& uiBounds ) const
{
	float sampledTimeMax = m_debugRootMotionTraslation.m_positionCurve.GetEndTimeMilliSeconds();
	float zSampledvalue	 = m_debugRootMotionTraslation.m_sampledPosition.z;

	// for every point in the curve for z value, plot a point in the graph
	// find the smallest and largest z value
	float minZ = FLT_MAX;
	float maxZ = FLT_MIN;
	for ( unsigned int curveIndex = 0; curveIndex < m_debugRootMotionTraslation.m_positionCurve.GetSize(); curveIndex++ )
	{
		Vector3Keyframe const& keyFrameValue = m_debugRootMotionTraslation.m_positionCurve.m_keyframes[ curveIndex ];
		float				   zValue		 = keyFrameValue.m_value.z;

		if ( zValue < minZ )
		{
			minZ = zValue;
		}

		if ( zValue > maxZ )
		{
			maxZ = zValue;
		}
	}


	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;


	// graph
	AABB2 ui_graph = uiBounds /*.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f )*/;

	// z
	AABB2 uiTData  = ui_graph.GetBoxAtUVs( 0.f, 0.2f, 0.1f, 1.f );
	AABB2 uiTValue = uiTData.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiTValue, 16.f, Stringf( "%.2f", zSampledvalue ), Rgba8::RED, 1.f, Vec2( 0.5f, 0.5f ), TextBoxMode::OVERRUN );

	// fade ms
	AABB2 uiFadeMs = ui_graph.GetBoxAtUVs( 0.1f, 0.f, 1.f, 0.1f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiFadeMs, 20.f, "time Ms" );

	// easing function
	AABB2 uiEasingFunc = ui_graph.GetBoxAtUVs( 0.1f, 0.1f, 0.9f, 0.9f );


	// lerp data
	Vec2  lerpStart	   = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.2f ) );
	Vec2  lerpEnd	   = uiEasingFunc.GetPointAtUV( Vec2( 1.f, 1.f ) );

	float tCurrent	   = RangeMap( m_debugRootMotionTraslation.m_sampleTime, 0.f, sampledTimeMax, lerpStart.x, lerpEnd.x );
	float valueCurrent = RangeMap( m_debugRootMotionTraslation.m_sampledPosition.z, minZ, maxZ, lerpStart.y, lerpEnd.y );
	AddVertsForDisc2D( verts, Vec2( tCurrent, valueCurrent ), 10.f, Rgba8::RED );


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_simpleBitmapFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugrenderYRootMotionTranslationUI( AABB2 const& uiBounds ) const
{
	float sampledTimeMax = m_debugRootMotionTraslation.m_positionCurve.GetEndTimeMilliSeconds();
	float ySampledvalue	 = m_debugRootMotionTraslation.m_sampledPosition.y;

	// for every point in the curve for z value, plot a point in the graph
	// find the smallest and largest z value
	float minY = FLT_MAX;
	float maxY = FLT_MIN;
	for ( unsigned int curveIndex = 0; curveIndex < m_debugRootMotionTraslation.m_positionCurve.GetSize(); curveIndex++ )
	{
		Vector3Keyframe const& keyFrameValue = m_debugRootMotionTraslation.m_positionCurve.m_keyframes[ curveIndex ];
		float				   yValue		 = keyFrameValue.m_value.y;

		if ( yValue < minY )
		{
			minY = yValue;
		}

		if ( yValue > maxY )
		{
			maxY = yValue;
		}
	}


	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	// graph
	AABB2 ui_graph = uiBounds /*.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f )*/;
	AABB2 uiTData  = ui_graph.GetBoxAtUVs( 0.f, 0.2f, 0.1f, 1.f );
	AABB2 uiTValue = uiTData.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiTValue, 16.f, Stringf( "%.2f", ySampledvalue ), Rgba8::RED, 1.f, Vec2( 0.5f, 0.5f ), TextBoxMode::OVERRUN );

	// easing function
	AABB2 uiEasingFunc = ui_graph.GetBoxAtUVs( 0.1f, 0.1f, 0.9f, 0.9f );

	// lerp data
	Vec2  lerpStart	   = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.2f ) );
	Vec2  lerpEnd	   = uiEasingFunc.GetPointAtUV( Vec2( 1.f, 1.f ) );

	float tCurrent	   = RangeMap( m_debugRootMotionTraslation.m_sampleTime, 0.f, sampledTimeMax, lerpStart.x, lerpEnd.x );
	float valueCurrent = RangeMap( ySampledvalue, minY, maxY, lerpStart.y, lerpEnd.y );
	AddVertsForDisc2D( verts, Vec2( tCurrent, valueCurrent ), 10.f, Rgba8::RED );


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_simpleBitmapFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugrenderXRootMotionTranslationUI( AABB2 const& uiBounds ) const
{
	float sampledTimeMax = m_debugRootMotionTraslation.m_positionCurve.GetEndTimeMilliSeconds();
	float xSampledvalue	 = m_debugRootMotionTraslation.m_sampledPosition.x;

	// for every point in the curve for z value, plot a point in the graph
	// find the smallest and largest z value
	float minX = FLT_MAX;
	float maxX = FLT_MIN;
	for ( unsigned int curveIndex = 0; curveIndex < m_debugRootMotionTraslation.m_positionCurve.GetSize(); curveIndex++ )
	{
		Vector3Keyframe const& keyFrameValue = m_debugRootMotionTraslation.m_positionCurve.m_keyframes[ curveIndex ];
		float				   xValue		 = keyFrameValue.m_value.x;

		if ( xValue < minX )
		{
			minX = xValue;
		}

		if ( xValue > maxX )
		{
			maxX = xValue;
		}
	}


	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	// graph
	AABB2 ui_graph = uiBounds /*.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f )*/;
	AABB2 uiTData  = ui_graph.GetBoxAtUVs( 0.f, 0.2f, 0.1f, 1.f );
	AABB2 uiTValue = uiTData.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiTValue, 16.f, Stringf( "%.2f", xSampledvalue ), Rgba8::RED, 1.f, Vec2( 0.5f, 0.5f ), TextBoxMode::OVERRUN );

	// easing function
	AABB2 uiEasingFunc = ui_graph.GetBoxAtUVs( 0.1f, 0.1f, 0.9f, 0.9f );

	// lerp data
	Vec2  lerpStart	   = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.2f ) );
	Vec2  lerpEnd	   = uiEasingFunc.GetPointAtUV( Vec2( 1.f, 1.f ) );

	float tCurrent	   = RangeMap( m_debugRootMotionTraslation.m_sampleTime, 0.f, sampledTimeMax, lerpStart.x, lerpEnd.x );
	float valueCurrent = RangeMap( xSampledvalue, minX, maxX, lerpStart.y, lerpEnd.y );
	AddVertsForDisc2D( verts, Vec2( tCurrent, valueCurrent ), 10.f, Rgba8::RED );


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_simpleBitmapFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void Character::CreateRootMotionTranslationGraphVerts()
{
	ClearGraphVerts();

	m_debugRootMotionTraslation.m_positionCurve	  = m_movementState->m_rootMotionTranslationCurve;
	m_debugRootMotionTraslation.m_sampleTime	  = m_movementState->m_sampleTimeMs;
	m_debugRootMotionTraslation.m_sampledPosition = m_movementState->m_debugSampledUnrotatedtranslation;

	std::vector<Vertex_PCU> geometryVerts;
	std::vector<Vertex_PCU> textVerts;

	AABB2					screenBounds = g_theGame->m_screenCamera.GetOrthographicBounds();
	AABB2					zBounds		 = screenBounds.GetBoxAtUVs( AABB2( 0.7f, 0.f, 1.f, 0.3f ) );
	CreateAxisSpecificRootMotionTranslationGraphVerts( zBounds, AxisType::z, geometryVerts, textVerts );

	AABB2 yBounds = screenBounds.GetBoxAtUVs( AABB2( 0.7f, 0.3f, 1.f, 0.6f ) );
	CreateAxisSpecificRootMotionTranslationGraphVerts( yBounds, AxisType::y, geometryVerts, textVerts );

	AABB2 xBounds = screenBounds.GetBoxAtUVs( AABB2( 0.7f, 0.6f, 1.f, 0.9f ) );
	CreateAxisSpecificRootMotionTranslationGraphVerts( xBounds, AxisType::x, geometryVerts, textVerts );


	// copy cpu to gpu
	m_debugGeometryBuffer				 = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	m_debugGeometryBuffer->m_vertexCount = ( unsigned int ) geometryVerts.size();
	size_t geometryVertsSize			 = geometryVerts.size() * sizeof( Vertex_PCU );
	g_theRenderer->CopyCPUToGPU( geometryVerts.data(), geometryVertsSize, m_debugGeometryBuffer );

	m_debugTextbuffer				 = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	m_debugTextbuffer->m_vertexCount = ( unsigned int ) textVerts.size();
	size_t textVertsSize			 = textVerts.size() * sizeof( Vertex_PCU );
	g_theRenderer->CopyCPUToGPU( textVerts.data(), textVertsSize, m_debugTextbuffer );
}


//----------------------------------------------------------------------------------------------------------
void Character::CreateAxisSpecificRootMotionTranslationGraphVerts( AABB2 const& uiBounds, AxisType type, std::vector<Vertex_PCU>& geometryVerts, std::vector<Vertex_PCU>& textVerts )
{
	float sampledTimeMax = m_debugRootMotionTraslation.m_positionCurve.GetEndTimeMilliSeconds();

	// for every point in the curve for z value, plot a point in the graph
	// find the smallest and largest z value
	float minValue = FLT_MAX;
	float maxValue = FLT_MIN;
	for ( unsigned int curveIndex = 0; curveIndex < m_debugRootMotionTraslation.m_positionCurve.GetSize(); curveIndex++ )
	{
		Vector3Keyframe const& keyFrameValue = m_debugRootMotionTraslation.m_positionCurve.m_keyframes[ curveIndex ];
		float				   value;
		if ( type == AxisType::z )
		{
			value = keyFrameValue.m_value.z;
		}
		else if ( type == AxisType::y )
		{
			value = keyFrameValue.m_value.y;
		}
		else
		{
			value = keyFrameValue.m_value.x;
		}

		if ( value < minValue )
		{
			minValue = value;
		}

		if ( value > maxValue )
		{
			maxValue = value;
		}
	}

	AddVertsForAABB2( geometryVerts, uiBounds, Rgba8( 0, 0, 0, 255 ) );


	// graph
	AABB2 ui_graph = uiBounds /*.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f )*/;

	// z
	AABB2		uiTData = ui_graph.GetBoxAtUVs( 0.f, 0.2f, 0.1f, 1.f );
	AABB2		uiTText = uiTData.GetBoxAtUVs( 0.f, 0.3f, 1.f, 1.f );
	std::string axisName;
	if ( type == AxisType::z )
	{
		axisName = "z";
	}
	else if ( type == AxisType::y )
	{
		axisName = "y";
	}
	else
	{
		axisName = "x";
	}
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiTText, 20.f, axisName );


	// fade ms
	AABB2 uiFadeMs = ui_graph.GetBoxAtUVs( 0.1f, 0.f, 1.f, 0.1f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiFadeMs, 20.f, "time Ms" );

	// easing function
	AABB2 uiEasingFunc = ui_graph.GetBoxAtUVs( 0.1f, 0.1f, 0.9f, 0.9f );

	// y
	Vec2 verticalStart	 = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.f ) );
	Vec2 verticalEnd	 = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 1.f ) );
	Vec2 horizontalStart = uiEasingFunc.GetPointAtUV( Vec2( 0.02f, 0.2f ) );
	Vec2 horizontalEnd	 = uiEasingFunc.GetPointAtUV( Vec2( 1.f, 0.2f ) );


	AddVertsForLineSegment2D( geometryVerts, verticalStart, verticalEnd, 5.f, Rgba8::WHITE );
	AABB2 uiYAxisMax = uiEasingFunc.GetBoxAtUVs( -0.1f, 0.8f, 0.1f, 1.f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiYAxisMax, 20.f, Stringf( "%.1f", maxValue ) );

	// x

	AddVertsForLineSegment2D( geometryVerts, horizontalStart, horizontalEnd, 5.f, Rgba8::WHITE );
	AABB2 uiXAxisMin = uiEasingFunc.GetBoxAtUVs( -0.1f, 0.f, 0.1f, 0.2f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiXAxisMin, 20.f, Stringf( "%.1f", minValue ) );
	AABB2 uiXAxisMax = uiEasingFunc.GetBoxAtUVs( 0.7f, 0.f, 1.f, 0.1f );

	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiXAxisMax, 20.f, Stringf( "%.1f", sampledTimeMax ) );

	// lerp data
	Vec2 lerpStart = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.2f ) );
	Vec2 lerpEnd   = uiEasingFunc.GetPointAtUV( Vec2( 1.f, 1.f ) );
	// AddVertsForLineSegment2D( verts, lerpStart, lerpEnd, 5.f, Rgba8::WHITE );


	for ( unsigned int curveIndex = 0; curveIndex < m_debugRootMotionTraslation.m_positionCurve.GetSize(); curveIndex++ )
	{
		Vector3Keyframe const& keyFrameValue = m_debugRootMotionTraslation.m_positionCurve.m_keyframes[ curveIndex ];
		float				   sampledTime	 = keyFrameValue.m_timeMilliSeconds;

		float				   value;
		if ( type == AxisType::z )
		{
			value = keyFrameValue.m_value.z;
		}
		else if ( type == AxisType::y )
		{
			value = keyFrameValue.m_value.y;
		}
		else
		{
			value = keyFrameValue.m_value.x;
		}

		// range map sampledTime from (0.f, max) to horizontal axis
		float tMapped = RangeMap( sampledTime, 0.f, sampledTimeMax, lerpStart.x, lerpEnd.x );

		// range map z value from (min, max) to vertical axis
		float valueMapped = RangeMap( value, minValue, maxValue, lerpStart.y, lerpEnd.y );

		AddVertsForDisc2D( geometryVerts, Vec2( tMapped, valueMapped ), 3.f, Rgba8::WHITE );
	}
}


//----------------------------------------------------------------------------------------------------------
void Character::ClearGraphVerts()
{
	delete m_debugGeometryBuffer;
	m_debugGeometryBuffer = nullptr;

	delete m_debugTextbuffer;
	m_debugTextbuffer = nullptr;
}


//----------------------------------------------------------------------------------------------------------
void Character::DebugRenderGraphVerts() const
{
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexBuffer( m_debugGeometryBuffer, m_debugGeometryBuffer->m_vertexCount );

	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &( g_simpleBitmapFont->GetTexture() ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexBuffer( m_debugTextbuffer, m_debugTextbuffer->m_vertexCount );
}


//----------------------------------------------------------------------------------------------------------
void Character::InitSpriteLitShader()
{
	m_spriteLitShader = g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/SpriteLit", VertexType::Vertex_PCUTBN );
}


//----------------------------------------------------------------------------------------------------------
void Character::LoadMeshData()
{
	FbxFileImporter::LoadPreRiggedAndPreSkinnedMeshBindPoseFromFile( "Data/Meshes/XBotTPose.fbx", m_meshVerts, m_vertexJointIdWeightMapping );
	FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/XBotTPose.fbx", m_bindPose );
	m_bindPose.CalculateGlobalInverseBindPoseMatrices();
}


//----------------------------------------------------------------------------------------------------------
void Character::RenderMeshData( AnimPose const& sampledPose ) const
{
	if ( !m_renderMesh )
		return;

	// create the skinning matrix for each joint
	std::vector<Mat44> skinningMatricesByJointId;
	int				   numJoints = m_bindPose.GetNumberOfJoints();
	skinningMatricesByJointId.reserve( numJoints );
	for ( int jointIndex = 0; jointIndex < numJoints; jointIndex++ )
	{
		// get the matrix that transforms verts into skin space
		Mat44 const& inverseBindPoseTransformMatrix = m_bindPose.GetGlobalInverseBindPoseMatrixOfJoint( jointIndex );

		// calculate the matrix that transforms verts to new animated pos
		Transform animatedJointGlobalTransform = sampledPose.GetGlobalTransformOfJoint( jointIndex );
		animatedJointGlobalTransform.m_scale	 = Vec3( 1.f, 1.f, 1.f );
		Mat44 animatedJointBindPoseTransformMatrix	 = Mat44::CreateFromTransform( animatedJointGlobalTransform );

		// create skinning matrix = transform to skin space, then transform to new animated pos
		Mat44 skinningMatrix = animatedJointBindPoseTransformMatrix;
		skinningMatrix.Append( inverseBindPoseTransformMatrix );
		skinningMatricesByJointId.push_back( skinningMatrix );
	}

	// skinning
	std::vector<Vertex_PCUTBN> vertsTransformedToNewAnimatedPos = m_meshVerts;
	size_t					   numMeshVerts						= vertsTransformedToNewAnimatedPos.size();
	for ( size_t vertexNum = 0; vertexNum < numMeshVerts; vertexNum++ )
	{
		Vertex_PCUTBN&							  vertex				 = vertsTransformedToNewAnimatedPos[ vertexNum ];
		Vec3									  overallTransformedPos	 = Vec3::ZERO;
		std::vector<std::pair<int, float>> const& jointIdToWeightMapping = m_vertexJointIdWeightMapping[ vertexNum ];

		for ( auto iter = jointIdToWeightMapping.begin(); iter != jointIdToWeightMapping.end(); iter++ )
		{
			std::pair<int, float> const& mapping = *iter;
			int							 jointId = mapping.first;
			float						 weight	 = mapping.second;

			// transform vertex
			Mat44 const& jointSkinningMatrix = skinningMatricesByJointId[ jointId ];
			Vec3		 transformedPos		 = jointSkinningMatrix.TransformPosition3D( vertex.m_position );

			// add according to the weight influence, smooth skinning
			overallTransformedPos += ( weight * transformedPos );
		}

		vertex.m_position = overallTransformedPos;
	}

	// Bind pose mesh
	GameFinalShowcase* game = dynamic_cast<GameFinalShowcase*>(g_theGame);
	g_theRenderer->SetLightingConstatnts( game->m_lightConstants );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( m_spriteLitShader );
	/*g_theRenderer->DrawVertexArrayPCUTBN( m_meshVerts );*/

	// animated pose mesh
	Mat44 modelTransformMatrix;
	modelTransformMatrix.AppendTranslation3D( m_physics.m_position );
	modelTransformMatrix.Append( m_physics.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp() );
	g_theRenderer->SetModelConstants( modelTransformMatrix );
	g_theRenderer->DrawVertexArrayPCUTBN( vertsTransformedToNewAnimatedPos );
}


//----------------------------------------------------------------------------------------------------------
void Character::ToggleMeshRender()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F11 ) )
	{
		m_renderMesh = !m_renderMesh;
	}
}
