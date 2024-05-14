#include "Game/ParkourLedgeHangStates.hpp"
#include "Game/ParkourClimbStates.hpp"
#include "Game/ThirdPersonController.hpp"
#include "Game/Character.hpp"
#include "Game/AnimationState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimClip.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//----------------------------------------------------------------------------------------------------------
ClimbOver::ClimbOver()
{
	// fix the z position of root joint animation, to start at current position of the character
	AnimationState*		 state							= AnimationState::GetAnimationStateByName( "climbOver" );
	AnimationState*		 hangState						= AnimationState::GetAnimationStateByName( "hang" );
	Vec3AnimCurve const& hangRootMotionTranslationCurve = hangState->m_clip->GetRootJointTranslationCurve();
	float				 lastHangingHeight				= hangRootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value.z;
	Vec3				 charCurrentPos					= g_theCharacter->m_physics.m_position;
	Vec3AnimCurve&		 rootJointTranslationCurve		= state->m_clip->GetRootJointTranslationCurveByRefernce();
	float				 climbOverCurveFirstZValue		= rootJointTranslationCurve.GetKeyframeAtFirstIndex().m_value.z;
	float				 climbOverCurveLastZValue		= rootJointTranslationCurve.GetKeyframeAtLastIndex().m_value.z;
	float				 climbDistance					= 2.0f;
	float				 newStartHeightZ				= lastHangingHeight;
	float				 newEndHeightZ					= lastHangingHeight + climbDistance;

	for ( unsigned int index = 0; index < rootJointTranslationCurve.GetSize(); index++ )
	{
		Vec3& rootTranslationValue = rootJointTranslationCurve.m_keyframes[ index ].m_value;
		rootTranslationValue.y	   = 0.f;
		float& zValue			   = rootTranslationValue.z;

		zValue					   = GetFractionWithinRange( zValue, climbOverCurveFirstZValue, climbOverCurveLastZValue );
		zValue					   = Lerp( newStartHeightZ, newEndHeightZ, zValue );
	}

	// set the root motion curve for debug display
	m_rootMotionTranslationCurve = state->GetRootMotionTranslation();

	InitCameraPosition();
}


//----------------------------------------------------------------------------------------------------------
MovementState* ClimbOver::UpdateTransition()
{
	if ( m_nextState == nullptr )
	{
		LerpCameraPosition();
	}

	return m_nextState;
}


//----------------------------------------------------------------------------------------------------------
void ClimbOver::NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation )
{
	m_notifyStateChangeToAnimation = triggerTransitionRequestToAnimation;

	// fix the z height here
	Vec3			finalRootBonePosOfCurrentState	= m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value;
	Vec3			startRootBonePosOfCurrentState	= m_rootMotionTranslationCurve.GetKeyframeAtFirstIndex().m_value;
	AnimationState* crouchIdleState					= AnimationState::GetAnimationStateByName( "crouchToStand" );
	Vec3			startRootBonePosOfNextState		= crouchIdleState->GetFirstKeyframeRootMotionTranslation();

	Vec3			characterPosTranslation			= Vec3::ZERO;
	float			upDiff							= finalRootBonePosOfCurrentState.z - startRootBonePosOfNextState.z;
	float			forwardDiff						= ( finalRootBonePosOfCurrentState.x - startRootBonePosOfCurrentState.x );
	Vec3			characterForwardDir				= g_theCharacter->m_physics.m_orientation.GetVector_XFwd();
	Vec3			characterUpDir					= g_theCharacter->m_physics.m_orientation.GetVector_ZUp();
	characterPosTranslation							= ( characterForwardDir * forwardDiff ) + ( characterUpDir * upDiff );
	g_theCharacter->m_physics.m_position		   += characterPosTranslation;

	// set next state
	m_nextState = new ClimbOverCrouchToStandingIdle();
}


//----------------------------------------------------------------------------------------------------------
ClimbOverCrouchToStandingIdle::ClimbOverCrouchToStandingIdle()
{
	// fix forward translation of 'crouch' to start at the end translation of 'standToCrouch'
	AnimationState* crouchToStandState			= AnimationState::GetAnimationStateByName( "crouchToStand" );
	Vec3AnimCurve&	crouhToStandRootMotionCurve = crouchToStandState->m_clip->GetRootJointTranslationCurveByRefernce();
	float			crouchToStandFirstX			= crouchToStandState->GetFirstKeyframeRootMotionTranslation().x;
	float			crouchToStandLastX			= crouchToStandState->GetLastKeyframeRootMotionTranslation().x;
	for ( unsigned int index = 0; index < crouhToStandRootMotionCurve.GetSize(); index++ )
	{
		Vec3& translation = crouhToStandRootMotionCurve.m_keyframes[ index ].m_value;
		translation.x	  = RangeMap( translation.x, crouchToStandFirstX, crouchToStandLastX, 0.f, 0.5f );
	}

	// InitCameraTranslationLerpParameters();

	m_rootMotionTranslationCurve = crouchToStandState->GetRootMotionTranslation();

	InitCameraPosition();
}


//----------------------------------------------------------------------------------------------------------
MovementState* ClimbOverCrouchToStandingIdle::UpdateTransition()
{
	if ( m_nextState == nullptr )
	{
		LerpCameraPosition();
	}

	return m_nextState;
}


//----------------------------------------------------------------------------------------------------------
void ClimbOverCrouchToStandingIdle::NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation )
{
	// fix the x forward position of the character's position
	AnimationState* crouchToStandIdleState		   = AnimationState::GetAnimationStateByName( "crouchToStand" );
	Vec3			previousAnimationLastKeyframe  = crouchToStandIdleState->GetLastKeyframeRootMotionTranslation();
	float			xDiff						   = previousAnimationLastKeyframe.x;
	Vec3			characterForward			   = g_theCharacter->m_physics.m_orientation.GetVector_XFwd();
	Vec3			characterTranslation		   = xDiff * characterForward;
	g_theCharacter->m_physics.m_position		  += characterTranslation;

	m_notifyStateChangeToAnimation				   = triggerTransitionRequestToAnimation;
	m_nextState									   = new IdleMovementState();
}


//----------------------------------------------------------------------------------------------------------
WalkingEdgeSlip::WalkingEdgeSlip()
{
	FixRootMotionCurve();

	AnimationState* currentAnimationState = AnimationState::GetAnimationStateByName( "walkingEdgeSlip" );
	m_rootMotionTranslationCurve		  = currentAnimationState->GetRootMotionTranslation();

	InitCameraPosition();
}


//----------------------------------------------------------------------------------------------------------
MovementState* WalkingEdgeSlip::UpdateTransition()
{
	LerpCameraPosition();

	if ( g_theAnimationController->IsCurrentAnimationAtEndOfState() )
	{
		FixCharacterPosition();
		return new IdleToActionIdleTransition();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
void WalkingEdgeSlip::FixRootMotionCurve()
{
	// 1. fix idleToActionIdle according to idle
	AnimationState* actionIdleAnimationState	   = AnimationState::GetAnimationStateByName( "idle" );
	Vec3			actionIdleFirstkeyFramePos	   = actionIdleAnimationState->GetFirstKeyframeRootMotionTranslation();
	AnimationState* idleToActionIdleAnimationState = AnimationState::GetAnimationStateByName( "idleToActionIdle" );
	idleToActionIdleAnimationState->RemoveRootMotionTranslationOfYPos();
	idleToActionIdleAnimationState->EndRootMotionZPosAtFollowingZValue( actionIdleFirstkeyFramePos.z );

	// 2. fix walkingEdleSlip according to idleToActionIdle
	Vec3			idleToActionIdleFirstkeyFramePos = idleToActionIdleAnimationState->GetFirstKeyframeRootMotionTranslation();
	AnimationState* edgeSlipAnimationState			 = AnimationState::GetAnimationStateByName( "walkingEdgeSlip" );
	edgeSlipAnimationState->RemoveRootMotionTranslationOfYPos();
	edgeSlipAnimationState->EndRootMotionZPosAtFollowingZValue( idleToActionIdleFirstkeyFramePos.z );
}


//----------------------------------------------------------------------------------------------------------
void WalkingEdgeSlip::FixCharacterPosition()
{
	AnimationState* oldAnimationState			= AnimationState::GetAnimationStateByName( "walkingEdgeSlip" );
	Vec3			rootMotionTranslationDelta	= oldAnimationState->GetLastAndFirstKeyframeRootMotionTranslationDelta();
	Vec3			rootMotionXTranslation		= Vec3( rootMotionTranslationDelta.x, 0.f, 0.f );
	Quaternion		characterOrientation		= g_theThirdPersonController->GetCharacterOrientation();
	rootMotionXTranslation						= characterOrientation * rootMotionXTranslation;
	g_theCharacter->m_physics.m_position	   += rootMotionXTranslation;
}


//----------------------------------------------------------------------------------------------------------
IdleToActionIdleTransition::IdleToActionIdleTransition()
{
	AnimationState* currentAnimationState = AnimationState::GetAnimationStateByName( "idleToActionIdle" );
	m_rootMotionTranslationCurve		  = currentAnimationState->GetRootMotionTranslation();

	InitCameraPosition();
}


//----------------------------------------------------------------------------------------------------------
MovementState* IdleToActionIdleTransition::UpdateTransition()
{
	LerpCameraPosition();

	if ( g_theAnimationController->IsCurrentAnimationAtEndOfState() )
	{
		FixCharacterPosition();

		return new IdleMovementState();
	}

	return nullptr;
	// return m_nextState;
}


//----------------------------------------------------------------------------------------------------------
void IdleToActionIdleTransition::FixCharacterPosition()
{
	// 1. get end of Idle to action idle
	Vec3 endOfPreviosAnim = AnimationState::GetAnimationStateByName( "idleToActionIdle" )->GetLastKeyframeRootMotionTranslation();
	// 2. get beginning of action idle
	Vec3 startOfNextAnim = AnimationState::GetAnimationStateByName( "idle" )->GetFirstKeyframeRootMotionTranslation();
	// 3. subtract the m_position by the difference
	Vec3 diff = startOfNextAnim - endOfPreviosAnim;
	// diff.y							= 0.f;
	diff.z								  = 0.f;
	Quaternion characterOrientation		  = g_theThirdPersonController->GetCharacterOrientation();
	diff								  = characterOrientation * diff;
	g_theCharacter->m_physics.m_position -= diff;
}


//----------------------------------------------------------------------------------------------------------
IdleDropToFreeHang::IdleDropToFreeHang()
{
	//m_applyRootMotionTranslation = true;
	//g_theCharacter->m_physics.m_position.z = 0.f;
	RemoveYRootMotionTranslationFromAnimation();
	NormalizeXRootMotionTranslation();
	ShiftZRootMotionTranslationToCurrentPositionInAnimation();

	m_rootMotionTranslationCurve = AnimationState::GetAnimationStateByName( GetStateName() )->GetRootMotionTranslation();

	m_characterInitialPosition	 = g_theThirdPersonController->GetCharacterPosition();
}


//----------------------------------------------------------------------------------------------------------
MovementState* IdleDropToFreeHang::UpdateTransition()
{
	if (m_nextState == nullptr)
	{
		SampleXRootMotionAndSetCharacterPosition();
	}

	//if ( g_theAnimationController->IsCurrentAnimationAtEndOfState() )
	//{
	//	//g_theCharacter->m_physics.m_position.z = 0.33186f;
	//	/*ReverseCharacterAndCameraOrientation();
	//	SetCharacterFinalZPosition();*/

	//	//return new FreeHangToBracedHang();
	//}

	return m_nextState;
}


//constexpr float FREE_HANG_HEIGHT = 0.1f;
//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation )
{
	m_notifyStateChangeToAnimation = triggerTransitionRequestToAnimation;

	ReverseCharacterAndCameraOrientation();
	
	//// shift by this animation's last keyframe
	//float thisAnimationZValue		   = m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value.z;
	//g_theCharacter->m_physics.m_position.z += thisAnimationZValue;

	//// shift by next animation's first keyframe
	//AnimationState* nextAnimationState	   = AnimationState::GetAnimationStateByName( "freeHangToBracedHang" );
	//float			nextAnimationZValue	   = nextAnimationState->GetFirstKeyframeRootMotionTranslation().z;
	//g_theCharacter->m_physics.m_position.z -= nextAnimationZValue;
	float obstacleHeight				   = g_theCharacter->m_latestObstacleHeight;
	g_theCharacter->m_physics.m_position.z = -0.1f + ( obstacleHeight - 3.25f );

	m_nextState							   = new FreeHangToBracedHang();
}


//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::RemoveYRootMotionTranslationFromAnimation()
{
	AnimationState* idleDropToFreeHangState		   = AnimationState::GetAnimationStateByName( GetStateName() );
	Vec3AnimCurve&	animationRootMotionTranslation = idleDropToFreeHangState->m_clip->GetRootJointTranslationCurveByRefernce();
	for ( unsigned int index = 0; index < animationRootMotionTranslation.GetSize(); index++ )
	{
		Vec3& translation = animationRootMotionTranslation.m_keyframes[ index ].m_value;
		translation.y	  = 0.f;
		//translation.x	  = 0.f;
	}
}


//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::NormalizeXRootMotionTranslation()
{
	AnimationState* idleDropToFreeHangState		   = AnimationState::GetAnimationStateByName( GetStateName() );
	Vec3AnimCurve&	animationRootMotionTranslation = idleDropToFreeHangState->m_clip->GetRootJointTranslationCurveByRefernce();
	float			xMin;
	float			xMax;
	animationRootMotionTranslation.GetMaxAndMinXValuesFromCurve( xMin, xMax );
	for ( unsigned int index = 0; index < animationRootMotionTranslation.GetSize(); index++ )
	{
		Vec3&  translation	 = animationRootMotionTranslation.m_keyframes[ index ].m_value;
		float& xTranslation	 = translation.x;
		xTranslation		-= xMin;
		xTranslation		/= xMax;
	}
}


//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::ShiftZRootMotionTranslationToCurrentPositionInAnimation()
{
	// current animation
	Vec3AnimCurve const& currentRMTCurve	= g_theAnimationController->GetCurrentStateRootMotionTranslation();
	float				 currentLocalTimeMs = g_theAnimationController->GetLocalTimeMsOfCurrentAnimation();
	Vec3				 currentRootBonePos = currentRMTCurve.Sample( currentLocalTimeMs, false );

	// next animation
	AnimationState* idleDropToFreeHangState		   = AnimationState::GetAnimationStateByName( GetStateName() );
	Vec3AnimCurve&	animationRootMotionTranslation = idleDropToFreeHangState->m_clip->GetRootJointTranslationCurveByRefernce();
	Vec3			firstTranslation			   = animationRootMotionTranslation.GetKeyframeAtFirstIndex().m_value;
	float			diffZ						   = firstTranslation.z - currentRootBonePos.z;
	for ( unsigned int index = 0; index < animationRootMotionTranslation.GetSize(); index++ )
	{
		Vec3& translation  = animationRootMotionTranslation.m_keyframes[ index ].m_value;
		translation.z	  -= diffZ;
	}
}


//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::SampleXRootMotionAndSetCharacterPosition()
{
	float localTimeMs			= g_theAnimationController->GetLocalTimeMsOfCurrentAnimation();
	Vec3  normalizedTranslation = m_rootMotionTranslationCurve.Sample( localTimeMs, false );
	normalizedTranslation.y		= 0.f;
	normalizedTranslation.z		= 0.f;
	normalizedTranslation		= g_theThirdPersonController->RotateTowardsCharacterOrientation( normalizedTranslation );
	float scale					= 1.2f;
	Vec3  newCharacterPosition	= m_characterInitialPosition + ( normalizedTranslation * scale );

	g_theThirdPersonController->SetCharacterPosition( newCharacterPosition );
}


//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::ReverseCharacterAndCameraOrientation()
{
	// reverse character orientation
	Quaternion const& characterOrientation		  = g_theThirdPersonController->GetCharacterOrientation();
	Quaternion		  rotationAroundZAxis		  = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), 180.f );
	Quaternion		  rotatedCharacterOrientation = characterOrientation * rotationAroundZAxis;
	g_theThirdPersonController->SetCharacterOrientation( rotatedCharacterOrientation );

	// put camera behind the character
	Vec3 characterToCameraVector = g_theThirdPersonController->GetCameraPosition() - g_theThirdPersonController->GetCharacterPosition();
	characterToCameraVector		 = rotationAroundZAxis * characterToCameraVector;

	Vec3 newCharacterPos							   = g_theCharacter->m_physics.m_position;
	newCharacterPos.z							   = 0.1f;
	//g_theCharacter->m_physics.m_position.z = -0.1f;


	Vec3 newCameraPosition = characterToCameraVector + newCharacterPos;
	g_theThirdPersonController->SetCameraPosition( newCameraPosition );

	// rotate camera orientation
	Quaternion newCameraOrientation = g_theThirdPersonController->GetCameraOrientation();
	newCameraOrientation			= newCameraOrientation * rotationAroundZAxis;
	g_theThirdPersonController->SetCameraOrientation( newCameraOrientation );
}


//----------------------------------------------------------------------------------------------------------
void IdleDropToFreeHang::SetCharacterFinalZPosition()
{
	AnimationState* currentState		  = AnimationState::GetAnimationStateByName( GetStateName() );
	Vec3			translationDelta	  = currentState->GetLastAndFirstKeyframeRootMotionTranslationDelta();
	translationDelta.x					  = 0.f;
	translationDelta.y					  = 0.f;

	g_theCharacter->m_physics.m_position += translationDelta;
}


//----------------------------------------------------------------------------------------------------------
FreeHangToBracedHang::FreeHangToBracedHang()
{
	m_applyRootMotionTranslation = false;

	//// fix the root motion curve to start where the previous one ends
	//AnimationState*		 hangState							= AnimationState::GetAnimationStateByName( "hang" );
	//Vec3AnimCurve const& hangRootMotionCurve				= hangState->m_clip->GetRootJointTranslationCurve();
	//float				 hangAnimationZRootMotionStartvalue = hangRootMotionCurve.GetKeyframeAtFirstIndex().m_value.z;
	//float				 initValue							= hangAnimationZRootMotionStartvalue;

	//AnimationState*		 currentState						= AnimationState::GetAnimationStateByName( GetStateName() );
	//Vec3AnimCurve&		 currentRootMotionCurve				= currentState->m_clip->GetRootJointTranslationCurveByRefernce();
	//float				 scaleValue							= currentState->GetLastAndFirstKeyframeRootMotionTranslationDelta().z;
	//float				 minZValue;
	//float				 maxZValue;
	//currentRootMotionCurve.GetMaxAndMinZValuesFromCurve( minZValue, maxZValue );

	//for (unsigned int index = 0; index < currentRootMotionCurve.GetSize(); index++)
	//{
	//	Vec3& keyValue = currentRootMotionCurve.m_keyframes[ index ].m_value;
	//	float& zvalue	= keyValue.z;

	//	// normalize
	//	zvalue			= RangeMap( zvalue, minZValue, maxZValue, 0.f, 1.f );

	//	// scale and start at init value
	//	zvalue *= scaleValue;
	//	zvalue += initValue;
	//}


	m_rootMotionTranslationCurve = AnimationState::GetAnimationStateByName( GetStateName() )->GetRootMotionTranslation();
}


//----------------------------------------------------------------------------------------------------------
MovementState* FreeHangToBracedHang::UpdateTransition()
{
	if (m_nextMovementState != nullptr)
	{
		// get the difference between free_hang_to_braced_hang last root motion value & braced_hang first root motion value
		Vec3			freeToBracedHanglastXValue	= m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value;
		AnimationState* nextAnimationState			= AnimationState::GetAnimationStateByName( "hang" );
		Vec3			bracedHangFirstXValue		= nextAnimationState->GetFirstKeyframeRootMotionTranslation();
		Vec3			diff						= bracedHangFirstXValue - freeToBracedHanglastXValue;

		g_theCharacter->m_physics.m_position	   -= (g_theCharacter->m_physics.m_orientation * diff);
	}

	

	return m_nextMovementState;

}


//constexpr float BRACED_HANG_HEIGHT = 0.33186f;
//----------------------------------------------------------------------------------------------------------
void FreeHangToBracedHang::NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation )
{
	m_notifyStateChangeToAnimation = triggerTransitionRequestToAnimation;

	//g_theCharacter->m_physics.m_position.z = BRACED_HANG_HEIGHT;
	//g_theCharacter->m_physics.m_position.x = 0.f;

	


	m_nextMovementState						   = new HangingIdleState();
}
