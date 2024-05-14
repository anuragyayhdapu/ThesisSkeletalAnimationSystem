#pragma once

#include "Engine/Animation/AnimCurve.hpp"

#include <string>

class Character;


//----------------------------------------------------------------------------------------------------------
class MovementState
{
public:
	MovementState();
	virtual ~MovementState() {}

	virtual std::string	   GetStateName() const = 0;
	virtual MovementState* UpdateTransition()	= 0;

	Vec3AnimCurve		   m_rootMotionTranslationCurve;
	bool				   m_applyRootMotionTranslation		  = false;

	Vec3				   m_debugSampledUnrotatedtranslation = Vec3::ZERO;
	Vec3				   m_previousFrameTranslation		  = Vec3::ZERO;
	Vec3				   m_thisFrameTranslation			  = Vec3::ZERO;
	float				   m_sampleTimeMs					  = 0.f;
	void				   InitRootMotionTranslation();
	virtual void		   UpdateRootMotionTranslation();
	Vec3				   GetRootMotionDeltaTranslation() const;

	bool				   m_notifyStateChangeToAnimation = true;
	virtual void		   NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation ) { ( void ) ( triggerTransitionRequestToAnimation ); }

	Vec3				   m_initialCameraPos = Vec3::ZERO;
	Vec3				   m_finalCameraPos	  = Vec3::ZERO;
	void				   InitCameraPosition();
	void				   LerpCameraPosition();
};


//----------------------------------------------------------------------------------------------------------
class IdleMovementState : public MovementState
{
public:
	IdleMovementState();
	virtual ~IdleMovementState() {}

	virtual std::string	   GetStateName() const { return "idle"; }
	virtual MovementState* UpdateTransition() override;
};


//----------------------------------------------------------------------------------------------------------
class WalkMovementState : public MovementState
{
public:
	WalkMovementState();
	virtual ~WalkMovementState() {}

	virtual std::string	   GetStateName() const { return "walk"; }
	virtual MovementState* UpdateTransition() override;

	MovementState*		   CheckTransitionToEdgeSlip();
};


//----------------------------------------------------------------------------------------------------------
class RunMovementState : public MovementState
{
public:
	RunMovementState();
	virtual ~RunMovementState() {}

	virtual std::string	   GetStateName() const { return "run"; }
	virtual MovementState* UpdateTransition() override;
};


//----------------------------------------------------------------------------------------------------------
class RunStop : public MovementState
{
public:
	RunStop();
	virtual ~RunStop() {}

	virtual std::string	   GetStateName() const { return "runStop"; }
	virtual MovementState* UpdateTransition() override;

	void				   RemoveZRootMotion();
	void				   RemoveYRootMotion();
	void				   NormalizeXRootMotion();
	void				   SampleCurveAndSetCharacterPosition();
	void				   SampleCurveAndSetCameraPosition();

	Vec3				   m_initialCharacterPos = Vec3::ZERO;
	Vec3				   m_initialCameraPos	 = Vec3::ZERO;
};


//----------------------------------------------------------------------------------------------------------
class JumpMovementState : public MovementState
{
public:
	JumpMovementState();
	virtual ~JumpMovementState() {}

	virtual std::string	   GetStateName() const { return "jump"; }
	virtual MovementState* UpdateTransition() override;
};


//----------------------------------------------------------------------------------------------------------
class CrouchMovementState : public MovementState
{
public:
	CrouchMovementState();
	virtual ~CrouchMovementState() {}

	virtual std::string	   GetStateName() const { return "crouch"; }
	virtual MovementState* UpdateTransition() override;
};


//----------------------------------------------------------------------------------------------------------
class CrouchWalkMovementState : public MovementState
{
public:
	CrouchWalkMovementState();
	virtual ~CrouchWalkMovementState() {}

	virtual std::string	   GetStateName() const { return "walk"; }
	virtual MovementState* UpdateTransition() override;
};