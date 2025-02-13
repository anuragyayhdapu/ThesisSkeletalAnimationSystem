#include "Game/AnimationState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/Player.hpp"
#include "Game/GameBasicMovement.hpp"
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


void GameBasicMovement::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	AnimationState::LoadAnimationStateFromXML( "Data/Animations/AnimConfig.xml" );

	CreateScene();
	//AddBasisAtOrigin();

	PrintTitleText( "Basic Movement\n F6: Lock/Free Camera, F5: Camera Freeze" );

	g_theAnimationController = new AnimationController();
	g_theAnimationController->Startup();

	g_theCharacter = new Character();
	g_theCharacter->Startup();
	g_theCharacter->CreateVerts();



	//m_controller			  = new ThirdPersonController();
	g_theThirdPersonController = new ThirdPersonController();

	
}


//----------------------------------------------------------------------------------------------------------
void GameBasicMovement::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameBasicMovement::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();

	g_theThirdPersonController->Update();
	g_theCharacter->Update();

	
}


//----------------------------------------------------------------------------------------------------------
void GameBasicMovement::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *( g_theThirdPersonController->m_worldCamera ) );

	RenderGridLines();

	DebugRenderWorld( *( g_theThirdPersonController->m_worldCamera ) );

	g_theThirdPersonController->Render();
	g_theCharacter->Render();
	

	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	g_theAnimationController->DebugRender();

	DebugRenderScreen( m_screenCamera );


	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void GameBasicMovement::PrintDebugScreenMessage()
{
	//// camera position screen text
	//Vec3 cameraPos = m_controller.GetCameraPosition();

	//std::string cameraPosStr = Stringf( "Camera Position: %.2f, %.2f, %.2f", cameraPos.x, cameraPos.y, cameraPos.z );
	float		fontSize	 = 15.f;
	AABB2		cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float		textMinX = cameraBounds.m_mins.x;
	float		textMinY = cameraBounds.m_maxs.y - fontSize;
	//Vec2		topLeftLinePosition( textMinX, textMinY );
	//Vec2		topLeftAlignment = Vec2( 0.f, 1.f );
	float		duration		 = 0.f; // one frame
	//DebugAddScreenText( cameraPosStr, topLeftLinePosition, fontSize, topLeftAlignment, duration );

	//EulerAngles cameraOrientation	 = m_controller.GetCameraOrientation().GetAsEulerAngles();
	//std::string cameraOrientationStr = Stringf( "Camera orientation: %.2f, %.2f, %.2f",
	//	cameraOrientation.m_yawDegrees, cameraOrientation.m_pitchDegrees, cameraOrientation.m_rollDegrees );
	//DebugAddScreenText( cameraOrientationStr, topLeftLinePosition - Vec2( 0.f, fontSize ), fontSize, topLeftAlignment, duration );


	//// character position screen text
	//Vec3 characterPos = m_controller.GetCharacterPosition();

	//std::string characterPosStr = Stringf( "Character Position: %.2f, %.2f, %.2f", characterPos.x, characterPos.y, characterPos.z );
	//DebugAddScreenText( characterPosStr, topLeftLinePosition - Vec2( 0.f, fontSize * 3.f ), fontSize, topLeftAlignment, duration );

	//EulerAngles characterOrientation	= m_controller.GetCharacterOrientation().GetAsEulerAngles();
	//std::string characterOrientationStr = Stringf( "Character orientation: %.2f, %.2f, %.2f",
	//	characterOrientation.m_yawDegrees, characterOrientation.m_pitchDegrees, characterOrientation.m_rollDegrees );
	//DebugAddScreenText( characterOrientationStr, topLeftLinePosition - Vec2( 0.f, fontSize * 4.f ), fontSize, topLeftAlignment, duration );


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
