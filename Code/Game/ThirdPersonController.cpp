#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/MovementState.hpp"
#include "Game/Character.hpp"
#include "Game/ThirdPersonController.hpp"
#include "Game/GameCommon.hpp"


#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"


constexpr float MOVEMENT_SPEED			 = 2.f;
constexpr float FAST_MOVEMENT_MULTIPLIER = 5.f;
constexpr float MIN_PITCH_DEGREES		 = -85.f;
constexpr float MAX_PITCH_DEGREES		 = 85.f;
constexpr float MIN_ROLL_DEGREES		 = -45.f;
constexpr float MAX_ROLL_DEGREES		 = 45.f;


ThirdPersonController::ThirdPersonController()
{
	Startup();

	Clock* gameClock = g_theGame->m_GameClock;
	m_characterClock = new Clock( *gameClock );
	m_cameraClock	 = new Clock();
}

ThirdPersonController::~ThirdPersonController()
{
	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_characterClock;
	m_characterClock = nullptr;

	delete m_cameraClock;
	m_cameraClock = nullptr;

	delete m_debugBasisMesh;
}


void ThirdPersonController::Startup()
{
	m_worldCamera				  = new Camera();
	m_worldCamera->m_position	  = Vec3( 8.94f, -16.81f, 3.04f );
	Quaternion defaultOrientation = Quaternion( -0.114f, 0.124f, 0.666f, 0.726f );
	defaultOrientation.Normalize();
	m_worldCamera->SetOrientation( defaultOrientation );

	float aspect	 = g_theRenderer->GetConfig().m_window->GetConfig().m_clientAspect;
	float fovDegrees = 60.f;
	float zNearPlane = 0.1f;
	float zFarPlane	 = 10000.f;
	m_worldCamera->SetPerspectiveView( aspect, fovDegrees, zNearPlane, zFarPlane );

	Vec3 d3dIBasis( 0.f, 0.f, 1.f );
	Vec3 d3dJBasis( -1.f, 0.f, 0.f );
	Vec3 d3dKBasis( 0.f, 1.f, 0.f );
	m_worldCamera->SetRenderBasis( d3dIBasis, d3dJBasis, d3dKBasis );

	CreateDebugBasisMesh();
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::Update()
{
	UpdateControllerMode();
	UpdateMovement();
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateControllerMode()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F6 ) )
	{
		if ( m_controllerMode == ControllerMode::Follow )
		{
			m_controllerMode = ControllerMode::FreeRoam;
		}
		else // free roam
		{
			m_controllerMode = ControllerMode::Follow;
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateMovement()
{
	bool isCurrentWindowFocused = g_theWindow->DoesCurrentWindowHaveFocus();
	bool isDevConsoleClosed		= !g_theDevConsole->IsOpen();

	if ( isCurrentWindowFocused && isDevConsoleClosed )
	{
		if ( m_controllerMode == ControllerMode::Follow )
		{
			// only update the horizontal movement and yaw
			float deltaseconds = m_characterClock->GetDeltaSeconds();
			UpdateMovementFollow( deltaseconds );
		}
		else if ( m_controllerMode == ControllerMode::FreeRoam )
		{
			float deltaseconds = m_cameraClock->GetDeltaSeconds();
			UpdateMovementFreeRoam( deltaseconds );
		}
	}
}


void ThirdPersonController::UpdateMovementFreeRoam( float deltaseconds )
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		m_lockCamera = !m_lockCamera;
	}

	if ( !m_lockCamera )
	{
		UpdateMovementFreeflow( deltaseconds );
	}
	UpdateOrientationFreeflow();
}

void ThirdPersonController::UpdateMovementFreeflow( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------
	// vertical movement
	Vec3 moveIntentions = Vec3( 0.f, 0.f, 0.f );
	Vec3 worldUp		= Vec3( 0.f, 0.f, 1.f );

	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		moveIntentions -= worldUp;
	}
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		moveIntentions += worldUp;
	}

	//----------------------------------------------------------------------------------------------------------
	// horizontal movement
	Mat44 cameraOrientation = GetCameraOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3  iForward			= cameraOrientation.GetIBasis3D();
	iForward.z				= 0.f;
	iForward.Normalize();

	Vec3 jLeft = cameraOrientation.GetJBasis3D();
	jLeft.z	   = 0.f;
	jLeft.Normalize();

	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		moveIntentions += iForward;
	}
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		moveIntentions -= iForward;
	}
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		moveIntentions += jLeft;
	}
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		moveIntentions -= jLeft;
	}


	//----------------------------------------------------------------------------------------------------------
	// update
	float movementSpeed = MOVEMENT_SPEED;
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		movementSpeed *= FAST_MOVEMENT_MULTIPLIER;
	}

	Vec3 deltaPosition	   = ( moveIntentions * movementSpeed * deltaSeconds );
	Vec3 cameraPosition	   = GetCameraPosition();
	Vec3 newCameraPosition = cameraPosition + deltaPosition;
	SetCameraPosition( newCameraPosition );
}


void ThirdPersonController::UpdateOrientationFreeflow()
{
	// update camera
	IntVec2	   cursorDeltaPosition = g_theInput->GetCursorClientDelta();
	float	   smoothingFraction   = 0.1f;
	float	   deltaYaw			   = ( float ) cursorDeltaPosition.x * smoothingFraction;
	float	   deltaPitch		   = ( float ) cursorDeltaPosition.y * smoothingFraction;

	Quaternion camraOrientation	   = GetCameraOrientation();

	// yaw
	Quaternion yawRotation = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), -deltaYaw );
	camraOrientation	   = yawRotation * camraOrientation;

	// pitch
	Quaternion pitchRotation = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), deltaPitch );
	camraOrientation		 = camraOrientation * pitchRotation;
	camraOrientation.Normalize();

	SetCameraOrientation( camraOrientation );
}


void ThirdPersonController::Render() const
{
	DebugRenderCameraBasis();
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateMovementFollow( float deltaseconds )
{
	UpdateCameraOrientationInFollowMode();
	UpdateCharacterOreintationAndXYMovementInFollowMode( deltaseconds );
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateCameraOrientationInFollowMode()
{
	if ( g_theCharacter->m_movementState->GetStateName() != "idle" &&
		 g_theCharacter->m_movementState->GetStateName() != "walk" &&
		 g_theCharacter->m_movementState->GetStateName() != "run" )
	{
		return;
	}

	// update camera
	IntVec2	   cursorDeltaPosition = g_theInput->GetCursorClientDelta();
	float	   smoothingFraction   = 0.1f;
	float	   deltaYaw			   = ( float ) cursorDeltaPosition.x * smoothingFraction;
	float	   deltaPitch		   = ( float ) cursorDeltaPosition.y * smoothingFraction;

	Quaternion camraOrientation	   = GetCameraOrientation();

	// yaw
	Quaternion yawRotation = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), -deltaYaw );
	camraOrientation	   = yawRotation * camraOrientation;

	// pitch
	Quaternion pitchRotation = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), deltaPitch );
	camraOrientation		 = camraOrientation * pitchRotation;
	camraOrientation.Normalize();

	SetCameraOrientation( camraOrientation );

	// set camera to rotate around the player
	Vec3 const& characterPos		 = GetCharacterPosition();
	Vec3 const& cameraPos			 = GetCameraPosition();
	Vec3		characterToCameraDir = cameraPos - characterPos;

	Vec3		newCamerapos		 = characterPos + ( yawRotation * characterToCameraDir );

	// DebuggerPrintf( "Camera Pos: %.8f, %.8f, %.8f\n", newCamerapos.x, newCamerapos.y, newCamerapos.z );

	SetCameraPosition( newCamerapos );
}




//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateCharacterOreintationAndXYMovementInFollowMode( float deltaseconds )
{
	UNUSED( deltaseconds );

	if (g_theCharacter->m_movementState->GetStateName() == "idleDropToFreeHang")
	{
		return;
	}

	ApplyRootMotionTranslationToCharacter();
	if ( g_theCharacter->m_movementState->m_applyRootMotionTranslation )
	{
		return;
	}

	UpdateCharacterOrientation();
	UpdateCharacterMovementXY();
}



//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::ApplyRootMotionTranslationToCharacter()
{
	// update
	g_theCharacter->m_movementState->UpdateRootMotionTranslation();
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateCharacterOrientation()
{
	Quaternion	cameraOrientation	 = GetCameraOrientation();
	EulerAngles cameraEulerAngles	 = cameraOrientation.GetAsEulerAngles();
	cameraEulerAngles.m_rollDegrees	 = 0.f;
	cameraEulerAngles.m_pitchDegrees = 0.f;
	cameraOrientation				 = Quaternion::MakeFromEulerAngles( cameraEulerAngles );

	float	   deltaseconds			 = m_characterClock->GetDeltaSeconds();
	Quaternion characterOrientation	 = GetCharacterOrientation();
	characterOrientation			 = RotateTowards( characterOrientation, cameraOrientation, 50.f * deltaseconds );

	SetCharacterOrientation( characterOrientation );
}


float maxSpeed	   = 2.f;
//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::UpdateCharacterMovementXY()
{
	bool headBlocked = g_theCharacter->IsHeadInRangeOfAnObstacle( FloatRange( 0.f, 0.45f ) );
	bool footBlocked = g_theCharacter->IsFootInFrontOfAnObstacle( 0.45f );
	if ( headBlocked || footBlocked )
	{
		return;
	}

	bool isNotOnGround = !g_theCharacter->IsCharacterOnTheGround();
	bool isNearAnEdge  = !g_theCharacter->IsGroundRaycastHittingSomething();
	if ( isNotOnGround && isNearAnEdge )
	{
		return;
	}

	if ( g_theCharacter->m_movementState->GetStateName() != "walk" &&
		 g_theCharacter->m_movementState->GetStateName() != "run" && 
		g_theCharacter->m_movementState->GetStateName() != "idle" )
	{
		return;
	}

	Mat44 characterOrientationMatrix = GetCharacterOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3  iForward					 = characterOrientationMatrix.GetIBasis3D();
	iForward.z						 = 0.f;
	iForward.Normalize();

	float deltaseconds = m_characterClock->GetDeltaSeconds();

	// normal movement
	float acceleration	 = 0.5f;
	bool  running		 = false;
	Vec3  moveIntentions = iForward;
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		acceleration = 10.f;
		//moveIntentions += iForward;
		//float deltaSpeed  = acceleration /** deltaseconds*/;
		//m_characterSpeed   += deltaSpeed;

		if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
		{
			acceleration = 15.f;
			running		 = true;
		}
	}
	else
	{
		acceleration = 0.f;
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_SHIFT))
	{
		running = false;
	}

	//float deltaFriction = ( m_friction /** deltaseconds*/ );
		
	float previousCululativeSpeed = m_cumulativeSpeed;
	m_cumulativeSpeed = ( acceleration * deltaseconds ) + m_cumulativeSpeed - ( m_friction * deltaseconds );

	/*if (m_characterSpeed > 0.f)
	{
		m_characterSpeed -= ( m_friction * deltaseconds );
	}*/

	if ( running || (m_cumulativeSpeed < previousCululativeSpeed) )
	{
		maxSpeed = 10.f;
	}
	else
	{
		maxSpeed = 2.f;
	}

	if ( m_cumulativeSpeed > maxSpeed )
	{
		m_cumulativeSpeed = maxSpeed;
	}

	if ( m_cumulativeSpeed < 0.f )
	{
		m_cumulativeSpeed = 0.f;
	}

	//DebuggerPrintf( "character Speed: %f\n", m_characterSpeed );
	DebuggerPrintf( "cumulative Speed: %f\n", m_cumulativeSpeed );

	moveIntentions *= m_cumulativeSpeed;

	// fast movement
	/*float movementSpeed = MOVEMENT_SPEED;
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		movementSpeed *= FAST_MOVEMENT_MULTIPLIER;
	}*/

	// set character position
	
	Vec3  deltaPosition		   = ( moveIntentions/* * movementSpeed*/ * deltaseconds );
	Vec3  characterPosition	   = GetCharacterPosition();
	Vec3  newCharacterPosition = characterPosition + deltaPosition;
	SetCharacterPosition( newCharacterPosition );

	// set camera position
	Vec3 cameraPos = GetCameraPosition();
	SetCameraPosition( cameraPos + deltaPosition );
}


//----------------------------------------------------------------------------------------------------------
Vec3 ThirdPersonController::GetCharacterPosition() const
{
	return g_theCharacter->m_physics.m_position;
}


//----------------------------------------------------------------------------------------------------------
Quaternion ThirdPersonController::GetCharacterOrientation() const
{
	return g_theCharacter->m_physics.m_orientation;
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::SetCharacterPosition( Vec3 const& position )
{
	g_theCharacter->m_physics.m_position = position;
}


//----------------------------------------------------------------------------------------------------------
Vec3 ThirdPersonController::RotateTowardsCharacterOrientation( Vec3 const& translation )
{
	Quaternion const& characterOrientation = GetCharacterOrientation();
	Vec3			  orientedTranslation  = characterOrientation * translation;

	return orientedTranslation;
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::SetCharacterOrientation( Quaternion const& orientation )
{
	g_theCharacter->m_physics.m_orientation = orientation;
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::OrientAndAppendTranslationToCharacterPostion( Vec3 translation )
{
	translation							  = g_theCharacter->m_physics.m_orientation * translation;
	g_theCharacter->m_physics.m_position += translation;
}


//----------------------------------------------------------------------------------------------------------
Vec3 ThirdPersonController::GetCameraPosition() const
{
	return m_worldCamera->m_position;
}


//----------------------------------------------------------------------------------------------------------
Quaternion ThirdPersonController::GetCameraOrientation() const
{
	return m_worldCamera->GetOrientation();
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::SetCameraPosition( Vec3 const& position )
{
	m_worldCamera->m_position = position;
}

//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::SetCameraOrientation( Quaternion const& orientation )
{
	m_worldCamera->SetOrientation( orientation );
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::SetCameraPositionFromLocalTranslation( Vec3 const& initialPosition, Vec3 const& localTranslation )
{
	Quaternion const& cameraOrientation	  = GetCameraOrientation();
	Vec3			  orientedTranslation = cameraOrientation * localTranslation;
	Vec3			  newCameraPosition	  = initialPosition + orientedTranslation;
	SetCameraPosition( newCameraPosition );
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::CreateDebugBasisMesh()
{
	std::vector<Vertex_PCU> verts;

	Vec3					origin	 = Vec3( 0.f, 0.f, 0.f );
	Vec3					xForward = Vec3( 1.f, 0.f, 0.f );
	Vec3					yLeft	 = Vec3( 0.f, 1.f, 0.f );
	Vec3					zUp		 = Vec3( 0.f, 0.f, 1.f );

	float					radius	 = 0.1f;

	AddVertsForArrow3D( verts, origin, xForward, radius, Rgba8::RED );
	AddVertsForArrow3D( verts, origin, yLeft, radius, Rgba8::GREEN );
	AddVertsForArrow3D( verts, origin, zUp, radius, Rgba8::BLUE );

	m_debugBasisMesh				= g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	m_debugBasisMesh->m_vertexCount = ( unsigned int ) verts.size();
	g_theRenderer->CopyCPUToGPU( verts.data(), sizeof( Vertex_PCU ) * verts.size(), m_debugBasisMesh );
}


//----------------------------------------------------------------------------------------------------------
void ThirdPersonController::DebugRenderCameraBasis() const
{
	if ( !g_theApp->CanDebugDraw3D() )
	{
		return;
	}

	Quaternion const& cameraOrientation	 = GetCameraOrientation();
	Vec3			  characterPos		 = GetCharacterPosition();
	characterPos						+= Vec3( 0.f, 0.f, 1.1f );

	Mat44 transform						 = cameraOrientation.GetAsMatrix_XFwd_YLeft_ZUp();
	transform.SetTranslation3D( characterPos );
	transform.AppendScaleUniform3D( 0.3f );

	g_theRenderer->SetModelConstants( transform );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexBuffer( m_debugBasisMesh, m_debugBasisMesh->m_vertexCount );
}