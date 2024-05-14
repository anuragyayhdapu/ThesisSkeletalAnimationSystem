#include "Game/Player.hpp"
#include "Game/GameAdditiveBlending.hpp"
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


void GameAdditiveBlending::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();

	PrintTitleText( "Additive Blending" );

	// add title text for clips
	DebugAddWorldTextHelper( "Reference Clip", Vec3( 0.f, 0.f, 3.9f ) );
	DebugAddWorldTextHelper( "Walking", Vec3( 0.f, 0.f, 3.7f ) );
	DebugAddWorldTextHelper( "Source Clip", Vec3( 3.f, 0.f, 3.9f ) );
	DebugAddWorldTextHelper( "Crouched Walking", Vec3( 3.f, 0.f, 3.7f ) );
	DebugAddWorldTextHelper( "Target Clip", Vec3( 6.f, 0.f, 3.9f ) );
	DebugAddWorldTextHelper( "Running", Vec3( 6.f, 0.f, 3.7f ) );
	DebugAddWorldTextHelper( "Additive Clip", Vec3( 9.f, 0.f, 3.9f ) );
	DebugAddWorldTextHelper( "Crouched Running", Vec3( 9.f, 0.f, 3.7f ) );

	m_walkingPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_walkingPose );
	m_crouchedWalking = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_crouchedWalking );
	m_differencePose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_differencePose );
	m_runningPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_runningPose );
	m_crouchedRunningPose = new AnimPose();
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/Default/Default_T_Pose.fbx", *m_crouchedRunningPose );

	m_walkingClip				= new AnimClip();
	m_walkingClip->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Walking.fbx", *m_walkingClip );

	m_crouchedWalkingClip				= new AnimClip();
	m_crouchedWalkingClip->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Crouched_Walking.fbx", *m_crouchedWalkingClip );

	m_runningClip			   = new AnimClip();
	m_runningClip->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/Default/Default_Running.fbx", *m_runningClip );
}


//----------------------------------------------------------------------------------------------------------
void GameAdditiveBlending::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameAdditiveBlending::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();

	AdditiveBlend();
}


//----------------------------------------------------------------------------------------------------------
void GameAdditiveBlending::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );

	RenderPose( *m_walkingPose, Vec2( 0.f, 0.f ), Rgba8::WHITE );
	RenderPose( *m_crouchedWalking, Vec2( 3.f, 0.f ), Rgba8::LAVENDER );
	RenderPose( *m_runningPose, Vec2( 6.f, 0.f ), Rgba8::LIGHT_CORAL );
	RenderPose( *m_crouchedRunningPose, Vec2( 9.f, 0.f ), Rgba8::DUSTY_ROSE );

	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	DebugRenderScreen( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void GameAdditiveBlending::AdditiveBlend()
{
	// update blend value
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_blendValue -= 0.01f;
	}

	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_blendValue += 0.01f;
	}

	float deltaTimeInSeconds	  = m_GameClock->GetDeltaSeconds();
	float deltaTimeInMilliSeconds = deltaTimeInSeconds * 1000.f;
	m_animationTimeInMilliSeconds += deltaTimeInMilliSeconds;

	m_walkingClip->Sample( m_animationTimeInMilliSeconds, *m_walkingPose );
	m_crouchedWalkingClip->Sample( m_animationTimeInMilliSeconds, *m_crouchedWalking );
	m_runningClip->Sample( m_animationTimeInMilliSeconds, *m_runningPose );

	// 1. calculate the difference between source clip and reference clip
	m_crouchedWalking->GetDifference( *m_walkingPose, *m_differencePose );
	//m_walkingPose->GetDifference( *m_crouchedWalking, *m_differencePose );

	// 2. apply difference pose to walking animation
	m_runningPose->GetAddition( *m_differencePose, *m_crouchedRunningPose );
	//m_differencePose->GetAddition( *m_runningPose, *m_crouchedRunningPose );




	// AnimPose::Blend( *m_animPose, *m_animPose, *m_headTurnPose, m_blendValue, -1 );

	// debug screen print
	float		duration		 = 0.f; // one frame
	Vec2		topLeftAlignment = Vec2( 0.f, 1.f );
	float		fontSize		 = 15.f;
	AABB2		cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float		textMinX = cameraBounds.m_mins.x;
	float		textMinY = cameraBounds.m_maxs.y - fontSize;
	Vec2		topLeftLinePosition( textMinX, textMinY );
	std::string playerOrientationStr = Stringf( "Blend Value: %.2f ", m_blendValue );
	DebugAddScreenText( playerOrientationStr, topLeftLinePosition - Vec2( 0.f, 2.f * fontSize ), fontSize, topLeftAlignment, duration );
}


//----------------------------------------------------------------------------------------------------------
void GameAdditiveBlending::RenderPose( AnimPose const& pose, Vec2 offsetXY, Rgba8 color ) const
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

	Mat44 modelConstant;
	modelConstant.SetTranslation2D( offsetXY );
	g_theRenderer->SetModelConstants( modelConstant, color );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
}
