#include "Game/ParkourClimbStates.hpp"
#include "Game/ThirdPersonController.hpp"
#include "Game/Character.hpp"
#include "Game/AnimationController.hpp"
#include "Game/ParkourLedgeHangStates.hpp"
#include "Game/ParkourMovementStates.hpp"
#include "Game/MovementState.hpp"
#include "Game/AnimationState.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimClip.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------
MovementState::MovementState()
{
}

//----------------------------------------------------------------------------------------------------------
void MovementState::InitRootMotionTranslation()
{
	// set the root motion translation for next translation state
	AnimationState* currentAnimationState = g_theAnimationController->m_animationStack.top();
	std::string		movementStateName	  = GetStateName();
	if ( currentAnimationState->m_name == movementStateName )
	{
		AnimClip*		   animClip				= currentAnimationState->m_clip;
		AnimChannel const& rootJoint			= animClip->m_animChannels[ 0 ];
		Vec3AnimCurve	   rootJointTranslation = rootJoint.m_positionCurve;

		m_rootMotionTranslationCurve			= rootJointTranslation;
	}
	else
	{
		Transition* nextStateTransition = currentAnimationState->GetTransitionByName( movementStateName );
		if ( nextStateTransition )
		{
			std::string		   nextAnimationStateName = nextStateTransition->m_animationState;

			AnimationState*	   nextAnimationState	  = AnimationState::GetAnimationStateByName( nextAnimationStateName );
			AnimClip*		   animClip				  = nextAnimationState->m_clip;
			AnimChannel const& rootJoint			  = animClip->m_animChannels[ 0 ];
			Vec3AnimCurve	   rootJointTranslation	  = rootJoint.m_positionCurve;

			m_rootMotionTranslationCurve			  = rootJointTranslation;
		}
	}


	m_sampleTimeMs			   = 0.f;
	m_previousFrameTranslation = m_rootMotionTranslationCurve.Sample( m_sampleTimeMs, false );
	// m_previousFrameTranslation.z = 0.f;

	m_thisFrameTranslation = m_rootMotionTranslationCurve.Sample( m_sampleTimeMs, false );
	// m_thisFrameTranslation.z = 0.f;
}


//----------------------------------------------------------------------------------------------------------
void MovementState::UpdateRootMotionTranslation()
{
	// copy old
	m_previousFrameTranslation = m_thisFrameTranslation;

	if ( m_sampleTimeMs > m_rootMotionTranslationCurve.GetEndTimeMilliSeconds() )
	{
		// don't loop
		return;
	}

	if ( m_rootMotionTranslationCurve.IsEmpty() )
	{
		return;
	}

	// sample new
	Vec3 sampledTranslation			   = m_rootMotionTranslationCurve.Sample( m_sampleTimeMs, false );


	m_debugSampledUnrotatedtranslation = sampledTranslation;
	Quaternion rotor				   = g_theThirdPersonController->GetCharacterOrientation();
	sampledTranslation				   = rotor * sampledTranslation;

	// sampledTranslation.z = 0.f;
	// sampledTranslation.z *= 1.5f;

	m_thisFrameTranslation = sampledTranslation;

	// update delta time
	float deltaseconds	= g_theThirdPersonController->m_characterClock->GetDeltaSeconds();
	m_sampleTimeMs	   += ( deltaseconds * 1000.f );

	/*DebuggerPrintf( "//----------------------------------------------------------------------------------------------------------\n" );
	DebuggerPrintf( "Prev Frame Translation: %.3f, %.3f, %.3f\n", m_previousFrameTranslation.x, m_previousFrameTranslation.y, m_previousFrameTranslation.z );
	DebuggerPrintf( "This Frame Translation: %.3f, %.3f, %.3f\n", m_thisFrameTranslation.x, m_thisFrameTranslation.y, m_thisFrameTranslation.z );
	Vec3 deltaTranslation = GetRootMotionDeltaTranslation();
	DebuggerPrintf( "Delta      Translation: %.3f, %.3f, %.3f\n", deltaTranslation.x, deltaTranslation.y, deltaTranslation.z );*/

	if ( m_applyRootMotionTranslation )
	{
		// update character position and orientation
		// set character position
		Vec3 characterPos		  = g_theThirdPersonController->GetCharacterPosition();
		Vec3 newCharacterPosition = characterPos + GetRootMotionDeltaTranslation();
		g_theThirdPersonController->SetCharacterPosition( newCharacterPosition );

		// set camera position
		Vec3 cameraPos	  = g_theThirdPersonController->GetCameraPosition();
		Vec3 newCameraPos = cameraPos + GetRootMotionDeltaTranslation();
		g_theThirdPersonController->SetCameraPosition( newCameraPos );

		// DebuggerPrintf( "New Character Position: %.3f, %.3f, %.3f\n", newCharacterPosition.x, newCharacterPosition.y, newCharacterPosition.z );
	}
}


//----------------------------------------------------------------------------------------------------------
Vec3 MovementState::GetRootMotionDeltaTranslation() const
{
	Vec3 deltaTranslation = m_thisFrameTranslation - m_previousFrameTranslation;
	return deltaTranslation;
}


//----------------------------------------------------------------------------------------------------------
void MovementState::InitCameraPosition()
{
	m_initialCameraPos = g_theThirdPersonController->GetCameraPosition();

	// only consider x and z motion to update camera position
	Vec3 translationAtFirstKeyframe = m_rootMotionTranslationCurve.GetKeyframeAtFirstIndex().m_value;
	Vec3 translationAtLastKeyframe	= m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value;
	Vec3 totalTranslation			= translationAtLastKeyframe - translationAtFirstKeyframe;
	totalTranslation.y				= 0.f;
	totalTranslation				= g_theThirdPersonController->RotateTowardsCharacterOrientation( totalTranslation );
	m_finalCameraPos				= m_initialCameraPos + totalTranslation;
}


//----------------------------------------------------------------------------------------------------------
void MovementState::LerpCameraPosition()
{
	float localTimeMs		  = g_theAnimationController->GetLocalTimeMsOfCurrentAnimation();
	float parametricZeroToOne = localTimeMs / m_rootMotionTranslationCurve.GetEndTimeMilliSeconds();
	Vec3  lerpedPosition	  = Lerp( m_initialCameraPos, m_finalCameraPos, parametricZeroToOne );
	g_theThirdPersonController->SetCameraPosition( lerpedPosition );
}


//----------------------------------------------------------------------------------------------------------
IdleMovementState::IdleMovementState()
{
	AnimationState* state		 = AnimationState::GetAnimationStateByName( "idle" );
	m_rootMotionTranslationCurve = state->GetRootMotionTranslation();
	m_sampleTimeMs				 = 0.f;
	if ( !m_rootMotionTranslationCurve.IsEmpty() )
	{
		m_previousFrameTranslation = m_rootMotionTranslationCurve.GetKeyframeAtIndex( 0 ).m_value;
		m_thisFrameTranslation	   = m_previousFrameTranslation;
	}
}


//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
MovementState* IdleMovementState::UpdateTransition()
{
	if ( g_theInput->WasKeyJustPressed( 'W' ) )
	{
		return new WalkMovementState();
	}

	if ( g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) )
	{
		return new JumpMovementState();
	}

	if ( g_theInput->WasKeyJustPressed( 'C' ) )
	{
		// get the last forward translation of 'standToCrouch' root joint translation
		AnimationState* standToCrouchState				  = AnimationState::GetAnimationStateByName( "standToCrouch" );
		Vec3			lastTranslationOfPreviosAnimation = standToCrouchState->GetLastKeyframeRootMotionTranslation();
		float			previosAnimationLastXTranslation  = lastTranslationOfPreviosAnimation.x;

		// fix forward translation of 'crouch' to start at the end translation of 'standToCrouch'
		AnimationState* crouchIdleState				= AnimationState::GetAnimationStateByName( "crouchedIdle" );
		Vec3AnimCurve&	crouchedIdleRootMotionCurve = crouchIdleState->m_clip->GetRootJointTranslationCurveByRefernce();
		for ( unsigned int index = 0; index < crouchedIdleRootMotionCurve.GetSize(); index++ )
		{
			Vec3& translation = crouchedIdleRootMotionCurve.m_keyframes[ index ].m_value;
			translation.x	  = previosAnimationLastXTranslation;
		}

		return new CrouchMovementState();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
WalkMovementState::WalkMovementState()
{
	AnimationState* state		 = AnimationState::GetAnimationStateByName( "walk" );
	m_rootMotionTranslationCurve = state->GetRootMotionTranslation();
	m_sampleTimeMs				 = 0.f;
	if ( !m_rootMotionTranslationCurve.IsEmpty() )
	{
		m_previousFrameTranslation = m_rootMotionTranslationCurve.GetKeyframeAtIndex( 0 ).m_value;
		m_thisFrameTranslation	   = m_previousFrameTranslation;
	}
}


//----------------------------------------------------------------------------------------------------------
MovementState* WalkMovementState::UpdateTransition()
{
	// return to idle
	if ( !g_theInput->IsKeyDown( 'W' ) )
	{
		return new IdleMovementState();
	}

	// ledge grab
	bool inRangeOfLedgeGrab = g_theCharacter->IsHeadInRangeOfAnObstacle( FloatRange( 0.f, 0.6f ) );
	// bool isActionButtonDown = g_theInput->WasKeyJustPressed( KEYCODE_SHIFT );
	bool isActionButtonDown = true;
	bool canLedgeGrab		= inRangeOfLedgeGrab && isActionButtonDown;
	if ( canLedgeGrab )
	{
		return new IdleToLedgeGrabState();
	}

	// run
	if ( g_theInput->IsKeyDown( 'W' ) && g_theInput->WasKeyJustPressed( KEYCODE_SHIFT ) )
	{
		return new RunMovementState();
	}

	// jump
	if ( g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) )
	{
		return new JumpMovementState();
	}

	// edge slip
	MovementState* edgeSlipState = CheckTransitionToEdgeSlip();
	if ( edgeSlipState != nullptr )
	{
		// TODO: remove later, only for testing
		return new IdleDropToFreeHang();
		// return edgeSlipState;
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
MovementState* WalkMovementState::CheckTransitionToEdgeSlip()
{
	MovementState* edgeSlipState = nullptr;

	bool		   isNotOnGround = !g_theCharacter->IsCharacterOnTheGround();
	bool		   isNearAnEdge	 = !g_theCharacter->IsGroundRaycastHittingSomething();
	if ( isNotOnGround && isNearAnEdge )
	{
		edgeSlipState = new WalkingEdgeSlip();
	}

	return edgeSlipState;
}


//----------------------------------------------------------------------------------------------------------
RunMovementState::RunMovementState()
{
	AnimationState* state		 = AnimationState::GetAnimationStateByName( "run" );
	m_rootMotionTranslationCurve = state->GetRootMotionTranslation();
	m_sampleTimeMs				 = 0.f;
	if ( !m_rootMotionTranslationCurve.IsEmpty() )
	{
		m_previousFrameTranslation = m_rootMotionTranslationCurve.GetKeyframeAtIndex( 0 ).m_value;
		m_thisFrameTranslation	   = m_previousFrameTranslation;
	}
}


//----------------------------------------------------------------------------------------------------------
MovementState* RunMovementState::UpdateTransition()
{
	if ( !g_theInput->IsKeyDown( 'W' ) )
	{
		return new IdleMovementState();
	}

	if ( !g_theInput->IsKeyDown( KEYCODE_SHIFT ) && g_theInput->IsKeyDown( 'W' ) )
	{
		return new WalkMovementState();
	}

	if (g_theInput->WasKeyJustPressed('C'))
	{
		return new RunningSlide();
	}

	/*if ( g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) )
	{
		return new JumpMovementState();
	}*/

	// if near an obstacle big enough to vault
	Character* character	 = g_theCharacter;
	bool	   isFootBlocked = character->IsFootInRangeOfAnObstacle( FloatRange( 2.f, 3.f ) );
	bool	   isHeadBlocked = character->IsHeadBlockedByAnObstacle();
	bool	   canVault		 = !isHeadBlocked && isFootBlocked;
	if ( canVault )
	{
		return new VaultMovementState();
	}

	// if near a ledge, stop running
	bool isGroundRaycastHittinSomething = character->IsGroundRaycastHittingSomething();
	bool isOnGround						= character->IsCharacterOnTheGround();
	bool isNearALedge					= !isOnGround && !isGroundRaycastHittinSomething;
	if ( isNearALedge )
	{
		return new RunStop();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
RunStop::RunStop()
{
	AnimationState* runStopAnimationState = AnimationState::GetAnimationStateByName( "runStop" );
	m_rootMotionTranslationCurve		  = runStopAnimationState->GetRootMotionTranslation();

	RemoveZRootMotion();
	RemoveYRootMotion();

	NormalizeXRootMotion();

	// initialize initial character and camera position for sampling
	m_initialCharacterPos = g_theThirdPersonController->GetCharacterPosition();
	m_initialCameraPos	  = g_theThirdPersonController->GetCameraPosition();
}


//----------------------------------------------------------------------------------------------------------
MovementState* RunStop::UpdateTransition()
{
	SampleCurveAndSetCharacterPosition();
	SampleCurveAndSetCameraPosition();

	if ( g_theAnimationController->IsCurrentAnimationAtEndOfState() )
	{
		return new IdleMovementState();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
void RunStop::RemoveZRootMotion()
{
	for ( unsigned int index = 0; index < m_rootMotionTranslationCurve.GetSize(); index++ )
	{
		Vec3& value = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value;
		value.z		= 0.f;
	}
}


//----------------------------------------------------------------------------------------------------------
void RunStop::RemoveYRootMotion()
{
	for ( unsigned int index = 0; index < m_rootMotionTranslationCurve.GetSize(); index++ )
	{
		Vec3& value = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value;
		value.y		= 0.f;
	}
}


//----------------------------------------------------------------------------------------------------------
void RunStop::NormalizeXRootMotion()
{
	float minX;
	float maxX;
	m_rootMotionTranslationCurve.GetMaxAndMinXValuesFromCurve( minX, maxX );

	// 1.1 subtract the lowest value from entire curve
	// 1.2 divide all the points by the highest value
	for ( unsigned int index = 0; index < m_rootMotionTranslationCurve.GetSize(); index++ )
	{
		float& x  = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value.x;
		x		 -= minX;
		x		 /= maxX;
	}
}


//----------------------------------------------------------------------------------------------------------
void RunStop::SampleCurveAndSetCharacterPosition()
{
	float sampleTimeMs		  = g_theAnimationController->GetLocalTimeMsOfCurrentAnimation();
	Vec3  sampledTranslation  = m_rootMotionTranslationCurve.Sample( sampleTimeMs, false );
	sampledTranslation		  = g_theThirdPersonController->RotateTowardsCharacterOrientation( sampledTranslation );
	Vec3 newCharacterPosition = m_initialCharacterPos + sampledTranslation;

	g_theThirdPersonController->SetCharacterPosition( newCharacterPosition );

	// DebuggerPrintf( "sampledTranslation: %f,%f,%f\n", sampledTranslation.x, sampledTranslation.y, sampledTranslation.z );
}


//----------------------------------------------------------------------------------------------------------
void RunStop::SampleCurveAndSetCameraPosition()
{
	float sampleTimeMs		 = g_theAnimationController->GetLocalTimeMsOfCurrentAnimation();
	Vec3  sampledTranslation = m_rootMotionTranslationCurve.Sample( sampleTimeMs, false );
	sampledTranslation		 = g_theThirdPersonController->RotateTowardsCharacterOrientation( sampledTranslation );
	Vec3 newCameraPosition	 = m_initialCameraPos + sampledTranslation;

	g_theThirdPersonController->SetCameraPosition( newCameraPosition );
}


//----------------------------------------------------------------------------------------------------------
JumpMovementState::JumpMovementState()
{
	InitRootMotionTranslation();
}


//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
MovementState* JumpMovementState::UpdateTransition()
{
	AnimationState* currentAnimationState = g_theAnimationController->m_animationStack.top();
	if ( currentAnimationState->IsAtEndOfState() )
	{
		if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) && g_theInput->IsKeyDown( 'W' ) )
		{
			return new RunMovementState();
		}
		else if ( g_theInput->IsKeyDown( 'W' ) )
		{
			return new WalkMovementState();
		}
		else
		{
			return new IdleMovementState();
		}
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
CrouchMovementState::CrouchMovementState()
{
	// set the curve for debug viewing
	m_rootMotionTranslationCurve = AnimationState::GetAnimationStateByName( "crouchedIdle" )->m_clip->GetRootJointTranslationCurve();
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
MovementState* CrouchMovementState::UpdateTransition()
{
	if ( g_theInput->WasKeyJustPressed( 'C' ) )
	{
		// get the last forward translation of 'standToCrouch' root joint translation
		AnimationState* crouchIdle						  = AnimationState::GetAnimationStateByName( "crouchedIdle" );
		Vec3			lastTranslationOfPreviosAnimation = crouchIdle->GetLastKeyframeRootMotionTranslation();
		float			previosAnimationLastXTranslation  = lastTranslationOfPreviosAnimation.x;

		// fix forward translation of 'crouch' to start at the end translation of 'standToCrouch'
		AnimationState* crouchToStandState			= AnimationState::GetAnimationStateByName( "crouchToStand" );
		Vec3AnimCurve&	crouhToStandRootMotionCurve = crouchToStandState->m_clip->GetRootJointTranslationCurveByRefernce();
		float			crouchToStandFirstX			= crouchToStandState->GetFirstKeyframeRootMotionTranslation().x;
		float			crouchToStandLastX			= crouchToStandState->GetLastKeyframeRootMotionTranslation().x;
		for ( unsigned int index = 0; index < crouhToStandRootMotionCurve.GetSize(); index++ )
		{
			Vec3& translation = crouhToStandRootMotionCurve.m_keyframes[ index ].m_value;
			translation.x	  = RangeMap( translation.x, crouchToStandFirstX, crouchToStandLastX, previosAnimationLastXTranslation, 0.f );
		}

		return new IdleMovementState();
	}

	if ( g_theInput->WasKeyJustPressed( 'W' ) )
	{
		return new CrouchWalkMovementState();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
CrouchWalkMovementState::CrouchWalkMovementState()
{
	InitRootMotionTranslation();
}

//----------------------------------------------------------------------------------------------------------
MovementState* CrouchWalkMovementState::UpdateTransition()
{
	if ( g_theInput->WasKeyJustReleased( 'W' ) )
	{
		return new CrouchMovementState();
	}

	return nullptr;
}
