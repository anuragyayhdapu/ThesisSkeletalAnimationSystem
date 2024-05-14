#include "Game/Map.hpp"
#include "Game/AnimationState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/Player.hpp"
#include "Game/GameFinalShowcase.hpp"
#include "Game/App.hpp"

#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"


constexpr int			MAX_VERTEXAS		 = 3;
constexpr unsigned char MIN_TEST_COLOR_VALUE = 5;
constexpr unsigned char MAX_TEST_COLOR_VALUE = 255;

extern App* g_theApp;


void GameFinalShowcase::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	 //AddBasisAtOrigin();

	PrintTitleText( "Final Showcase\n F6: Lock/Free Camera, F5: Camera Freeze, F9: Toggle Debug Drawing" );
	CreateObstacle();

	InitLightConstants();

	AnimationState::LoadAnimationStateFromXML( "Data/Animations/AnimConfig.xml" );

	g_theCharacter = new Character();
	g_theCharacter->m_map = m_map;
	g_theCharacter->Startup();
	g_theCharacter->CreateVerts();

	g_theThirdPersonController = new ThirdPersonController();

	g_theAnimationController = new AnimationController();
	g_theAnimationController->Startup();
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::Shutdown()
{
	delete m_player;

	g_theAnimationController->Shutdown();
	delete g_theAnimationController;
	g_theAnimationController = nullptr;

	delete g_theThirdPersonController;
	g_theThirdPersonController = nullptr;

	delete g_theCharacter;
	g_theCharacter = nullptr;
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();
	UpdateLightConstants();

	g_theThirdPersonController->Update();
	g_theCharacter->Update();
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *( g_theThirdPersonController->m_worldCamera ) );

	RenderGridLines();

	DebugRenderWorld( *( g_theThirdPersonController->m_worldCamera ) );

	g_theThirdPersonController->Render();
	g_theCharacter->Render();

	RenderObstacle();

	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera ); 
	// add text / UI code here

	g_theAnimationController->DebugRender();
	g_theCharacter->DebugRenderUI();

	DebugRenderScreen( m_screenCamera );


	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::PrintDebugScreenMessage()
{
	float fontSize = 15.f;
	AABB2 cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float textMinX = cameraBounds.m_mins.x;
	float textMinY = cameraBounds.m_maxs.y - fontSize;
	float duration = 0.f; // one frame
	
	// time and fps
	textMinX = cameraBounds.m_maxs.x;
	textMinY = cameraBounds.m_maxs.y - fontSize;
	Vec2  topRightLinePosition( textMinX, textMinY );
	Vec2  topRightAlignment = Vec2( 1.f, 1.f );
	float totalSeconds		= m_GameClock->GetTotalSeconds();

	float deltaSeconds = m_GameClock->GetDeltaSeconds();
	float fps		   = 1.f / deltaSeconds;
	float ms		   = deltaSeconds * 1000.f;

	std::string timeValuesStr = Stringf( "Time: %.2f, FPS: %.1f, MS: %.1f", totalSeconds, fps, ms );
	DebugAddScreenText( timeValuesStr, topRightLinePosition, fontSize, topRightAlignment, duration );
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::CreateObstacle()
{
	AABB3 vaultObstacle = AABB3( Vec3( 10.f, -20.f, 0.f ), Vec3( 11.5f, 0.f, 1.4f ) );
	AABB3 vaultObstacle2 = AABB3( Vec3( 20.f, -30.f, 0.f ), Vec3( 30.f, -28.5f, 1.4f ) );
	AABB3 vaultObstacle3 = AABB3( Vec3( -10.f, -45.f, 0.f ), Vec3( -8.5f, -8.5f, 1.4f ) );
	AABB3 vaultObstacle4 = AABB3( Vec3( -30.f, 0.f, 0.f ), Vec3( -28.5f, 30.f, 1.4f ) );
	AABB3 vaultObstacle5 = AABB3( Vec3( 35.f, 20.f, 0.f ), Vec3( 36.5f, 40.f, 1.4f ) );
	

	AABB3 ledgeGrabObstacle = AABB3( Vec3( -45.f, -50.f, 0.f ), Vec3( -25.f, -30.f, 5.f) );
	AABB3 ledgeGrabObstacle2 = AABB3( Vec3( -35.f, 25.f, 0.f ), Vec3( -5.f, 45.f,	3.25f) );
	AABB3 ledgeGrabObstacle3 = AABB3( Vec3( 10.f, 10.f, 0.f ), Vec3( 25.f, 25.f,	4.f) );
	
	// create map
	m_map = new Map( std::vector<AABB3>{ vaultObstacle, vaultObstacle2, vaultObstacle3, vaultObstacle4, vaultObstacle5,
		ledgeGrabObstacle, ledgeGrabObstacle2, ledgeGrabObstacle3 } );
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::RenderObstacle() const
{
	std::vector<Vertex_PCU> verts;

	for (int index = 0; index < m_map->m_obstacleAABBs.size(); index++)
	{
		AABB3 const& obstacle = m_map->m_obstacleAABBs[ index ];

		AddVertsForAABB3D( verts, obstacle, Rgba8::DUSTY_ROSE );
		AddVertsForWireframeAABB3D( verts, obstacle, 0.01f, Rgba8::RED );
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::InitLightConstants()
{
	m_lightControlClock = new Clock();
	// m_sunRotation						 = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), 90.f );
	m_sunRotation = Quaternion( 0.289f, 0.914f, -0.277f, -0.058f );
	m_sunRotation.Normalize();

	m_lightConstants.m_sunDirection		 = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	m_lightConstants.m_sunIntensity		 = 0.6f;
	m_lightConstants.m_ambientIntensity	 = 0.4f;
	m_lightConstants.m_worldEyePosition	 = g_theGame->m_player->m_position;
	m_lightConstants.m_normalMode		 = 0;
	m_lightConstants.m_specularMode		 = 0;
	m_lightConstants.m_specularIntensity = 1.f;
	m_lightConstants.m_specularPower	 = 32.f;
}


//----------------------------------------------------------------------------------------------------------
void GameFinalShowcase::UpdateLightConstants()
{
	float deltaSeconds = m_lightControlClock->GetDeltaSeconds();

	// Pitch the sun direction at a rate of 45 degrees per second while held.
	if ( g_theInput->IsKeyDown( KEYCODE_UP ) )
	{
		Quaternion rotor				= Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), 45.f * deltaSeconds );
		m_sunRotation					= rotor * m_sunRotation;
		m_lightConstants.m_sunDirection = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	}

	// Pitch the sun direction at a rate of -45 degrees per second while held
	if ( g_theInput->IsKeyDown( KEYCODE_DOWN ) )
	{
		Quaternion rotor				= Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), -45.f * deltaSeconds );
		m_sunRotation					= rotor * m_sunRotation;
		m_lightConstants.m_sunDirection = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	}

	// Yaw the sun direction at a rate of 45 degrees per second while held
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHT ) )
	{
		Quaternion rotor				= Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), 45.f * deltaSeconds );
		m_sunRotation					= rotor * m_sunRotation;
		m_lightConstants.m_sunDirection = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	}

	// Yaw the sun direction at a rate of -45 degrees per second while held
	if ( g_theInput->IsKeyDown( KEYCODE_LEFT ) )
	{
		Quaternion rotor				= Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), -45.f * deltaSeconds );
		m_sunRotation					= rotor * m_sunRotation;
		m_lightConstants.m_sunDirection = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	}


	// Ambient intensity should always be the reciprocal of the sun intensity.
	// Increase the sun intensity by 0.1.
	if ( g_theInput->WasKeyJustPressed( KEYCODE_COMMA ) )
	{
		m_lightConstants.m_sunIntensity		+= 0.1f;
		m_lightConstants.m_ambientIntensity -= 0.1f;
	}

	// Decrease the sun intensity by 0.1.
	if ( g_theInput->WasKeyJustPressed( KEYCODE_PERIOD ) )
	{
		m_lightConstants.m_sunIntensity		-= 0.1f;
		m_lightConstants.m_ambientIntensity += 0.1f;
	}
}
