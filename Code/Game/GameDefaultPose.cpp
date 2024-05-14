#include "Game/Player.hpp"
#include "Game/GameDefaultPose.hpp"
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


void GameDefaultPose::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	//AddBasisAtOrigin();

	PrintTitleText( "Default Pose" );


	// load fbx file
	// fbxFileImporter = new FbxFileImporter( "Data/Animations/CarefulWalking.fbx" );
	// fbxFileImporter = new FbxFileImporter( "Data/Animations/Granny_Skin_Jab_Cross.fbx" );
	//fbxFileImporter = new FbxFileImporter( "Data/Animations/Default/Default_T_Pose.fbx" );
	fbxFileImporter = new FbxFileImporter( "Data/Animations/XBot/TPose.fbx" );
	m_pose			= fbxFileImporter->m_restPose;



	
}


//----------------------------------------------------------------------------------------------------------
void GameDefaultPose::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameDefaultPose::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();
}


//----------------------------------------------------------------------------------------------------------
void GameDefaultPose::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );


	//float totalSeconds = m_GameClock->GetTotalSeconds();

	//AnimPose  animatedPose = m_pose;
	//AnimClip* animClip	   = fbxFileImporter->m_animClip;
	//animClip->Sample( totalSeconds * 1000.f, animatedPose );

	std::vector<Vertex_PCU> verts;

	// for each transform in the pose, debug draw line from parent to child
	for ( int index = 0; index < m_pose.GetNumberOfJoints(); index++ )
	{
		Transform const& self		 = m_pose.GetGlobalTransformOfJoint( index );
		int				 parentIndex = m_pose.GetParentOfJoint( index );
		if ( parentIndex < 0 )
		{
			continue;
		}
		Transform const& parent = m_pose.GetGlobalTransformOfJoint( parentIndex );

		// AddVertsForCylinder3D( verts, self.m_position, parent.m_position, 1.f );
		AddVertsForCone3D( verts, parent.m_position, self.m_position, 0.03f );
		// DebugAddWorldLine( self.m_position, parent.m_position, 1.f, 0.f );
	}

	/*Mat44 hackTransform;
	hackTransform.AppendXRotation( 90.f );
	hackTransform.AppendScaleUniform3D( 0.02f );

	g_theRenderer->SetModelConstants( hackTransform );*/
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::WHITE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );





	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	DebugRenderScreen( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}