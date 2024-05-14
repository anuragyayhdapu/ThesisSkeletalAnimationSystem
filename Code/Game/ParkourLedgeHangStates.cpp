#include "Game/ParkourClimbStates.hpp"
#include "Game/AnimationController.hpp"
#include "Game/AnimationState.hpp"
#include "Game/ThirdPersonController.hpp"
#include "Game/ParkourLedgeHangStates.hpp"
#include "Game/Character.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimClip.hpp"
#include "Engine/Animation/AnimCurve.hpp"
#include "Engine/Math/Easing.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

constexpr float CAMERA_HEIGHT_OFFSET = 1.5f;

//----------------------------------------------------------------------------------------------------------
IdleToLedgeGrabState::IdleToLedgeGrabState()
{
	m_applyRootMotionTranslation = false;

	//InitRootMotionTranslation();

	AnimationState* currentAnimationState = AnimationState::GetAnimationStateByName( "idleToLedgeGrab" );
	Vec3AnimCurve&	rmtCurve			  = currentAnimationState->m_clip->GetRootJointTranslationCurveByRefernce();

	float			firstZValue			  = rmtCurve.GetKeyframeAtFirstIndex().m_value.z;
	float			lastZValue			  = rmtCurve.GetKeyframeAtLastIndex().m_value.z;
	float			obstacleHeight		  = g_theCharacter->m_latestObstacleHeight;

	for ( Vector3Keyframe& keyframe : rmtCurve.m_keyframes )
	{
		float& zValue	   = keyframe.m_value.z;
		float  normalizedZ = RangeMap( zValue, firstZValue, lastZValue, 0.f, 1.f );
		// zValue			   = normalizedZ * ( lastZValue + ( 0.75f ) );

		zValue = ( normalizedZ * ( obstacleHeight - firstZValue - 1.27f ) ) + firstZValue;
	}
	m_rootMotionTranslationCurve = rmtCurve;


	// init camera lerp start value
	m_cameraLerpStartValue = g_theThirdPersonController->GetCameraPosition();
	m_cameraLerpEndValue   = m_cameraLerpStartValue;
	m_cameraLerpEndValue.z = m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value.z + CAMERA_HEIGHT_OFFSET;

	
}


//----------------------------------------------------------------------------------------------------------
MovementState* IdleToLedgeGrabState::UpdateTransition()
{
	if (m_nextState == nullptr)
	{
		float totalAnimationTime = m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_timeMilliSeconds;
		float currentAnimationTime = g_theAnimationController->GetLocalTimeMsOfCurrentAnimation();
		float cameraLerpedZValue   = RangeMap( currentAnimationTime, 0.f, totalAnimationTime, m_cameraLerpStartValue.z, m_cameraLerpEndValue.z );
		Vec3  newCameraPos		   = m_cameraLerpStartValue;
		newCameraPos.z			   = cameraLerpedZValue;

		g_theThirdPersonController->SetCameraPosition( newCameraPos );
	}

	return m_nextState;
}


//----------------------------------------------------------------------------------------------------------
void IdleToLedgeGrabState::NotifyEndOfAnimationState( bool triggerTransitionRequestToAnimation )
{
	m_notifyStateChangeToAnimation = triggerTransitionRequestToAnimation;
	// g_theCharacter->m_physics.m_position = m_characterFinalPos;

	// fix the z height here
	Vector3Keyframe const& currentStateLastKeyFrame	 = m_rootMotionTranslationCurve.GetKeyframeAtLastIndex();
	float				   currentStateLastZValue	 = currentStateLastKeyFrame.m_value.z;

	m_nextState										 = new HangingIdleState();
	Vector3Keyframe const& nextStateFirstKeyFrame	 = m_nextState->m_rootMotionTranslationCurve.GetKeyframeAtFirstIndex();
	float				   nextStateFirstZValue		 = nextStateFirstKeyFrame.m_value.z;

	float				   zDifference				 = currentStateLastZValue - nextStateFirstZValue;
	g_theCharacter->m_physics.m_position			+= Vec3( 0.f, 0.f, zDifference );
}


//----------------------------------------------------------------------------------------------------------
HangingIdleState::HangingIdleState()
{
	m_applyRootMotionTranslation = false;

	AnimationState* state		 = AnimationState::GetAnimationStateByName( "hang" );
	m_rootMotionTranslationCurve = state->GetRootMotionTranslation();
	m_sampleTimeMs				 = 0.f;
	if ( !m_rootMotionTranslationCurve.IsEmpty() )
	{
		m_previousFrameTranslation = m_rootMotionTranslationCurve.GetKeyframeAtIndex( 0 ).m_value;
		m_thisFrameTranslation	   = m_previousFrameTranslation;
	}
}


//----------------------------------------------------------------------------------------------------------
MovementState* HangingIdleState::UpdateTransition()
{
	if ( g_theInput->WasKeyJustPressed( 'R' ) )
	{
		return new HangingIdleToDropState();
	}

	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		if ( g_theCharacter->IsRightShoulderRaycastHittinSomething() )
		{
			return new ShimmyRight();
		}
	}

	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		if ( g_theCharacter->IsLeftShoulderRaycastHittinSomething() )
		{
			return new ShimmyLeft();
		}
	}

	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		return new ClimbOver();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
HangingIdleToDropState::HangingIdleToDropState()
{
	m_applyRootMotionTranslation = false;

	m_characterStartZPos		 = g_theCharacter->m_physics.m_position.z;
	// g_theCharacter->m_physics.m_position.z = 0.f;

	InitRootMotionTranslation();

	// TODO remove hard-coded state name later
	AnimationState*		   previousState				   = AnimationState::GetAnimationStateByName( "hang" );
	AnimClip*			   previousStateClip			   = previousState->m_clip;
	Vec3AnimCurve const&   previosStateRootMotionCurve	   = previousStateClip->GetRootJointTranslationCurve();
	Vector3Keyframe const& previousStateLastKeyframe	   = previosStateRootMotionCurve.GetKeyframeAtLastIndex();
	float				   previousStateLastKeyframeZValue = previousStateLastKeyframe.m_value.z;

	/*AnimationState*		   nextState					= AnimationState::GetAnimationStateByName( "idle" );
	AnimClip*			   nextStateClip				= nextState->m_clip;
	Vec3AnimCurve const&   nextStateRootMotionCurve		= nextStateClip->GetRootJointTranslationCurve();
	Vector3Keyframe const& nextStateFirstKeyframe		= nextStateRootMotionCurve.GetKeyframeAtFirstIndex();
	float				   nextStateFirstKeyframeZValue = nextStateFirstKeyframe.m_value.z;*/

	AnimationState*				  currentAnimationState		  = AnimationState::GetAnimationStateByName( "hangToIdle" );
	AnimClip*					  currentAnimClip			  = currentAnimationState->m_clip;
	Vec3AnimCurve&				  currentRootJointTranslation = currentAnimClip->GetRootJointTranslationCurveByRefernce();
	std::vector<Vector3Keyframe>& currentKeyframes			  = currentRootJointTranslation.m_keyframes;

	float						  linearSplitStartParametric  = 0.f;
	float						  linearSplitEndParametric	  = 0.1f;
	float						  zValueAtLinearSplitStart	  = previousStateLastKeyframeZValue;
	float						  zValueAtLinearSpliEnd		  = zValueAtLinearSplitStart + 0.1f;

	float						  cubicBezierStartParametric  = 0.1f;
	float						  cubicBezierEndParametric	  = 0.5f;
	float						  zValueAtCubicSplitStart	  = zValueAtLinearSpliEnd;
	float						  timeMsAtCubicSegmentEnd	  = Lerp( 0.f, currentRootJointTranslation.GetEndTimeMilliSeconds(), cubicBezierEndParametric );
	float						  zValueAtCubicSegmentEnd	  = currentRootJointTranslation.Sample( timeMsAtCubicSegmentEnd, false ).z;
	float						  cubicBezierPoint1			  = 0.7f;
	float						  cubicBezierScaleFactor	  = zValueAtCubicSplitStart / cubicBezierPoint1;
	float						  maxJumpHeight = zValueAtCubicSplitStart += 0.3f;

	for ( int index = 0; index < currentKeyframes.size(); index++ )
	{
		Vector3Keyframe& keyframe			 = currentKeyframes[ index ];
		float&			 z					 = keyframe.m_value.z;

		float			 parametricZeroToOne = GetFractionWithinRange( keyframe.m_timeMilliSeconds, 0.f, currentRootJointTranslation.GetEndTimeMilliSeconds() );
		// linear segment
		if ( parametricZeroToOne >= linearSplitStartParametric && parametricZeroToOne <= linearSplitEndParametric )
		{
			float t		= GetFractionWithinRange( parametricZeroToOne, linearSplitStartParametric, linearSplitEndParametric );
			float value = ComputeCubicBezier1D( 0.f, 0.7f, 0.25f, 1.f, t );
			value		= RangeMap( value, 0.f, 1.f, zValueAtLinearSplitStart, zValueAtLinearSpliEnd );
			z			= value;
		}

		// cubic bezier segment
		if ( parametricZeroToOne >= cubicBezierStartParametric && parametricZeroToOne <= cubicBezierEndParametric )
		{
			float t			  = GetFractionWithinRange( parametricZeroToOne, cubicBezierStartParametric, cubicBezierEndParametric );
			float cubicValue  = ComputeCubicBezier1D( 0.7f, 1.6f, 0.f, 0.f, t );
			cubicValue		 *= cubicBezierScaleFactor;
			cubicValue		  = RangeMap( cubicValue, 0.f, cubicBezierScaleFactor, zValueAtCubicSegmentEnd, maxJumpHeight );
			z				  = cubicValue;
		}
	}

	m_rootMotionTranslationCurve = currentRootJointTranslation;

	m_cameraLerpStartValue		 = g_theThirdPersonController->GetCameraPosition();
	float lastZPos				  = m_rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value.z;
	m_cameraLerpEndValue		 = m_cameraLerpStartValue;
	m_cameraLerpEndValue.z		  = 1.38f + 1.35f ;
}


//----------------------------------------------------------------------------------------------------------
MovementState* HangingIdleToDropState::UpdateTransition()
{
	//----------------------------------------------------------------------------------------------------------
	float splitStartParametric = 0.35f;
	float splitEndParametric   = 0.59f;
	float parametricZeroToOne  = GetFractionWithinRange( m_sampleTimeMs, 0.f, m_rootMotionTranslationCurve.GetEndTimeMilliSeconds() );

	if ( parametricZeroToOne >= splitStartParametric && parametricZeroToOne <= splitEndParametric )
	{
		float  zEndValue	 = 0.f;
		float& zCurrentValue = g_theCharacter->m_physics.m_position.z;
		zCurrentValue		 = RangeMap( parametricZeroToOne, splitStartParametric, splitEndParametric, m_characterStartZPos, zEndValue );
	}

	DebuggerPrintf( "parametric zero to one: %.2f\n", parametricZeroToOne );

	// camera lerp
	float cameraLerpedZValue = RangeMap( parametricZeroToOne, 0.f, 1.f, m_cameraLerpStartValue.z, m_cameraLerpEndValue.z );
	Vec3  newCameraPos		 = m_cameraLerpStartValue;
	newCameraPos.z			 = cameraLerpedZValue;

	g_theThirdPersonController->SetCameraPosition( newCameraPos );


	//----------------------------------------------------------------------------------------------------------
	bool animationComplete = m_sampleTimeMs > m_rootMotionTranslationCurve.GetEndTimeMilliSeconds();
	if ( animationComplete )
	{
		return new IdleMovementState();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
ShimmyRight::ShimmyRight()
{
	m_applyRootMotionTranslation = false;
	InitRootMotionTranslation();

	// fix the shimmy right z height
	AnimationState* idleHangAnimationState		  = AnimationState::GetAnimationStateByName( "hang" );
	Vec3AnimCurve	idleHangRootMotionTranslation = idleHangAnimationState->GetRootMotionTranslation();
	Vec3			hangStartPos				  = idleHangRootMotionTranslation.GetKeyframeAtFirstIndex().m_value;
	float			hangStartZ					  = hangStartPos.z;

	AnimationState* shimmyRightAnimationState	  = AnimationState::GetAnimationStateByName( "shimmyRight" );
	Vec3AnimCurve&	shimmyRightRootMotionCurve	  = shimmyRightAnimationState->m_clip->GetRootJointTranslationCurveByRefernce();
	float			shimmyZMinValue;
	float			shimmyZMaxvalue;
	shimmyRightRootMotionCurve.GetMaxAndMinZValuesFromCurve( shimmyZMinValue, shimmyZMaxvalue );

	for ( unsigned int index = 0; index < shimmyRightRootMotionCurve.GetSize(); index++ )
	{
		Vec3&  shimmyPos = shimmyRightRootMotionCurve.m_keyframes[ index ].m_value;
		float& zValue	 = shimmyPos.z;

		// normalize, get in 0 to 1 range
		zValue = GetFractionWithinRange( zValue, shimmyZMinValue, shimmyZMaxvalue );

		// get in the range of idle hang z to idle hang z + 0.2f;
		zValue = Interpolate( hangStartZ, hangStartZ + 0.2f, zValue );
	}

	// fix shimmy right y values
	float sidewaysMovementLength = 1.f;
	float maxYValue				 = shimmyRightRootMotionCurve.GetKeyframeAtLastIndex().m_value.y;
	float scalingFraction		 = sidewaysMovementLength / fabsf( maxYValue );
	for ( unsigned int index = 0; index < shimmyRightRootMotionCurve.GetSize(); index++ )
	{
		Vec3&  shimmyPos  = shimmyRightRootMotionCurve.m_keyframes[ index ].m_value;
		float& yValue	  = shimmyPos.y;
		yValue			 *= scalingFraction;
	}

	// update the movement state root motion curve
	m_rootMotionTranslationCurve = shimmyRightRootMotionCurve;

	// set character & camera start and end y positions
	m_characterStartPos				= g_theCharacter->m_physics.m_position;
	m_characterEndPos				= g_theCharacter->m_physics.m_position + sidewaysMovementLength * ( g_theCharacter->m_physics.m_orientation * Vec3( 0.f, 1.f, 0.f ) );
	m_characterToCameraDisplacement = g_theThirdPersonController->GetCameraPosition() - g_theCharacter->m_physics.m_position;
}


//----------------------------------------------------------------------------------------------------------
MovementState* ShimmyRight::UpdateTransition()
{
	// while the root motion is updating, translate the player position as well
	AnimationState* shimmyRightAnimationState = AnimationState::GetAnimationStateByName( "shimmyRight" );
	float			rootBoneYValue			  = m_rootMotionTranslationCurve.Sample( shimmyRightAnimationState->m_localTimeMs, false ).y;
	Vec3&			charPos					  = g_theCharacter->m_physics.m_position;
	charPos									  = Lerp( m_characterStartPos, m_characterEndPos, rootBoneYValue );

	// update camera position
	if ( g_theThirdPersonController->m_controllerMode != ControllerMode::FreeRoam )
	{
		Vec3 newCameraPosition = g_theCharacter->m_physics.m_position + m_characterToCameraDisplacement;
		g_theThirdPersonController->SetCameraPosition( newCameraPosition );
	}


	// return to hanging idle state, when this animation completes
	AnimationState* currentAnimationState = g_theAnimationController->m_animationStack.top();
	if ( currentAnimationState->IsAtEndOfState() )
	{
		// go back to idle hang state
		return new HangingIdleState();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
ShimmyLeft::ShimmyLeft()
{
	m_applyRootMotionTranslation = false;
	InitRootMotionTranslation();

	// fix the shimmy right z height
	AnimationState* idleHangAnimationState		  = AnimationState::GetAnimationStateByName( "hang" );
	Vec3AnimCurve	idleHangRootMotionTranslation = idleHangAnimationState->GetRootMotionTranslation();
	Vec3			hangStartPos				  = idleHangRootMotionTranslation.GetKeyframeAtFirstIndex().m_value;
	float			hangStartZ					  = hangStartPos.z;

	AnimationState* shimmyRightAnimationState	  = AnimationState::GetAnimationStateByName( "shimmyLeft" );
	Vec3AnimCurve&	shimmyRightRootMotionCurve	  = shimmyRightAnimationState->m_clip->GetRootJointTranslationCurveByRefernce();
	float			shimmyZMinValue;
	float			shimmyZMaxvalue;
	shimmyRightRootMotionCurve.GetMaxAndMinZValuesFromCurve( shimmyZMinValue, shimmyZMaxvalue );

	for ( unsigned int index = 0; index < shimmyRightRootMotionCurve.GetSize(); index++ )
	{
		Vec3&  shimmyPos = shimmyRightRootMotionCurve.m_keyframes[ index ].m_value;
		float& zValue	 = shimmyPos.z;

		// normalize, get in 0 to 1 range
		zValue = GetFractionWithinRange( zValue, shimmyZMinValue, shimmyZMaxvalue );

		// get in the range of idle hang z to idle hang z + 0.2f;
		zValue = Interpolate( hangStartZ, hangStartZ + 0.2f, zValue );
	}

	// fix shimmy right y values
	float sidewaysMovementLength = 1.f;
	float maxYValue				 = shimmyRightRootMotionCurve.GetKeyframeAtLastIndex().m_value.y;
	float scalingFraction		 = sidewaysMovementLength / fabsf( maxYValue );
	for ( unsigned int index = 0; index < shimmyRightRootMotionCurve.GetSize(); index++ )
	{
		Vec3&  shimmyPos  = shimmyRightRootMotionCurve.m_keyframes[ index ].m_value;
		float& yValue	  = shimmyPos.y;
		yValue			 *= scalingFraction;
	}

	// update the movement state root motion curve
	m_rootMotionTranslationCurve = shimmyRightRootMotionCurve;

	// set character & camera start and end y positions
	m_characterStartPos				= g_theCharacter->m_physics.m_position;
	m_characterEndPos				= g_theCharacter->m_physics.m_position + sidewaysMovementLength * ( g_theCharacter->m_physics.m_orientation * Vec3( 0.f, 1.f, 0.f ) );
	m_characterToCameraDisplacement = g_theThirdPersonController->GetCameraPosition() - g_theCharacter->m_physics.m_position;
}


//----------------------------------------------------------------------------------------------------------
MovementState* ShimmyLeft::UpdateTransition()
{
	// while the root motion is updating, translate the player position as well
	AnimationState* shimmyRightAnimationState = AnimationState::GetAnimationStateByName( "shimmyLeft" );
	float			rootBoneYValue			  = m_rootMotionTranslationCurve.Sample( shimmyRightAnimationState->m_localTimeMs, false ).y;
	Vec3&			charPos					  = g_theCharacter->m_physics.m_position;
	charPos									  = Lerp( m_characterStartPos, m_characterEndPos, rootBoneYValue );

	// update camera position
	if ( g_theThirdPersonController->m_controllerMode != ControllerMode::FreeRoam )
	{
		Vec3 newCameraPosition = g_theCharacter->m_physics.m_position + m_characterToCameraDisplacement;
		g_theThirdPersonController->SetCameraPosition( newCameraPosition );
	}


	// return to hanging idle state, when this animation completes
	AnimationState* currentAnimationState = g_theAnimationController->m_animationStack.top();
	if ( currentAnimationState->IsAtEndOfState() )
	{
		// go back to idle hang state
		return new HangingIdleState();
	}

	return nullptr;
}
