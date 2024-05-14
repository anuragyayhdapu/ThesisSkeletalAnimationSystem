#include "Game/MovementState.hpp"


//----------------------------------------------------------------------------------------------------------
class VaultMovementState : public MovementState
{
public:
	VaultMovementState();
	virtual ~VaultMovementState() {}

	virtual std::string	   GetStateName() const { return "vault"; }
	virtual MovementState* UpdateTransition() override;
	virtual void		   UpdateRootMotionTranslation() override;

	//----------------------------------------------------------------------------------------------------------
	void FixRootMotionInXForwardDirection();
	void RemoveRootMotionInZUpDirection();
};


//----------------------------------------------------------------------------------------------------------
class RunningSlide : public MovementState
{
public:
	RunningSlide();
	virtual ~RunningSlide() {}

	virtual std::string	   GetStateName() const { return "runningSlide"; }
	virtual MovementState* UpdateTransition() override;
	virtual void		   UpdateRootMotionTranslation() override;
};