#include "Game/Player.hpp"
#include "Game/GamePoseBlending.hpp"
#include "Game/App.hpp"

#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"


constexpr int			MAX_VERTEXAS		 = 3;
constexpr unsigned char MIN_TEST_COLOR_VALUE = 5;
constexpr unsigned char MAX_TEST_COLOR_VALUE = 255;

extern App* g_theApp;


void GamePoseBlending::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();

	PrintTitleText( "Pose Blending" );


	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/XBot/TPose.fbx", m_pose1 );
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/XBot/TPose.fbx", m_pose2 );
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/XBot/TPose.fbx", m_blendedPose );

	AnimClip animClip1;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/XBot/Run.fbx", animClip1 );

	AnimClip animClip2;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/XBot/Parkour/Vault.fbx", animClip2 );

	animClip1.Sample( 0.f, m_pose1 );
	animClip2.Sample( animClip2.GetEndTime() * 0.2f, m_pose2 );

	AnimPose::Blend( m_blendedPose, m_pose1, m_pose2, m_blendValue, -1 );
}


//----------------------------------------------------------------------------------------------------------
void GamePoseBlending::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GamePoseBlending::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();

	UpdateBlendValue();
}


//----------------------------------------------------------------------------------------------------------
void GamePoseBlending::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );

	RenderPose( m_pose1, Rgba8::RED );
	RenderPose( m_pose2, Rgba8::BLUE );

	RenderPose( m_blendedPose, Rgba8::WHITE );
	// RenderPose( m_blendedPose );
	// RenderPose( m_pose1 );
	// RenderPose( m_pose2 );




	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	DebugRenderScreen( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}



//----------------------------------------------------------------------------------------------------------
void GamePoseBlending::UpdateBlendValue()
{
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		// increase by 10%
		m_blendValue += 0.1f;
		m_blendValue = GetClampedZeroToOne( m_blendValue );

		AnimPose::Blend( m_blendedPose, m_pose1, m_pose2, m_blendValue, -1 );
	}

	if ( g_theInput->IsKeyDown( '9' ) )
	{
		// decrease by 10%
		m_blendValue -= 0.1f;
		m_blendValue = GetClampedZeroToOne( m_blendValue );

		AnimPose::Blend( m_blendedPose, m_pose1, m_pose2, m_blendValue, -1 );
	}
}


//----------------------------------------------------------------------------------------------------------
void GamePoseBlending::RenderPose( AnimPose const& pose, Rgba8 color ) const
{
	std::vector<Vertex_PCU> verts;

	// for each transform in the pose, debug draw line from parent to child
	for ( int index = 0; index < pose.GetNumberOfJoints(); index++ )
	{
		Transform const& self		 = pose.GetGlobalTransformOfJoint( index );
		int				 parentIndex = pose.GetParentOfJoint( index );
		if ( parentIndex < 0 )
		{
			continue;
		}
		Transform const& parent = pose.GetGlobalTransformOfJoint( parentIndex );

		// AddVertsForCylinder3D( verts, self.m_position, parent.m_position, 1.f );
		AddVertsForCone3D( verts, parent.m_position, self.m_position, 0.03f );
		// DebugAddWorldLine( self.m_position, parent.m_position, 1.f, 0.f );
	}

	g_theRenderer->SetModelConstants( Mat44(), color );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
}
