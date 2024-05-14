#pragma once

#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

class Camera;
class Clock;
class Character;
class VertexBuffer;


//----------------------------------------------------------------------------------------------------------
enum class ControllerMode
{
	FreeRoam,
	Follow
};


//----------------------------------------------------------------------------------------------------------
class ThirdPersonController
{
public:
	ThirdPersonController();
	virtual ~ThirdPersonController();

	void Startup();
	void Update();
	void Render() const;
	void Shutdown() {}

	// movement
	ControllerMode m_controllerMode = ControllerMode::Follow;
protected:
	void UpdateControllerMode();
	void UpdateMovement();
	void UpdateMovementFreeRoam( float deltaseconds );
	void UpdateMovementFreeflow( float deltaseconds );
	void UpdateOrientationFreeflow();
	void UpdateMovementFollow( float deltaseconds );

	// character
protected:
	float m_cumulativeSpeed = 0.f;
	float m_friction	   = 5.f;
	void UpdateCharacterOreintationAndXYMovementInFollowMode( float deltaseconds );
	void UpdateCharacterOrientation();
	void UpdateCharacterMovementXY();
	void ApplyRootMotionTranslationToCharacter();
public:
	Clock*	   m_characterClock = nullptr;
	Vec3	   GetCharacterPosition() const;
	Quaternion GetCharacterOrientation() const;
	void	   SetCharacterPosition( Vec3 const& position );
	Vec3	   RotateTowardsCharacterOrientation( Vec3 const& translation );
	void	   OrientAndAppendTranslationToCharacterPostion( Vec3 translation );
	void	   SetCharacterOrientation( Quaternion const& orientation );

	// camera
protected:
	bool   m_lockCamera	 = false;
	Clock* m_cameraClock = nullptr;
public:
	Camera*	   m_worldCamera = nullptr;
	void	   UpdateCameraOrientationInFollowMode();
	Vec3	   GetCameraPosition() const;
	Quaternion GetCameraOrientation() const;
	void	   SetCameraPosition( Vec3 const& position );
	void	   SetCameraOrientation( Quaternion const& orientation );
	void	   SetCameraPositionFromLocalTranslation( Vec3 const& initialPosition, Vec3 const& localTranslation );

	// camera debug basis
protected:
	VertexBuffer* m_debugBasisMesh = nullptr;
	void		  CreateDebugBasisMesh();
	void		  DebugRenderCameraBasis() const;
};