#include "Game/GameLoadMesh.hpp"
#include "Game/Player.hpp"

#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/StringUtils.hpp"


//----------------------------------------------------------------------------------------------------------
GameLoadMesh::GameLoadMesh()
{
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::Startup()
{
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();
	PrintTitleText( "Load Mesh" );
	LoadMeshData();

	InitSpriteLitShader();
	InitLightConstants();
	CreateSunVerts();
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::Update()
{
	UpdateGameState();
	UpdatePlayer();
	UpdateLightConstants();
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	// world
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );
	RenderGridLines();
	RenderMeshData();
	RenderSunDirection();
	DebugRenderWorld( *m_player->m_worldCamera );
	g_theRenderer->EndCamera( *m_player->m_worldCamera );

	// screen
	g_theRenderer->BeginCamera( m_screenCamera );
	PrintLightConstants();
	DebugRenderScreen( m_screenCamera );
	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::Shutdown()
{
	delete m_player;
	delete m_modelVertexBuffer;
	delete m_sunMesh;
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::InitSpriteLitShader()
{
	m_spriteLitShader = g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/SpriteLit", VertexType::Vertex_PCUTBN );
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::LoadMeshData()
{
	std::vector<Vertex_PCUTBN> meshVerts;
	// FbxFileImporter::LoadMeshFromFile( "Data/Meshes/blenderCube.fbx", meshVerts );
	//FbxFileImporter::LoadMeshFromFile( "Data/Meshes/BasicCylinder.fbx", meshVerts );
	FbxFileImporter::LoadMeshFromFile( "Data/Meshes/Car_Sedan.FBX", meshVerts );
	//FbxFileImporter::LoadMeshFromFile( "Data/Meshes/XBotTPose.FBX", meshVerts );

	m_modelVertexBuffer				   = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCUTBN ), sizeof( Vertex_PCUTBN ) );
	m_modelVertexBuffer->m_vertexCount = ( unsigned int ) meshVerts.size();
	size_t vertexDataSize			   = sizeof( Vertex_PCUTBN ) * meshVerts.size();
	g_theRenderer->CopyCPUToGPU( meshVerts.data(), vertexDataSize, m_modelVertexBuffer );
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::RenderMeshData() const
{
	g_theRenderer->SetLightingConstatnts( m_lightConstants );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( m_spriteLitShader );
	Mat44 transform;
	g_theRenderer->SetModelConstants( transform );

	g_theRenderer->DrawVertexBuffer( m_modelVertexBuffer, m_modelVertexBuffer->m_vertexCount );
	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	g_theRenderer->SetModelConstants( transform, Rgba8::RED );
	//g_theRenderer->DrawVertexBuffer( m_modelVertexBuffer, m_modelVertexBuffer->m_vertexCount );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::InitLightConstants()
{
	m_lightControlClock					 = new Clock();
	//m_sunRotation						 = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), 90.f );
	m_sunRotation						 = Quaternion(0.289f, 0.914f, -0.277f, -0.058f);
	m_sunRotation.Normalize();

	m_lightConstants.m_sunDirection		 = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	m_lightConstants.m_sunIntensity		 = 1.f;
	m_lightConstants.m_ambientIntensity	 = 0.f;
	m_lightConstants.m_worldEyePosition	 = g_theGame->m_player->m_position;
	m_lightConstants.m_normalMode		 = 0;
	m_lightConstants.m_specularMode		 = 0;
	m_lightConstants.m_specularIntensity = 1.f;
	m_lightConstants.m_specularPower	 = 32.f;
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::CreateSunVerts()
{
	std::vector<Vertex_PCU> verts;

	Vec3					start  = Vec3( 0.f, 0.f, 0.f );
	Vec3					end	   = Vec3( 1.f, 0.f, 0.f );
	float					radius = 0.07f;
	AddVertsForArrow3D( verts, start, end, radius, Rgba8::WHITE );

	m_sunMesh				 = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	m_sunMesh->m_vertexCount = ( unsigned int ) verts.size();
	size_t vertexDataSize	 = sizeof( Vertex_PCU ) * verts.size();
	g_theRenderer->CopyCPUToGPU( verts.data(), vertexDataSize, m_sunMesh );
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::UpdateLightConstants()
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

	//// Increase the specular intensity by 0.1.
	// if ( g_theInput->WasKeyJustPressed( KEYCODE_SEMICOLON ) )
	//{
	//	m_lightConstants.m_specularIntensity += 0.1f;
	// }

	//// Decrease the specular intensity by 0.1.
	// if ( g_theInput->WasKeyJustPressed( KEYCODE_SINGLE_QUOTE ) )
	//{
	//	m_lightConstants.m_specularIntensity -= 0.1f;
	// }

	//// Increase the specular power by 0.1.
	// if ( g_theInput->WasKeyJustPressed( KEYCODE_SQUARE_BRACKET_LEFT ) )
	//{
	//	m_lightConstants.m_specularPower += 0.1f;
	// }

	//// Decrease the specular power by 0.1.
	// if ( g_theInput->WasKeyJustPressed( KEYCODE_SQUARE_BRACKET_RIGHT ) )
	//{
	//	m_lightConstants.m_specularPower -= 0.1f;
	// }

	// Toggle debug rendering of tangent space basis vectors off and on.
	if ( g_theInput->WasKeyJustPressed( 'B' ) )
	{
		m_isDebugDrawTangent = !m_isDebugDrawTangent;
	}

	//// Toggle between rendering with normal maps or vertex normals.
	// if ( g_theInput->WasKeyJustPressed( 'N' ) )
	//{
	//	m_lightConstants.m_normalMode == 0 ? m_lightConstants.m_normalMode = 1 : m_lightConstants.m_normalMode = 0;
	// }

	//// Toggle between rendering with specular maps or constant specular.
	// if ( g_theInput->WasKeyJustPressed( 'M' ) )
	//{
	//	m_lightConstants.m_specularMode == 0 ? m_lightConstants.m_specularMode = 1 : m_lightConstants.m_specularMode = 0;
	// }
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::PrintLightConstants() const
{
	// time and fps
	float duration = 0.f; // one frame
	float fontSize = 15.f;
	AABB2 cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float textMinX = cameraBounds.m_maxs.x;
	float textMinY = cameraBounds.m_maxs.y - fontSize;
	Vec2  linePosition( textMinX, textMinY );
	Vec2  topRightAlignment		   = Vec2( 1.f, 1.f );

	linePosition.y				  -= 2.f * fontSize;
	std::string sunOrientationStr  = Stringf( "Sun Direction: (%.1f, %.1f, %.1f)", m_lightConstants.m_sunDirection.x, m_lightConstants.m_sunDirection.y, m_lightConstants.m_sunDirection.z );
	DebugAddScreenText( sunOrientationStr, linePosition, fontSize, topRightAlignment, duration );

	// sun intensity
	linePosition.y				-= fontSize;
	std::string sunIntensityStr	 = Stringf( "Sun Intensity: %.1f", m_lightConstants.m_sunIntensity );
	DebugAddScreenText( sunIntensityStr, linePosition, fontSize, topRightAlignment, duration );
}


//----------------------------------------------------------------------------------------------------------
void GameLoadMesh::RenderSunDirection() const
{
	Vec3  iForward = m_sunRotation * Vec3( 1.f, 0.f, 0.f );
	Vec3  jLeft	   = m_sunRotation * Vec3( 0.f, 1.f, 0.f );
	Vec3  kUp	   = m_sunRotation * Vec3( 0.f, 0.f, 1.f );
	Vec3  origin   = Vec3( 0.f, 0.f, 10.f );

	Mat44 transform;
	transform.SetIJKT3D( iForward, jLeft, kUp, origin );

	g_theRenderer->SetModelConstants( transform, Rgba8::YELLOW );
	g_theRenderer->BindTextures( std::vector<Texture*>{ nullptr, nullptr, nullptr } );
	g_theRenderer->BindShader( nullptr );

	g_theRenderer->DrawVertexBuffer( m_sunMesh, m_sunMesh->m_vertexCount );
}
