#include "Game/Player.hpp"
#include "Game/GameCrossfadeAnimationSmoothLinear.hpp"
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


void GameCrossfadeAnimationSmoothLinear::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();

	PrintTitleText( "Cross fading Animations\n Smooth Linear (Manual)" );

	m_animPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_animPose );

	m_currentClipPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_currentClipPose );

	m_nextClipPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_nextClipPose );


	// m_firstClipLastPose = new AnimPose();
	// FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default_T_Pose.fbx", *m_firstClipLastPose );

	// m_secondClipFirstPose = new AnimPose();
	// FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default_T_Pose.fbx", *m_secondClipFirstPose );

	m_animClipWalking			   = new AnimClip();
	m_animClipWalking->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Walking.fbx", *m_animClipWalking );

	m_animClipRunning			   = new AnimClip();
	m_animClipRunning->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Running.fbx", *m_animClipRunning );

	/*float firstClipEndTimeMilliSeconds = m_animClipWalking->GetEndTime();
	m_animClipWalking->Sample( firstClipEndTimeMilliSeconds, *m_firstClipLastPose );

	m_animClipRunning->Sample( 0.f, *m_secondClipFirstPose );*/

	m_currentClip = m_animClipWalking;
	m_nextClip	  = m_animClipRunning;
}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationSmoothLinear::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationSmoothLinear::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();

	// UpdateBlendValue();

	float timeInSeconds		 = m_GameClock->GetDeltaSeconds();
	float timeInMilliSeconds = timeInSeconds * 1000.f;
	m_animationTimeInMilliSeconds += timeInMilliSeconds;

	// float walkingClipEndTimeMilliSeconds = m_animClipWalking->GetEndTime();

	float animationBlendDuration = 1.f;

	switch ( m_animState )
	{
	case GameCrossfadeAnimationSmoothLinear::AnimState::Animating: {

		m_currentClip->Sample( m_animationTimeInMilliSeconds, *m_animPose );

		if (g_theInput->WasKeyJustPressed('N'))
		{
			/**m_currentClipLastPose = *m_animPose;
			m_nextClip->Sample( 0.f, *m_nextClipFirstPose );*/

			m_animState = AnimState::Blending;
		}

		break;
	}

	case GameCrossfadeAnimationSmoothLinear::AnimState::Blending: {

		float blendValue = RangeMap( m_blendTimeInSeconds, 0.f, animationBlendDuration, 0.f, 1.f );

		m_currentClip->Sample( m_animationTimeInMilliSeconds, *m_currentClipPose );
		m_nextClip->Sample( m_blendTimeInSeconds * 1000.f, *m_nextClipPose );

		AnimPose::Blend( *m_animPose, *m_currentClipPose, *m_nextClipPose, blendValue, -1 );

		float deltaSeconds = m_GameClock->GetDeltaSeconds();
		float nextBlendTimeInSeconds = m_blendTimeInSeconds + deltaSeconds;
		
		if ( nextBlendTimeInSeconds > animationBlendDuration )
		{
			m_animState					  = AnimState::Animating;
			m_animationTimeInMilliSeconds = m_blendTimeInSeconds * 1000.f;
			m_blendTimeInSeconds		  = 0.f;

			AnimClip* tempCurrentClip = m_currentClip;
			m_currentClip			  = m_nextClip;
			m_nextClip				  = tempCurrentClip;
		}
		else
		{
			m_blendTimeInSeconds = nextBlendTimeInSeconds;
		}

		break;
	}

	default:
		break;
	}

	
}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationSmoothLinear::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );

	/*RenderPose( m_pose1, Rgba8::RED );
	RenderPose( m_pose2, Rgba8::BLUE );

	RenderPose( m_blendedPose, Rgba8::WHITE );*/


	RenderPose( *m_animPose, Rgba8::WHITE );


	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	DebugRenderScreen( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}



//----------------------------------------------------------------------------------------------------------
// void GameCrossfadeAnimation::UpdateBlendValue()
//{
//	if ( g_theInput->IsKeyDown( '0' ) )
//	{
//		// increase by 10%
//		m_blendValue += 0.1f;
//		m_blendValue = GetClampedZeroToOne( m_blendValue );
//
//		AnimPose::Blend( m_blendedPose, m_pose1, m_pose2, m_blendValue, -1 );
//	}
//
//	if ( g_theInput->IsKeyDown( '9' ) )
//	{
//		// decrease by 10%
//		m_blendValue -= 0.1f;
//		m_blendValue = GetClampedZeroToOne( m_blendValue );
//
//		AnimPose::Blend( m_blendedPose, m_pose1, m_pose2, m_blendValue, -1 );
//	}
//}


//----------------------------------------------------------------------------------------------------------
void GameCrossfadeAnimationSmoothLinear::RenderPose( AnimPose const& pose, Rgba8 color ) const
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
