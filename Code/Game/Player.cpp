
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"


constexpr float MOVEMENT_SPEED = 2.f;
constexpr float FAST_MOVEMENT_MULTIPLIER = 10.f;
constexpr float MIN_PITCH_DEGREES = -85.f;
constexpr float MAX_PITCH_DEGREES = 85.f;
constexpr float MIN_ROLL_DEGREES = -45.f;
constexpr float MAX_ROLL_DEGREES = 45.f;


Player::Player()
{
	Startup();

	m_playerClock = new Clock();
	//m_angularVelocity = EulerAngles(90.f, 90.f, 90.f);
}

Player::~Player()
{
}


void Player::Startup()
{
	m_worldCamera = new Camera();

	float aspect = g_theRenderer->GetConfig().m_window->GetConfig().m_clientAspect;
	float fovDegrees = 60.f;
	float zNearPlane = 0.1f;
	float zFarPlane = 10000.f;
	m_worldCamera->SetPerspectiveView(aspect, fovDegrees, zNearPlane, zFarPlane);

	Vec3 d3dIBasis(0.f, 0.f, 1.f);
	Vec3 d3dJBasis(-1.f, 0.f, 0.f);
	Vec3 d3dKBasis(0.f, 1.f, 0.f);
	m_worldCamera->SetRenderBasis(d3dIBasis, d3dJBasis, d3dKBasis);
}

void Player::Update()
{
	bool isCurrentWindowFocused = g_theWindow->DoesCurrentWindowHaveFocus();
	bool isDevConsoleClosed = !g_theDevConsole->IsOpen();
	if (isCurrentWindowFocused && isDevConsoleClosed)
	{
		float deltaseconds = m_playerClock->GetDeltaSeconds();

		UpdatePlayerMovement(deltaseconds);
	}

	// 'O' reset player 
	if (g_theInput->WasKeyJustPressed('R'))
	{
		m_position = Vec3(5.61f, -2.88f, 3.28f);
		m_orientation = EulerAngles( 152.40f, 19.10f, 0.f );
	}
}


Mat44 Player::GetModelMatrix() const
{
	Mat44 modelMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(m_position);

	return modelMatrix;
}

void Player::UpdatePlayerMovement(float deltaseconds)
{
	UpdateVerticalMovement(deltaseconds);
	UpdateHorizontalMovement(deltaseconds);
	UpdateOrientation();

	// update camera
	m_worldCamera->SetTransform(m_position, m_orientation);
}

void Player::UpdateVerticalMovement(float deltaSeconds)
{
	// world up and down using key 'W' & 'R'
	Vec3 moveIntentions = Vec3(0.f, 0.f, 0.f);
	Vec3 worldUp = Vec3(0.f, 0.f, 1.f);

	if (g_theInput->IsKeyDown('Q'))
	{
		moveIntentions -= worldUp;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		moveIntentions += worldUp;
	}

	float movementSpeed = MOVEMENT_SPEED;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		movementSpeed *= FAST_MOVEMENT_MULTIPLIER;
	}

	Vec3 verticalDeltaPosition = (moveIntentions * movementSpeed * deltaSeconds);
	m_position += verticalDeltaPosition;
}

void Player::UpdateHorizontalMovement(float deltaseconds)
{
	Vec3 iForward;
	Vec3 jLeft;
	Vec3 kUp;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(iForward, jLeft, kUp);
	iForward.z = 0.f;
	iForward.Normalize();
	jLeft.z = 0.f;
	jLeft.Normalize();

	Vec3 moveIntentions = Vec3(0.f, 0.f, 0.f);
	if (g_theInput->IsKeyDown('W'))
	{
		moveIntentions += iForward;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveIntentions -= iForward;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveIntentions += jLeft;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveIntentions -= jLeft;
	}

	float movementSpeed = MOVEMENT_SPEED;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		movementSpeed *= FAST_MOVEMENT_MULTIPLIER;
	}

	Vec3 horizontalDeltaPosition = (moveIntentions * movementSpeed * deltaseconds);
	m_position += horizontalDeltaPosition;
}

void Player::UpdateOrientation()
{
	bool isWindowInFocus = g_theWindow->DoesCurrentWindowHaveFocus();
	if (isWindowInFocus)
	{
		// hide mouse and set relative mode
		bool isCursorHidden = true;
		bool isCursorRelative = true;
		g_theInput->SetCursorMode(isCursorHidden, isCursorRelative);

		// update camera
		IntVec2 cursorDeltaPosition = g_theInput->GetCursorClientDelta();
		float smoothingFraction = 0.1f;
		float deltaYaw = (float)cursorDeltaPosition.x * smoothingFraction;
		float deltaPitch = (float)cursorDeltaPosition.y * smoothingFraction;

		m_orientation.m_yawDegrees -= deltaYaw;
		float pitch = GetClamped(m_orientation.m_pitchDegrees + deltaPitch, -89.9f, 89.9f);
		m_orientation.m_pitchDegrees = pitch;
	}
	else // isWindowInFocus = false
	{
		// show mouse and disable relative mode
		bool isCursorHidden = false;
		bool isCursorRelative = false;
		g_theInput->SetCursorMode(isCursorHidden, isCursorRelative);
	}
}


void Player::ClampOrientation()
{
	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, MIN_PITCH_DEGREES, MAX_PITCH_DEGREES);
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, MIN_ROLL_DEGREES, MAX_ROLL_DEGREES);
}


void Player::Render() const
{
}

