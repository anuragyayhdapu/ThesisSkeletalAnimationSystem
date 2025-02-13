#include "Game/Map.hpp"
#include "Game/AnimationState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/Player.hpp"
#include "Game/GameLedgeGrabTest.hpp"
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


void GameLedgeGrabTest::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	 AddBasisAtOrigin();

	PrintTitleText( "Ledge Grab Test\n F6: Lock/Free Camera, F5: Camera Freeze, F9: Toggle Debug Drawing" );
	CreateObstacle();

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
void GameLedgeGrabTest::Shutdown()
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
void GameLedgeGrabTest::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();

	g_theThirdPersonController->Update();
	g_theCharacter->Update();
}


//----------------------------------------------------------------------------------------------------------
void GameLedgeGrabTest::Render() const
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
void GameLedgeGrabTest::PrintDebugScreenMessage()
{
	//// camera position screen text
	// Vec3 cameraPos = m_controller.GetCameraPosition();

	// std::string cameraPosStr = Stringf( "Camera Position: %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z );
	float fontSize = 15.f;
	AABB2 cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float textMinX = cameraBounds.m_mins.x;
	float textMinY = cameraBounds.m_maxs.y - fontSize;
	// Vec2		topLeftLinePosition( textMinX, textMinY );
	// Vec2		topLeftAlignment = Vec2( 0.f, 1.f );
	float duration = 0.f; // one frame
	// DebugAddScreenText( cameraPosStr, topLeftLinePosition, fontSize, topLeftAlignment, duration );

	// EulerAngles cameraOrientation	 = m_controller.GetCameraOrientation().GetAsEulerAngles();
	// std::string cameraOrientationStr = Stringf( "Camera orientation: %.2f, %.2f, %.2f",
	//	cameraOrientation.m_yawDegrees, cameraOrientation.m_pitchDegrees, cameraOrientation.m_rollDegrees );
	// DebugAddScreenText( cameraOrientationStr, topLeftLinePosition - Vec2( 0.f, fontSize ), fontSize, topLeftAlignment, duration );


	//// character position screen text
	// Vec3 characterPos = m_controller.GetCharacterPosition();

	// std::string characterPosStr = Stringf( "Character Position: %.2f, %.2f, %.2f", characterPos.x, characterPos.y, characterPos.z );
	// DebugAddScreenText( characterPosStr, topLeftLinePosition - Vec2( 0.f, fontSize * 3.f ), fontSize, topLeftAlignment, duration );

	// EulerAngles characterOrientation	= m_controller.GetCharacterOrientation().GetAsEulerAngles();
	// std::string characterOrientationStr = Stringf( "Character orientation: %.2f, %.2f, %.2f",
	//	characterOrientation.m_yawDegrees, characterOrientation.m_pitchDegrees, characterOrientation.m_rollDegrees );
	// DebugAddScreenText( characterOrientationStr, topLeftLinePosition - Vec2( 0.f, fontSize * 4.f ), fontSize, topLeftAlignment, duration );


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
void GameLedgeGrabTest::CreateObstacle()
{
	AABB3 vaultObstacle = AABB3( Vec3( 0.f, 0.f, 0.f ), Vec3( 20.f, 1.5f, 1.4f ) );

	AABB3 ledgeGrabObstacle = AABB3( Vec3( 20.f, 1.5f, 0.f ), Vec3( 30.5f, 20.5f, 3.25f) );

	m_map = new Map( std::vector<AABB3>{ vaultObstacle, ledgeGrabObstacle } );
}


//----------------------------------------------------------------------------------------------------------
void GameLedgeGrabTest::RenderObstacle() const
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
