#include "Game/GameFixCameraIdleTurn.hpp"
#include "Game/GameFinalShowcase.hpp"
#include "Game/GameLoadFbxOnThread.hpp"
#include "Game/GameSkinning.hpp"
#include "Game/GameLoadMesh.hpp"
#include "Game/GameLedgeGrabTest.hpp"
#include "Game/GameVaultingTest.hpp"
#include "Game/GameQuaternionUnitTest.hpp"
#include "Game/GameBasicMovement.hpp"
#include "Game/GameAdditiveBlending.hpp"
#include "Game/GameCrossfadeAnimationWithController.hpp"
#include "Game/GameCrossfadeAnimationSmoothLinear.hpp"
#include "Game/GameCrossfadeAnimationLastFrame.hpp"
#include "Game/GamePoseBlending.hpp"
#include "Game/GameDefaultPose.hpp"
#include "Game/GameBasicAnimationPlayback.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"
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




//----------------------------------------------------------------------------------------------------------
Game* Game::CreateNewGameOfType( GameMode type )
{
	switch ( type )
	{
		case GAMEMODE_DEFAULT_POSE:												return new GameDefaultPose();
		case GAMEMODE_BASIC_ANIMATION_PLAYBACK:									return new GameBasicAnimationPlayback();
		case GAMEMODE_POSE_BLENDING:											return new GamePoseBlending();
		case GAMEMODE_CROSSFADE_ANIMATION_BLENDING_ONE_FRAME:					return new GameCrossfadeAnimationLastFrame();
		case GAMEMODE_CROSSFADE_ANIMATION_BLENDING_SMOOTH_LINEAR_MANUAL:		return new GameCrossfadeAnimationSmoothLinear();
		case GAMEMODE_CROSSFADE_ANIMATION_BLENDING_SMOOTH_LINEAR_CONTROLLER:	return new GameCrossfadeAnimationWithController();
		case GAMEMODE_ADDITIVE_BLENDING:										return new GameAdditiveBlending();
		case GAMEMODE_BASIC_3D_MOVEMENT:										return new GameBasicMovement();
		case GAMEMODE_VAULTING_TEST:											return new GameVaultingTest();
		case GAMEMODE_LEDGE_GRAB_TEST:											return new GameLedgeGrabTest();
		case GAMEMODE_LOADMESH:													return new GameLoadMesh();
		case GAMEMODE_SKINNING:													return new GameSkinning();
		case GAMEMODE_FINAL_SHOWCASE:											return new GameFinalShowcase();
		case GAMEMODE_LOAD_FBX_THREAD:											return new GameLoadFbxOnThread();
		case GAMEMODE_FIX_CAMERA_IDLE_TURN:										return new GameFixCameraIdleTurn();
		default:
		{
			ERROR_AND_DIE( Stringf( "Error: GameMode #%i not set", type ) );
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void Game::CreateScene()
{
	// 1. add a player to the scene
	m_player = new Player();
	m_player->Startup();
	m_player->m_position	= Vec3( 19.f, -9.f, 11.f );
	m_player->m_orientation = EulerAngles( 144.f, 19.f, 0.f );

	bool cursorHidden	= true;
	bool cursorRelative = true;
	g_theInput->SetCursorMode( cursorHidden, cursorRelative );

	CreateGridVerts();
}


//----------------------------------------------------------------------------------------------------------
void Game::AddBasisAtOrigin()
{
	// basis arrows
	Vec3 origin	  = Vec3( 0.f, 0.f, 0.f );
	Vec3 xForward = Vec3( 1.f, 0.f, 0.f );
	Vec3 yLeft	  = Vec3( 0.f, 1.f, 0.f );
	Vec3 zUp	  = Vec3( 0.f, 0.f, 1.f );

	float			radius	 = 0.1f;
	float			duration = -1.f;
	DebugRenderMode mode	 = DebugRenderMode::USE_DEPTH;
	DebugAddWorldArrow( origin, xForward, radius, duration, Rgba8::RED, Rgba8::RED, mode );
	DebugAddWorldArrow( origin, yLeft, radius, duration, Rgba8::GREEN, Rgba8::GREEN, mode );
	DebugAddWorldArrow( origin, zUp, radius, duration, Rgba8::BLUE, Rgba8::BLUE, mode );

	// text
	Mat44 alongXAxisTransform( Vec3( 0.f, -1.f, 0.f ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), Vec3( 0.2f, 0.f, 0.2f ) );
	float textHeight   = 0.2f;
	Vec2  alignment	   = Vec2( 0.f, 0.f );
	float textDuration = -1.f;
	DebugAddWorldText( "x - forward", alongXAxisTransform, textHeight, alignment, textDuration, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH );

	Mat44 alongYAxisTransform( Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, -1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), Vec3( 0.f, 0.2f, 0.2f ) );
	alignment = Vec2( 1.f, 0.f );
	DebugAddWorldText( "y - left", alongYAxisTransform, textHeight, alignment, textDuration, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH );

	Mat44 alongZAxisTransform( Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, -0.2f, 0.2f ) );
	alignment = Vec2( 0.f, 1.f );
	DebugAddWorldText( "z - up", alongZAxisTransform, textHeight, alignment, textDuration, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH );
}


//----------------------------------------------------------------------------------------------------------
void Game::PrintDebugScreenMessage()
{
	//// player position screen text
	//Vec3 playerPosition = m_player->m_position;

	//std::string playerPositionStr = Stringf( "Camera Position: %.2f, %.2f, %.2f", playerPosition.x, playerPosition.y, playerPosition.z );
	//float		fontSize		  = 15.f;
	//AABB2		cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	//float		textMinX = cameraBounds.m_mins.x;
	//float		textMinY = cameraBounds.m_maxs.y - fontSize;
	//Vec2		topLeftLinePosition( textMinX, textMinY );
	//Vec2		topLeftAlignment = Vec2( 0.f, 1.f );
	//float		duration		 = 0.f; // one frame
	//DebugAddScreenText( playerPositionStr, topLeftLinePosition, fontSize, topLeftAlignment, duration );

	//std::string playerOrientationStr = Stringf( "Camera orientation: %.2f, %.2f, %.2f", 
	//	m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees );
	//DebugAddScreenText( playerOrientationStr, topLeftLinePosition - Vec2(0.f, fontSize), fontSize, topLeftAlignment, duration );

	//// time and fps
	//textMinX = cameraBounds.m_maxs.x;
	//textMinY = cameraBounds.m_maxs.y - fontSize;
	//Vec2  topRightLinePosition( textMinX, textMinY );
	//Vec2  topRightAlignment = Vec2( 1.f, 1.f );
	//float totalSeconds		= m_GameClock->GetTotalSeconds();

	//float deltaSeconds = m_GameClock->GetDeltaSeconds();
	//float fps		   = 1.f / deltaSeconds;
	//float ms		   = deltaSeconds * 1000.f;

	//std::string timeValuesStr = Stringf( "Time: %.2f, FPS: %.1f, MS: %.1f", totalSeconds, fps, ms );
	//DebugAddScreenText( timeValuesStr, topRightLinePosition, fontSize, topRightAlignment, duration );
}


//----------------------------------------------------------------------------------------------------------
void Game::PrintTitleText( std::string text )
{
	Strings splitStrings = SplitStringOnDelimiter( text, '\n' );

	float fontSize = 18.f;
	AABB2 cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float textMinX = ( cameraBounds.m_maxs.x - cameraBounds.m_mins.x ) * 0.5f;
	float textMinY = cameraBounds.m_maxs.y - fontSize;
	Vec2  textMins( textMinX, textMinY );
	Vec2  alignment = Vec2( 0.5f, 1.f );
	float duration	= -1.f;
	DebugAddScreenText( splitStrings[ 0 ], textMins, fontSize, alignment, duration, Rgba8::LIGHT_CORAL );


	if ( splitStrings.size() > 1 )
	{
		float textMinsY2 = textMinY - fontSize;
			Vec2  textMins2( textMinX, textMinsY2 );
		DebugAddScreenText( splitStrings[ 1 ], textMins2, fontSize, alignment, duration, Rgba8::LIGHT_CORAL );
	}
}



//----------------------------------------------------------------------------------------------------------
void Game::BeginFrame()
{
}


//----------------------------------------------------------------------------------------------------------
void Game::UpdateGameState()
{
	XboxController const& controller = g_theInput->GetController( 0 );

	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		/*m_singleStepMode = true;

		m_GameClock->Unpause();*/

		m_GameClock->StepSingleFrame();
	}

	// Pause
	if ( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		if ( !m_GameClock->IsPaused() )
		{
			m_GameClock->Pause();
		}
		else
		{
			m_GameClock->Unpause();
			m_singleStepMode = false;
		}
	}

	// Start Slow Motion
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		//m_GameClock->SetTimeScale( 0.1f );
		m_GameClock->SetTimeScale( 0.2f );
	}
	else
	{
		m_GameClock->SetTimeScale( 1.f );
	}

	// Debug Mode (f1)
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) ||
		 controller.WasButtonJustPressed( XBOX_BUTTON_BACK ) )
	{
		m_showDebugView = !m_showDebugView;
	}

	
}


//----------------------------------------------------------------------------------------------------------
void Game::UpdatePlayer()
{
	//float deltaSeconds = m_GameClock->GetDeltaSeconds();

	m_player->Update( );

	/*DebuggerPrintf(Stringf("%.2f, %.2f, %.2f\n",
		m_player->m_orientation.m_yawDegrees,
		m_player->m_orientation.m_pitchDegrees,
		m_player->m_orientation.m_rollDegrees).c_str());*/
}


//----------------------------------------------------------------------------------------------------------
void Game::RenderGridLines() const
{
	g_theRenderer->BindTexture( nullptr );
	
	g_theRenderer->DrawVertexBuffer( m_gridGpuVerts, m_numGridVerts );
}


//----------------------------------------------------------------------------------------------------------
void Game::EndFrame()
{
	/*if ( m_singleStepMode )
	{
		m_GameClock->Pause();
	}*/
}


//----------------------------------------------------------------------------------------------------------
bool Game::IsDubugViewOn()
{
	return m_showDebugView;
}


//----------------------------------------------------------------------------------------------------------
void Game::CreateGridVerts()
{
	std::vector<Vertex_PCU> verts;

	// axis lines
	/*AABB3 xOriginPipe(Vec3(0.f, 0.f, 0.f), Vec3(1.f, 0.1f, 0.1f));
	AddVertsForAABB3D(verts, xOriginPipe, Rgba8(255, 0, 0, 175));

	AABB3 yOriginPipe(Vec3(0.f, 0.f, 0.f), Vec3(0.1f, 1.f, 0.1f));
	AddVertsForAABB3D(verts, yOriginPipe, Rgba8(0, 255, 0, 175));

	AABB3 zOriginPipe(Vec3(0.f, 0.f, 0.f), Vec3(0.1f, 0.1f, 1.0f));
	AddVertsForAABB3D(verts, zOriginPipe, Rgba8(0, 0, 255, 175));*/

	// thin x axis grid lines
	// lines parallel to x-axis going in the +y direction
	float yPos		 = 0.f;
	float halfLength = 50.f;
	// Rgba8 xPipeColor( 200, 0, 0, 175 );
	Rgba8 xPipeColor = Rgba8( 114, 114, 114 );
	/*for ( int numXLines = 0; numXLines < 50; numXLines++ )
	{
		AABB3 pipe( Vec3( -halfLength, yPos, 0.f ), Vec3( halfLength, yPos + 0.02f, 0.02f ) );
		AddVertsForAABB3D( verts, pipe, xPipeColor );
		yPos += 3.f;
	}*/

	// lines parallel to x-axis going in the -y direction
	/*yPos = -1.f;
	for ( int numXLines = 0; numXLines < 50; numXLines++ )
	{
		AABB3 pipe( Vec3( -halfLength, yPos, 0.f ), Vec3( halfLength, yPos + 0.02f, 0.02f ) );
		AddVertsForAABB3D( verts, pipe, xPipeColor );
		yPos -= 3.f;
	}*/

	// thin y grid lines
	// Rgba8 yPipeColor( 0, 200, 0, 175 );
	Rgba8 yPipeColor = Rgba8( 114, 114, 114 );
	/*for ( float xPos = -50.f; xPos <= 50.f; xPos += 1.f )
	{
		AABB3 yPipe( Vec3( xPos, -halfLength, 0.f ), Vec3( xPos + 0.02f, halfLength, 0.02f ) );
		AddVertsForAABB3D( verts, yPipe, yPipeColor );
	}*/

	// thick y Grids lines
	// Rgba8 yPipeColor(0, 200, 0, 175);
	for ( float xPos = -50.f; xPos <= 50.f; xPos += 5.f )
	{
		AABB3 yPipe( Vec3( xPos, -halfLength, 0.f ), Vec3( xPos + 0.08f, halfLength, 0.08f ) );
		AddVertsForAABB3D( verts, yPipe, yPipeColor );
	}

	// thick x Grid lines
	for ( yPos = -50.f; yPos <= 50.f; yPos += 5.f )
	{
		AABB3 pipe( Vec3( -halfLength, yPos, 0.f ), Vec3( halfLength, yPos + 0.08f, 0.08f ) );
		AddVertsForAABB3D( verts, pipe, xPipeColor );
	}


	// lines passing through origin
	AABB3 xOriginPipeLong( Vec3( -halfLength, 0.f, 0.f ), Vec3( halfLength, 0.1f, 0.1f ) );
	AddVertsForAABB3D( verts, xOriginPipeLong, xPipeColor );

	AABB3 yOriginPipeLong( Vec3( 0.f, -halfLength, 0.f ), Vec3( 0.11f, halfLength, 0.11f ) );
	AddVertsForAABB3D( verts, yOriginPipeLong, yPipeColor );

	AABB2 bigPlane = AABB2( -50.f, -50.f, 50.f, 50.f );
	AddVertsForAABB2( verts, bigPlane, Rgba8( 50, 50, 50 ) );

	/*AABB3 zOriginPipeLong(Vec3(0.f, 0.f, 0.f), Vec3(0.42f, 0.42f, halfLength));
	AddVertsForAABB3D(verts, zOriginPipeLong, Rgba8(0, 0, 200, 175));*/

	m_gridGpuVerts = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	g_theRenderer->CopyCPUToGPU( verts.data(), sizeof( Vertex_PCU ) * verts.size(), m_gridGpuVerts );
	m_numGridVerts = (int) verts.size();
}