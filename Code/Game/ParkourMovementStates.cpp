#include "Game/AnimationController.hpp"
#include "Game/AnimationState.hpp"
#include "Game/ThirdPersonController.hpp"
#include "Game/Character.hpp"
#include "Game/ParkourMovementStates.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimCurve.hpp"
#include "Engine/Animation/AnimClip.hpp"
#include "Engine/Math/Easing.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"


constexpr float X_SPLIT_START_TIME_FRACTION = 0.3f;
constexpr float X_SPLIT_END_TIME_FRACTION	= 0.7f;


//----------------------------------------------------------------------------------------------------------
VaultMovementState::VaultMovementState()
{
	m_applyRootMotionTranslation = true;

	InitRootMotionTranslation();

	FixRootMotionInXForwardDirection();
	RemoveRootMotionInZUpDirection();
}


//----------------------------------------------------------------------------------------------------------
MovementState* VaultMovementState::UpdateTransition()
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
void VaultMovementState::UpdateRootMotionTranslation()
{
	MovementState::UpdateRootMotionTranslation();
}


//----------------------------------------------------------------------------------------------------------
void VaultMovementState::FixRootMotionInXForwardDirection()
{
	float		 totalTime			= m_rootMotionTranslationCurve.GetEndTimeMilliSeconds();
	float		 splitStartTime		= totalTime * X_SPLIT_START_TIME_FRACTION;
	float		 splitEndTime		= totalTime * X_SPLIT_END_TIME_FRACTION;
	unsigned int splitStartIndex	= GetKeyframeIndexAtSampleTime( m_rootMotionTranslationCurve.m_keyframes, splitStartTime );
	unsigned int splitEndIndex		= GetKeyframeIndexAtSampleTime( m_rootMotionTranslationCurve.m_keyframes, splitEndTime );
	float		 oldSplitStartValue = m_rootMotionTranslationCurve.m_keyframes[ splitStartIndex ].m_value.x;
	float		 oldSplitEndValue	= m_rootMotionTranslationCurve.m_keyframes[ splitEndIndex ].m_value.x;

	float		 newSplitStartValue = oldSplitStartValue;
	float		 newSplitEndValue	= 6.f; // TODO: get this value from the obstacle width

	// lerp the split
	for ( unsigned int index = splitStartIndex; index <= splitEndIndex; index++ )
	{
		float& x				   = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value.x;
		float  parametricZeroToOne = GetFractionWithinRange( x, oldSplitStartValue, oldSplitEndValue );
		// parametricZeroToOne		   = SmoothStep5( parametricZeroToOne );
		x = Lerp( newSplitStartValue, newSplitEndValue, parametricZeroToOne );
	}

	// pad end of the curve
	float padding = newSplitEndValue - oldSplitEndValue;
	for ( unsigned int index = splitEndIndex + 1; index < m_rootMotionTranslationCurve.GetSize(); index++ )
	{
		float& x  = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value.x;
		x		 += padding;
	}
}


//----------------------------------------------------------------------------------------------------------
void VaultMovementState::RemoveRootMotionInZUpDirection()
{
	m_previousFrameTranslation.z = 0.f;
	m_thisFrameTranslation.z	 = 0.f;

	for ( unsigned int index = 0; index < m_rootMotionTranslationCurve.GetSize(); index++ )
	{
		float& z = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value.z;
		z		 = 0.f;
	}

	// scale up root motion in z according to our measures.
	Vec3AnimCurve& animRootMotionCurve = AnimationState::GetAnimationStateByName( "vault" )->m_clip->GetRootJointTranslationCurveByRefernce();
	float		   minZ, maxZ;
	animRootMotionCurve.GetMaxAndMinZValuesFromCurve( minZ, maxZ );
	float normalizeFraction = maxZ - minZ;
	float scalingFraction = 0.7f;
	for ( unsigned int index = 0; index < animRootMotionCurve.GetSize(); index++ )
	{
		float& zvalue = animRootMotionCurve.m_keyframes[ index ].m_value.z;
		zvalue		  /= normalizeFraction;
		zvalue		  *= scalingFraction;
	}
}


//----------------------------------------------------------------------------------------------------------
RunningSlide::RunningSlide()
{
	m_applyRootMotionTranslation = true;

	InitRootMotionTranslation();

	// remove root motion in z for m_position
	for ( unsigned int index = 0; index < m_rootMotionTranslationCurve.GetSize(); index++ )
	{
		float& z = m_rootMotionTranslationCurve.m_keyframes[ index ].m_value.z;
		z		 = m_rootMotionTranslationCurve.m_keyframes[ 0 ].m_value.z;
	}

	
}


//----------------------------------------------------------------------------------------------------------
MovementState* RunningSlide::UpdateTransition()
{
	if (g_theAnimationController->IsCurrentAnimationAtEndOfState())
	{
		return new RunMovementState();
	}

	return nullptr;
}


//----------------------------------------------------------------------------------------------------------
void RunningSlide::UpdateRootMotionTranslation()
{
	MovementState::UpdateRootMotionTranslation();
}
