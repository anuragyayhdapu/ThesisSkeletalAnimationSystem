#pragma once

#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Animation/AnimCurve.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec3.hpp"

#include <vector>

class VertexBuffer;
class ThirdPersonController;
class AnimPose;
class MovementState;
class Map;


//----------------------------------------------------------------------------------------------------------
class Character
{
public:
	Character();
	~Character();

	void Startup();
	void CreateVerts();
	void Update();
	void Render() const;

	struct Physics
	{
		Vec3	   m_position	  = Vec3::ZERO;
		Quaternion m_orientation  = Quaternion::IDENTITY;
		float	   m_height		  = 2.f;
		float	   m_radius		  = 1.f;
		Vec3	   m_velocity	  = Vec3::ZERO;
		Vec3	   m_acceleration = Vec3::ZERO;
		Vec3	   GetForward() const;
	};
	Physics m_physics;

	struct Mesh
	{
		Mesh() {}
		~Mesh();

		VertexBuffer*			m_gpuVerts = nullptr;
		std::vector<Vertex_PCU> m_cpuVerts = {};
	};
	Mesh		    m_mesh;
	LightConstants* m_lightingConstants = nullptr;

	Mesh		   m_debugPositionMesh;
	Mesh		   m_debugUnitVectorMesh;
	bool		   m_showDebugRender = true;
	void		   DebugRender() const;
	void		   DebugRenderUI() const;
	void		   DebugRenderPhysicsSphere() const;
	void		   DebugRenderBasisVectors() const;

	void		   UpdateAnimations();
	void		   RenderAnimations() const;
	void		   RenderPose( AnimPose const& pose, Rgba8 color ) const;

	MovementState* m_movementState = nullptr;
	void		   InitMovementState();
	void		   UpdateMovementState();
	void		   DebugRenderCurrentMovementState() const;

	// raycast
	float m_latestObstacleHeight = 0.f;
	struct RaycastResult3DEx : public RaycastResult3D
	{
		float m_obstacleHeight = 0.f;

		RaycastResult3DEx()	   = default;

		RaycastResult3DEx( RaycastResult3D result )
		{
			m_didImpact	   = result.m_didImpact;
			m_impactDist   = result.m_impactDist;
			m_impactPos	   = result.m_impactPos;
			m_impactNormal = result.m_impactNormal;
			m_rayFwdNormal = result.m_rayFwdNormal;
			m_rayStartPos  = result.m_rayStartPos;
			m_rayMaxLength = result.m_rayMaxLength;
		}
	};
	Map*					 m_map = nullptr;
	RaycastResult3DEx		 m_footRaycastResult;
	RaycastResult3DEx		 m_headRaycast;
	RaycastResult3DEx		 m_rightShoulderRaycastResult;
	RaycastResult3DEx		 m_leftShoulderRaycastResult;
	RaycastResult3DEx		 m_groundRaycastResult;
	float					 m_footRaycastMaxDistance		   = 3.f;
	float					 m_headraycastMaxDistance		   = 3.f;
	float					 m_rightShoulderRaycastMaxDistance = 2.f;
	float					 m_lefttShoulderRaycastMaxDistance = 2.f;
	Vec3					 m_groundRaycastStartOffset		   = Vec3( 0.65f, 0.f, -0.2f );
	float					 m_groundRaycastMaxDistance		   = 0.5f;
	std::vector<ConvexHull3> m_obstacles;
	void					 InitRaycast();
	void					 UpdateFootRaycast();
	void					 DebugRenderRaycast() const;
	bool					 IsFootInFrontOfAnObstacle( float obstacleDistance ) const;
	bool					 IsFootInRangeOfAnObstacle( FloatRange range ) const;
	bool					 PressedActionButton() const;
	void					 UpdateHeadRaycast();
	bool					 IsHeadBlockedByAnObstacle() const;
	bool					 IsHeadInRangeOfAnObstacle( FloatRange range ) const;
	void					 UpdateShoulderRaycasts();
	bool					 IsRightShoulderRaycastHittinSomething() const;
	bool					 IsLeftShoulderRaycastHittinSomething() const;
	void					 UpdateGroundRaycast();
	bool					 IsGroundRaycastHittingSomething() const;
	bool					 IsCharacterOnTheGround() const;

	// debug rendering of root motion translation curves
	struct DebugRootMotionTranslation
	{
		Vec3AnimCurve m_positionCurve;
		float		  m_sampleTime = 0.f;
		Vec3		  m_sampledPosition;
	};
	DebugRootMotionTranslation m_debugRootMotionTraslation;
	void					   InitDebugRootMotionTranslation();
	void					   UpdateDebugRootMotionTranslation();
	void					   DebugrenderZRootMotionTranslationUI( AABB2 const& uiBounds ) const;
	void					   DebugrenderYRootMotionTranslationUI( AABB2 const& uiBounds ) const;
	void					   DebugrenderXRootMotionTranslationUI( AABB2 const& uiBounds ) const;

	enum AxisType
	{
		x,
		y,
		z
	};
	VertexBuffer* m_debugTextbuffer		= nullptr;
	VertexBuffer* m_debugGeometryBuffer = nullptr;
	void		  CreateRootMotionTranslationGraphVerts();
	void		  CreateAxisSpecificRootMotionTranslationGraphVerts( AABB2 const& uiBounds, AxisType type, std::vector<Vertex_PCU>& geometryVerts, std::vector<Vertex_PCU>& textVerts );
	void		  ClearGraphVerts();
	void		  DebugRenderGraphVerts() const;

	// skinning mesh data
	std::vector<Vertex_PCUTBN>						m_meshVerts;
	std::vector<std::vector<std::pair<int, float>>> m_vertexJointIdWeightMapping;
	Shader*											m_spriteLitShader = nullptr;
	void											InitSpriteLitShader();
	void											LoadMeshData();
	void											RenderMeshData(AnimPose const& sampledPose) const;
	AnimPose										m_bindPose;
	bool											m_renderMesh = true;
	void											ToggleMeshRender();
};