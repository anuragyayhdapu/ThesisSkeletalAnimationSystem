#include "Game/MovementState.hpp"


//----------------------------------------------------------------------------------------------------------
class ClimbOver : public MovementState
{
public:
	ClimbOver();
	virtual ~ClimbOver() {}

	virtual std::string	   GetStateName() const override { return "climbOver"; }
	virtual MovementState* UpdateTransition() override;
	virtual void		   NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation ) override;

	MovementState*		   m_nextState = nullptr;
};


//----------------------------------------------------------------------------------------------------------
class ClimbOverCrouchToStandingIdle : public MovementState
{
public:
	ClimbOverCrouchToStandingIdle();
	virtual ~ClimbOverCrouchToStandingIdle() override {}

	virtual std::string	   GetStateName() const override { return "crouchToStand"; }
	virtual MovementState* UpdateTransition() override;
	virtual void		   NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation ) override;

	MovementState*		   m_nextState = nullptr;
};


//----------------------------------------------------------------------------------------------------------
class WalkingEdgeSlip : public MovementState
{
public:
	WalkingEdgeSlip();
	virtual ~WalkingEdgeSlip() override {}

	virtual std::string	   GetStateName() const override { return "walkingEdgeSlip"; }
	virtual MovementState* UpdateTransition() override;

	MovementState*		   m_nextState = nullptr;

	void				   FixRootMotionCurve();
	void				   FixCharacterPosition();
};


//----------------------------------------------------------------------------------------------------------
class IdleToActionIdleTransition : public MovementState
{
public:
	IdleToActionIdleTransition();
	virtual ~IdleToActionIdleTransition() override {}

	virtual std::string	   GetStateName() const override { return "idleToActionIdle"; }
	virtual MovementState* UpdateTransition() override;

	MovementState*		   m_nextState = nullptr;

	void				   FixCharacterPosition();
};


//----------------------------------------------------------------------------------------------------------
class IdleDropToFreeHang : public MovementState
{
public:
	IdleDropToFreeHang();
	virtual ~IdleDropToFreeHang() override {}

	virtual std::string	   GetStateName() const override { return "idleDropToFreeHang"; }
	virtual MovementState* UpdateTransition() override;
	virtual void		   NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation ) override;

	void				   RemoveYRootMotionTranslationFromAnimation();
	void				   NormalizeXRootMotionTranslation();
	void				   ShiftZRootMotionTranslationToCurrentPositionInAnimation();
	void				   SampleXRootMotionAndSetCharacterPosition();
	void				   ReverseCharacterAndCameraOrientation();
	void				   SetCharacterFinalZPosition();

	Vec3				   m_characterInitialPosition = Vec3::ZERO;
	MovementState*		   m_nextState				  = nullptr;
};


//----------------------------------------------------------------------------------------------------------
class FreeHangToBracedHang : public MovementState
{
public:
	FreeHangToBracedHang();
	virtual ~FreeHangToBracedHang() override {}

	virtual std::string	   GetStateName() const override { return "freeHangToBracedHang"; }
	virtual MovementState* UpdateTransition() override;
	virtual void		   NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation ) override;

	MovementState*		   m_nextMovementState = nullptr;
};