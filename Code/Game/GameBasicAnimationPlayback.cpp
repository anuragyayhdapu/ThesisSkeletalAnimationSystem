#include "Game/Player.hpp"
#include "Game/GameBasicAnimationPlayback.hpp"
#include "Game/App.hpp"

#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
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


void GameBasicAnimationPlayback::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();

	PrintTitleText( "Basic Animation Playback" );


	// load fbx file
	// fbxFileImporter = new FbxFileImporter( "Data/Animations/CarefulWalking.fbx" );
	// fbxFileImporter = new FbxFileImporter( "Data/Animations/Granny_Skin_Jab_Cross.fbx" );
	fbxFileImporter = new FbxFileImporter( "Data/Animations/Default/Default_Jab_Cross.fbx" );
	m_pose			= fbxFileImporter->m_restPose;

	LogAnimationData();
	g_theDevConsole->ToggleOpen( true );
}


//----------------------------------------------------------------------------------------------------------
void GameBasicAnimationPlayback::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameBasicAnimationPlayback::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();
}


//----------------------------------------------------------------------------------------------------------
void GameBasicAnimationPlayback::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );


	float totalSeconds = m_GameClock->GetTotalSeconds();

	AnimPose  animatedPose = m_pose;
	AnimClip* animClip	   = fbxFileImporter->m_animClip;
	animClip->Sample( totalSeconds * 1000.f, animatedPose );

	std::vector<Vertex_PCU> verts;

	// for each transform in the pose, debug draw line from parent to child
	for ( int index = 0; index < animatedPose.GetNumberOfJoints(); index++ )
	{
		Transform const& self		 = animatedPose.GetGlobalTransformOfJoint( index );
		int				 parentIndex = animatedPose.GetParentOfJoint( index );
		if ( parentIndex < 0 )
		{
			continue;
		}
		Transform const& parent = animatedPose.GetGlobalTransformOfJoint( parentIndex );

		// AddVertsForCylinder3D( verts, self.m_position, parent.m_position, 1.f );
		AddVertsForCone3D( verts, parent.m_position, self.m_position, 0.03f );
		// DebugAddWorldLine( self.m_position, parent.m_position, 1.f, 0.f );
	}

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


//----------------------------------------------------------------------------------------------------------
void GameBasicAnimationPlayback::LogAnimationData()
{
	AnimClip const& animClip = *fbxFileImporter->m_animClip;

	std::string name = animClip.m_name;

	LogLine( "" );
	LogLine( "" );
	LogLine( "-----------------------------------------------" );
	LogLine( "name: " + animClip.m_name );
	LogLine( Stringf( "|- start time: %.2f", animClip.GetStartTime() ) );
	LogLine( Stringf( "|- end time: %.2f", animClip.GetEndTime() ) );

	for ( int jointIndex = 0; jointIndex < animClip.m_animChannels.size(); jointIndex++ )
	{
		AnimChannel const& jointAnimData = animClip.m_animChannels[ jointIndex ];

		LogLine( Stringf( "|---- Joint ID: %d", jointAnimData.m_jointId ) );
		LogLine( Stringf( "| |-- Joint Name: %s", jointAnimData.m_jointName.c_str() ) );
		LogLine( Stringf( "| |-- Position Keyframes: %d", jointAnimData.m_positionCurve.GetSize() ) );
		LogLine( Stringf( "| |-- Rotation Keyframes: %d", jointAnimData.m_rotationCurve.GetSize() ) );
		LogLine( Stringf( "| |-- Scale Keyframes: %d", jointAnimData.m_scaleCurve.GetSize() ) );
	}

	LogLine( "-----------------------------------------------" );
}


//----------------------------------------------------------------------------------------------------------
void GameBasicAnimationPlayback::LogLine( std::string const& logLine )
{
	DebuggerPrintf( Stringf( "%s \n", logLine.c_str() ).c_str() );
	g_theDevConsole->AddLine( DevConsole::INFO_MINOR_COLOR, logLine );
}
