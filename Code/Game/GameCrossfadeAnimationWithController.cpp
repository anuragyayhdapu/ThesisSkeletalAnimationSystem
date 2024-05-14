#include "Game/Player.hpp"
#include "Game/GameCrossfadeAnimationWithController.hpp"
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


void GameCrossfadeAnimationWithController::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();

	PrintTitleText( "Cross fading Animations\n With Controller" );

	m_animPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_animPose );

	m_animClipWalking			   = new AnimClip();
	m_animClipWalking->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Walking.fbx", *m_animClipWalking );

	m_animClipRunning			   = new AnimClip();
	m_animClipRunning->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Running.fbx", *m_animClipRunning );

	m_animClipJumping			   = new AnimClip();
	m_animClipJumping->m_isLooping = false;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Jump.fbx", *m_animClipJumping );

	m_animClipIdle				   = new AnimClip();
	m_animClipIdle->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Standing_Idle.fbx", *m_animClipIdle );
	

	m_animCrossfadeController.m_currentAnimation   = m_animClipWalking;
	m_animCrossfadeController.m_currentSampledPose = *m_animPose;
}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationWithController::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationWithController::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();

	float deltaTimeInSeconds	  = m_GameClock->GetDeltaSeconds();
	float deltaTimeInMilliSeconds = deltaTimeInSeconds * 1000.f;
	m_animationTimeInMilliSeconds += deltaTimeInMilliSeconds;

	m_animCrossfadeController.Update( deltaTimeInSeconds );
	m_animPose = &m_animCrossfadeController.m_currentSampledPose;

	if ( g_theInput->WasKeyJustPressed( '0' ) )
	{
		AnimCrossFadeTarget newTarget;
		newTarget.m_targetAnimation			 = m_animClipIdle;
		newTarget.m_sampledPose				 = *m_animPose;
		newTarget.m_fadeDurationMilliseconds = 1000.f;
		newTarget.m_playbackTimeMilliseconds = m_animClipIdle->GetStartTime();

		m_animCrossfadeController.m_targets.push_back( newTarget );
	}
	if (g_theInput->WasKeyJustPressed('1'))
	{
		AnimCrossFadeTarget newTarget;
		newTarget.m_targetAnimation			 = m_animClipWalking;
		newTarget.m_sampledPose				 = *m_animPose;
		newTarget.m_fadeDurationMilliseconds = 1000.f;
		newTarget.m_playbackTimeMilliseconds = m_animClipWalking->GetStartTime();

		m_animCrossfadeController.m_targets.push_back( newTarget );
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		AnimCrossFadeTarget newTarget;
		newTarget.m_targetAnimation			 = m_animClipRunning;
		newTarget.m_sampledPose				 = *m_animPose;
		newTarget.m_fadeDurationMilliseconds = 1000.f;
		newTarget.m_playbackTimeMilliseconds = m_animClipRunning->GetStartTime();

		m_animCrossfadeController.m_targets.push_back( newTarget );
	}
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		AnimCrossFadeTarget newTarget;
		newTarget.m_targetAnimation			 = m_animClipJumping;
		newTarget.m_sampledPose				 = *m_animPose;
		newTarget.m_fadeDurationMilliseconds = 1000.f;
		newTarget.m_playbackTimeMilliseconds = m_animClipJumping->GetStartTime();

		m_animCrossfadeController.m_targets.push_back( newTarget );
	}
}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationWithController::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );


	RenderPose( *m_animPose, Rgba8::WHITE );


	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	DebugRenderScreen( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}



//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationWithController::RenderPose( AnimPose const& pose, Rgba8 color ) const
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

	/*Mat44 hackTransform;
	hackTransform.AppendXRotation( 90.f );
	hackTransform.AppendScaleUniform3D( 0.02f );*/

	g_theRenderer->SetModelConstants( Mat44(), color );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
}
