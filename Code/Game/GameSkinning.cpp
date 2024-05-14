#include "Game/GameSkinning.hpp"
#include "Game/Player.hpp"

#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Animation/Vertex_Skeletal.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------
GameSkinning::GameSkinning()
{
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::Startup()
{
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );
	m_GameClock = new Clock();

	CreateScene();
	m_player->m_position	= Vec3( 2.7f, -2.4f, 1.4f );
	m_player->m_orientation = EulerAngles( 144.f, 19.f, 0.f );
	// AddBasisAtOrigin();
	PrintTitleText( "Skinning" );
	LoadMeshData();

	LoadAnimPose();

	InitSpriteLitShader();
	InitLightConstants();
	CreateSunVerts();
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::Update()
{
	UpdateGameState();
	UpdatePlayer();
	UpdateLightConstants();
	PrintDebugScreenMessage();
	UpdateAnimatedPose();
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	// world
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );
	RenderGridLines();
	RenderMeshData();
	//RenderAnimPose();
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
void GameSkinning::Shutdown()
{
	delete m_player;
	// delete m_modelVertexBuffer;
	delete m_sunMesh;
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::PrintDebugScreenMessage()
{
	float fontSize = 15.f;
	AABB2 cameraBounds( m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight() );
	float textMinX = cameraBounds.m_mins.x;
	float textMinY = cameraBounds.m_maxs.y - fontSize;
	// Vec2		topLeftLinePosition( textMinX, textMinY );
	// Vec2		topLeftAlignment = Vec2( 0.f, 1.f );
	float duration = 0.f; // one frame

	// time and fps
	textMinX = cameraBounds.m_maxs.x;
	textMinY = cameraBounds.m_maxs.y - fontSize;
	Vec2		topRightLinePosition( textMinX, textMinY );
	Vec2		topRightAlignment = Vec2( 1.f, 1.f );
	float		totalSeconds	  = m_GameClock->GetTotalSeconds();

	float		deltaSeconds	  = m_GameClock->GetDeltaSeconds();
	float		fps				  = 1.f / deltaSeconds;
	float		ms				  = deltaSeconds * 1000.f;

	std::string timeValuesStr	  = Stringf( "Time: %.2f, FPS: %.1f, MS: %.1f", totalSeconds, fps, ms );
	DebugAddScreenText( timeValuesStr, topRightLinePosition, fontSize, topRightAlignment, duration );
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::InitSpriteLitShader()
{
	m_spriteLitShader = g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/SpriteLit", VertexType::Vertex_PCUTBN );
	// m_spriteLitShader = g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/SpriteLit", VertexType::Vertex_PCUTBN );
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::LoadMeshData()
{
	// std::vector<Vertex_PCUTBN> meshVerts;
	// std::vector<unsigned int>  jointIds;
	//FbxFileImporter::LoadPreRiggedAndPreSkinnedMeshBindPoseFromFile( "Data/Meshes/MayaCubeMesh3.fbx", m_meshVerts, m_vertexJointIdWeightMapping, m_animatedPose );
	FbxFileImporter::LoadPreRiggedAndPreSkinnedMeshBindPoseFromFile( "Data/Meshes/XBotTPose.fbx", m_meshVerts, m_vertexJointIdWeightMapping);
	// FbxFileImporter::LoadPreRiggedAndPreSkinnedMeshBindPoseFromFile( "Data/Animations/XBot/TPoseWithSkin.fbx", m_meshVerts, m_vertexJointIdWeightMapping );
	//  FbxFileImporter::LoadMeshFromFileIndexed( "Data/Meshes/MayaBasicCube.fbx", meshVerts, meshIndexes );
	//  FbxFileImporter::LoadMeshFromFile( "Data/Meshes/MayaBasicCylinder.fbx", meshVerts );
	//  FbxFileImporter::LoadMeshFromFile( "Data/Meshes/BasicCylinder.fbx", meshVerts );
	//   FbxFileImporter::LoadMeshFromFile( "Data/Meshes/XBotTPose.fbx", meshVerts );

	/*m_modelVertexBuffer				   = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCUTBN ), sizeof( Vertex_PCUTBN ) );
	m_modelVertexBuffer->m_vertexCount = ( unsigned int ) meshVerts.size();
	size_t vertexDataSize			   = sizeof( Vertex_PCUTBN ) * meshVerts.size();
	g_theRenderer->CopyCPUToGPU( meshVerts.data(), vertexDataSize, m_modelVertexBuffer );*/
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::RenderMeshData() const
{
	g_theRenderer->SetLightingConstatnts( m_lightConstants );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( m_spriteLitShader );
	Mat44 transform;
	g_theRenderer->SetModelConstants( transform );

	g_theRenderer->DrawVertexArrayPCUTBN( m_meshVerts );
	// g_theRenderer->DrawVertexBuffer( m_modelVertexBuffer, m_modelVertexBuffer->m_vertexCount );
	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	//g_theRenderer->SetModelConstants( transform, Rgba8::BLACK );
	//g_theRenderer->DrawVertexArrayPCUTBN( m_meshVerts );
	// g_theRenderer->DrawVertexBuffer( m_modelVertexBuffer, m_modelVertexBuffer->m_vertexCount );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// create the skinning matrix for each joint
	std::vector<Mat44> skinningMatricesByJointId;
	int numJoints = m_bindPose.GetNumberOfJoints();
	skinningMatricesByJointId.reserve( numJoints );
	for ( int jointIndex = 0; jointIndex < numJoints; jointIndex++ )
	{
		// get the matrix that transforms verts into skin space
		Mat44 const& inverseBindPoseTransformMatrix = m_bindPose.GetGlobalInverseBindPoseMatrixOfJoint( jointIndex );

		// calculate the matrix that transforms verts to new animated pos
		Transform secondMeshJointGlobalTransform = m_secondMeshBindPose.GetGlobalTransformOfJoint( jointIndex );
		secondMeshJointGlobalTransform.m_scale	 = Vec3( 1.f, 1.f, 1.f );
		Mat44 secondMeshBindPoseTransformMatrix	 = Mat44::CreateFromTransform( secondMeshJointGlobalTransform );

		// create skinning matrix = transform to skin space, then transform to new animated pos
		Mat44 skinningMatrix = secondMeshBindPoseTransformMatrix;
		skinningMatrix.Append( inverseBindPoseTransformMatrix );
		skinningMatricesByJointId.push_back( skinningMatrix );
	}

	// skinning
	std::vector<Vertex_PCUTBN> vertsTransformedToNewAnimatedPos = m_meshVerts;
	size_t					   numMeshVerts						= vertsTransformedToNewAnimatedPos.size();
	for ( size_t vertexNum = 0; vertexNum < numMeshVerts; vertexNum++ )
	{
		Vertex_PCUTBN&							  vertex				 = vertsTransformedToNewAnimatedPos[ vertexNum ];
		Vec3									  overallTransformedPos	 = Vec3::ZERO;
		std::vector<std::pair<int, float>> const& jointIdToWeightMapping = m_vertexJointIdWeightMapping[ vertexNum ];

		for ( auto iter = jointIdToWeightMapping.begin(); iter != jointIdToWeightMapping.end(); iter++ )
		{
			std::pair<int, float> const& mapping = *iter;
			int							 jointId = mapping.first;
			float						 weight	 = mapping.second;

			// transform vertex
			Mat44 const& jointSkinningMatrix = skinningMatricesByJointId[ jointId ];
			Vec3		 transformedPos		 = jointSkinningMatrix.TransformPosition3D( vertex.m_position );

			// add according to the weight influence, smooth skinning
			overallTransformedPos += ( weight * transformedPos );
		}

		vertex.m_position = overallTransformedPos;
		vertex.m_color	  = Rgba8::WHITE;
	}

	Mat44 transfrom2;
	transfrom2.AppendTranslation2D( Vec2( 0.f, -2.5f ) );
	g_theRenderer->SetModelConstants( transfrom2, Rgba8::GREEN );
	g_theRenderer->DrawVertexArrayPCUTBN( vertsTransformedToNewAnimatedPos );

	//g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	//g_theRenderer->SetModelConstants( transfrom2/*, Rgba8::DARK_GREY */);
	//g_theRenderer->DrawVertexArrayPCUTBN( vertsTransformedToNewAnimatedPos );
	//g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::LoadAnimPose()
{
	//FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/MayaCubeMesh3.fbx", m_bindPose );
	//FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/MayaCubeMesh3Deformed.fbx", m_secondMeshBindPose );

	// FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/XBot/TPoseWithSkin.fbx", m_bindPose );
	 FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/XBotTPose.fbx", m_bindPose );
	m_bindPose.CalculateGlobalInverseBindPoseMatrices();
	 FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/XBotTPose.fbx", m_secondMeshBindPose );
	// m_animatedPose				= m_bindPose;
	 m_secondMeshBindPose		= m_bindPose;
	 m_animatedClip				= new AnimClip();
	 m_animatedClip->m_isLooping = true;
	FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/XBot/Run.fbx", *m_animatedClip );
	////FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/XBot/CrouchIdle.fbx", *m_animatedClip );
	 //FbxFileImporter::LoadAnimClipFromFile( "Data/Animations/XBot/StandingIdleWithSkin.fbx", *m_animatedClip );
	////m_animatedClip->Sample( 0.f, m_animatedPose );
	//ModifyRunningSlideAnimationClip();
	 m_animatedClip->Sample( 0.f, m_secondMeshBindPose );
	//  FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/MayaBasicCube.fbx", m_animPose );
	//  FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/MayaBasicCylinder.fbx", m_animPose );
	//  FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/BasicCylinder.fbx", m_animPose );
	//  FbxFileImporter::LoadRestPoseFromFile( "Data/Meshes/XBotTPose.fbx", m_animPose );
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::UpdateAnimatedPose()
{
	float deltaSeconds = m_GameClock->GetDeltaSeconds();
	float deltaMilliseconds = deltaSeconds * 1000.f;
	m_animatedLocalTimeMs	+= deltaMilliseconds;
	m_animatedClip->Sample( m_animatedLocalTimeMs, m_secondMeshBindPose );
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::RenderAnimPose() const
{
	// 1. render bind pose
	std::vector<Vertex_PCU> skeletalVerts;
	for ( int jointIndex = 0; jointIndex < m_bindPose.GetNumberOfJoints(); jointIndex++ )
	{
		int parentIndex = m_bindPose.GetParentOfJoint( jointIndex );
		if ( parentIndex < 0 )
		{
			continue;
		}

		Transform const& jointGlobalTransform  = m_bindPose.GetGlobalTransformOfJoint( jointIndex );
		Transform const& parentGlobalTransform = m_bindPose.GetGlobalTransformOfJoint( parentIndex );
		AddVertsForCone3D( skeletalVerts, parentGlobalTransform.m_position, jointGlobalTransform.m_position, 0.005f, Rgba8::SOFT_RED );
	}
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( skeletalVerts );

	// 2. render animated pose
	skeletalVerts.clear();
	for ( int jointIndex = 0; jointIndex < m_secondMeshBindPose.GetNumberOfJoints(); jointIndex++ )
	{
		int parentIndex = m_secondMeshBindPose.GetParentOfJoint( jointIndex );
		if ( parentIndex < 0 )
		{
			continue;
		}

		Transform const& jointGlobalTransform  = m_secondMeshBindPose.GetGlobalTransformOfJoint( jointIndex );
		Transform const& parentGlobalTransform = m_secondMeshBindPose.GetGlobalTransformOfJoint( parentIndex );
		AddVertsForCone3D( skeletalVerts, parentGlobalTransform.m_position, jointGlobalTransform.m_position, 0.005f, Rgba8::SOFT_GREEN );
	}
	Mat44 transfrom2;
	transfrom2.AppendTranslation2D( Vec2( 0.f, -2.5f ) );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants( transfrom2 );
	g_theRenderer->DrawVertexArray( skeletalVerts );
}


//----------------------------------------------------------------------------------------------------------
void GameSkinning::InitLightConstants()
{
	m_lightControlClock = new Clock();
	// m_sunRotation						 = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), 90.f );
	m_sunRotation = Quaternion( 0.289f, 0.914f, -0.277f, -0.058f );
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
void GameSkinning::CreateSunVerts()
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
void GameSkinning::UpdateLightConstants()
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
void GameSkinning::PrintLightConstants() const
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
void GameSkinning::RenderSunDirection() const
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


//----------------------------------------------------------------------------------------------------------
void GameSkinning::ModifyRunningSlideAnimationClip()
{
	Vec3AnimCurve& rootMotionCurve = m_animatedClip->GetRootJointTranslationCurveByRefernce();

	float		   xMin, xMax;
	rootMotionCurve.GetMaxAndMinXValuesFromCurve( xMin, xMax );
	for (unsigned int index = 0; index < rootMotionCurve.GetSize(); index++)
	{
		float& x = rootMotionCurve.m_keyframes[ index ].m_value.x;
		x		 = RangeMap( x, xMin, xMax, 0.f, 1.f );

		float t	 = rootMotionCurve.m_keyframes[ index ].m_timeMilliSeconds;
		float parametricZeroToOne = t / rootMotionCurve.GetEndTimeMilliSeconds();
		float lastX;
		if (parametricZeroToOne < 0.5f)
		{
			x *= 15.f;
			lastX  = x;
		}
		else
		{
			x *= 0.3f;
			x = ( x + lastX );
		}
	}


}
