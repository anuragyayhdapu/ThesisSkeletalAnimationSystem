#include "Game/MovementState.hpp"



//----------------------------------------------------------------------------------------------------------
class IdleToLedgeGrabState : public MovementState
{
public:
	IdleToLedgeGrabState();
	virtual ~IdleToLedgeGrabState() {}

	virtual std::string	   GetStateName() const { return "hang"; }
	virtual MovementState* UpdateTransition() override;

	//----------------------------------------------------------------------------------------------------------
	MovementState* m_nextState = nullptr;
	virtual void   NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation ) override;

	Vec3		   m_cameraLerpStartValue = Vec3::ZERO;
	Vec3		   m_cameraLerpEndValue	= Vec3::ZERO;
};


//----------------------------------------------------------------------------------------------------------
class HangingIdleState : public MovementState
{
public:
	HangingIdleState();
	virtual ~HangingIdleState() {}

	virtual std::string	   GetStateName() const { return "hang"; }
	virtual MovementState* UpdateTransition() override;
};


//----------------------------------------------------------------------------------------------------------
class HangingIdleToDropState : public MovementState
{
public:
	HangingIdleToDropState();
	virtual ~HangingIdleToDropState() {}

	virtual std::string	   GetStateName() const { return "hangDrop"; }
	virtual MovementState* UpdateTransition() override;

	//----------------------------------------------------------------------------------------------------------
	float m_characterStartZPos = 0.f;
	Vec3  m_cameraLerpStartValue = Vec3::ZERO;
	Vec3  m_cameraLerpEndValue	 = Vec3::ZERO;
};


//----------------------------------------------------------------------------------------------------------
class ShimmyRight : public MovementState
{
public:
	ShimmyRight();
	virtual ~ShimmyRight() {}

	virtual std::string	   GetStateName() const { return "shimmyRight"; }
	virtual MovementState* UpdateTransition() override;

	Vec3				   m_characterStartPos			   = Vec3::ZERO;
	Vec3				   m_characterEndPos			   = Vec3::ZERO;
	Vec3				   m_characterToCameraDisplacement = Vec3::ZERO;
};


//----------------------------------------------------------------------------------------------------------
class ShimmyLeft : public MovementState
{
public:
	ShimmyLeft();
	virtual ~ShimmyLeft() {}

	virtual std::string	   GetStateName() const { return "shimmyLeft"; }
	virtual MovementState* UpdateTransition() override;

	Vec3				   m_characterStartPos			   = Vec3::ZERO;
	Vec3				   m_characterEndPos			   = Vec3::ZERO;
	Vec3				   m_characterToCameraDisplacement = Vec3::ZERO;
};