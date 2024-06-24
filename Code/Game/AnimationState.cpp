#include "Game/AnimationState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Animation/AnimClip.hpp"
#include "Engine/Animation/AnimBlendNode.hpp"
#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//----------------------------------------------------------------------------------------------------------
void JobLoadAnimationClip::Execute()
{
	g_theDevConsole->AddLine( Rgba8::RED, Stringf( "loading (%i) %s: %s", m_jobNum, m_stateName.c_str(), m_clipFileName.c_str() ) );

	m_animClip					   = AnimClip::LoadOrGetAnimationClip( m_clipFileName );
	m_animClip->m_removeRootMotion = m_removeRootMotion;

	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/XBot/TPose.fbx", m_defaultPose );

	m_blendTree				= new AnimBlendTree();
	AnimClipNode* clipNode	= new AnimClipNode( *m_animClip );
	clipNode->m_sampledPose = m_defaultPose;

	m_blendTree->m_rootNode = clipNode;
}


//----------------------------------------------------------------------------------------------------------
std::map<std::string, AnimationState*> AnimationState::s_animationStatesRegistery;


//----------------------------------------------------------------------------------------------------------
Transition::Transition( XmlElement const& xmlElement )
{
	m_name			 = ParseXmlAttribute( xmlElement, "name", m_name );
	m_animationState = ParseXmlAttribute( xmlElement, "animationState", m_animationState );
	m_fadeDuration	 = ParseXmlAttribute( xmlElement, "fadeDurationMs", m_fadeDuration );
}


//----------------------------------------------------------------------------------------------------------
AnimationState::AnimationState( XmlElement const& animStateElement )
{
	m_name								   = ParseXmlAttribute( animStateElement, "name", m_name );
	std::string			  clipFilePath	   = ParseXmlAttribute( animStateElement, "clip", "UNKOWN CLIP" );
	bool				  removeRootMotion = ParseXmlAttribute( animStateElement, "removeRootMotion", false );
	int					  stateNum		   = s_animationStatesRegistery.size() + 1;
	JobLoadAnimationClip* newJob		   = new JobLoadAnimationClip( stateNum, m_name, clipFilePath, removeRootMotion );
	g_theJobSystem->PostNewJob( newJob );
	//m_clip					   = AnimClip::LoadOrGetAnimationClip( clipFilePath );
	//m_clip->m_removeRootMotion = removeRootMotion;

	XmlElement const* transitionElementRoot = animStateElement.FirstChildElement( "Transitions" );
	if ( transitionElementRoot )
	{
		// transitions
		XmlElement const* transitionElement = transitionElementRoot->FirstChildElement( "Transition" );
		while ( transitionElement )
		{
			Transition* transition				= new Transition( *transitionElement );
			m_transitions[ transition->m_name ] = transition;

			transitionElement = transitionElement->NextSiblingElement( "Transition" );
		}

		// end transition
		XmlElement const* transitionEndElement = transitionElementRoot->FirstChildElement( "TransitionEnd" );
		if ( transitionEndElement )
		{
			m_endTransition			= new Transition( *transitionEndElement );
			m_endTransition->m_name = "Transition End";
		}

		// pop out transition
		XmlElement const* popOutTransitionElement = transitionElementRoot->FirstChildElement( "PopOutTransition" );
		if ( popOutTransitionElement )
		{
			m_popOutTransition		   = new Transition( *popOutTransitionElement );
			m_popOutTransition->m_name = "Pop Out Transition";
		}
	}

	/*InitSampledPose();
	InitBlendTree();*/
}


//----------------------------------------------------------------------------------------------------------
Transition* AnimationState::GetTransitionByName( std::string const& name ) const
{
	auto iter = m_transitions.find( name );
	if ( iter != m_transitions.cend() )
	{
		return iter->second;
	}
	else
	{
		DebuggerPrintf( "Error: Unable to find Transition by name: %s. No Transition from current state to this state.\n", name.c_str() );

		return nullptr;
	}
}


//----------------------------------------------------------------------------------------------------------
Vec3AnimCurve AnimationState::GetRootMotionTranslation() const
{
	Vec3AnimCurve rootMotionTranslationCurve;
	if ( m_clip )
	{
		rootMotionTranslationCurve = m_clip->GetRootJointTranslationCurve();
	}

	return rootMotionTranslationCurve;
}


//----------------------------------------------------------------------------------------------------------
Vec3 AnimationState::GetFirstKeyframeRootMotionTranslation() const
{
	Vec3AnimCurve const& rootMotionTranslationCurve = m_clip->GetRootJointTranslationCurve();
	Vec3				 firstRootMotionTranslation = rootMotionTranslationCurve.GetKeyframeAtFirstIndex().m_value;

	return firstRootMotionTranslation;
}


//----------------------------------------------------------------------------------------------------------
Vec3 AnimationState::GetLastKeyframeRootMotionTranslation() const
{
	Vec3AnimCurve const& rootMotionTranslationCurve = m_clip->GetRootJointTranslationCurve();
	Vec3				 lastRootMotionTranslation	= rootMotionTranslationCurve.GetKeyframeAtLastIndex().m_value;

	return lastRootMotionTranslation;
}


//----------------------------------------------------------------------------------------------------------
Vec3 AnimationState::GetLastAndFirstKeyframeRootMotionTranslationDelta() const
{
	Vec3 lastKeyframeRMT  = GetLastKeyframeRootMotionTranslation();
	Vec3 firstKeyframeRMT = GetFirstKeyframeRootMotionTranslation();
	Vec3 translationDelta = lastKeyframeRMT - firstKeyframeRMT;

	return translationDelta;
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::EndRootMotionZPosAtFollowingZValue( float newZValue )
{
	Vec3AnimCurve& rmtCurve	   = m_clip->GetRootJointTranslationCurveByRefernce();
	float		   lastZValue = rmtCurve.GetKeyframeAtLastIndex().m_value.z;
	float		   diff		   = lastZValue - newZValue;

	for ( unsigned int index = 0; index < rmtCurve.GetSize(); index++ )
	{
		Vec3&  rootBonePosInCurve = rmtCurve.m_keyframes[ index ].m_value;
		float& rootBoneZInCurve	  = rootBonePosInCurve.z;
		rootBoneZInCurve -= diff;
	}
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::RemoveRootMotionTranslationOfYPos()
{
	Vec3AnimCurve& rmtCurve = m_clip->GetRootJointTranslationCurveByRefernce();
	for ( unsigned int index = 0; index < rmtCurve.GetSize(); index++ )
	{
		Vec3&  rootBonePosInCurve = rmtCurve.m_keyframes[ index ].m_value;
		float& rootBoneYInCurve	  = rootBonePosInCurve.y;
		rootBoneYInCurve		  = 0.f;
	}
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::LoadAnimationStateFromXML( std::string const& xmlFilePath )
{
	XmlDocument animationDefinitionDocument;
	animationDefinitionDocument.LoadFile( xmlFilePath.c_str() );

	XmlElement* rootElement			= animationDefinitionDocument.RootElement();
	XmlElement* animationDefElement = rootElement->FirstChildElement( "AnimationState" );
	while ( animationDefElement )
	{
		AnimationState* state						= new AnimationState( *animationDefElement );
		s_animationStatesRegistery[ state->m_name ] = state;

		animationDefElement = animationDefElement->NextSiblingElement();
	}
}


//----------------------------------------------------------------------------------------------------------
AnimationState* AnimationState::GetAnimationStateByName( std::string const& name )
{
	std::map<std::string, AnimationState*>::const_iterator iter = s_animationStatesRegistery.find( name );
	if ( iter != s_animationStatesRegistery.cend() )
	{
		return iter->second;
	}
	else
	{
		DebuggerPrintf( "Error: Unable to find Animation by name: %s\n", name.c_str() );

		return nullptr;
	}
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::Update( bool loop )
{
	UNUSED( loop );

	// advance local time of the animation and sample
	float deltaSeconds	 = g_theAnimationController->m_animationClock->GetDeltaSeconds();
	float deltaSecondsMs = deltaSeconds * 1000.f;
	m_localTimeMs += deltaSecondsMs;

	if ( m_localTimeMs > m_clip->GetEndTime() )
	{
		// m_localTimeMs = m_clip->GetStartTime();
		m_localTimeMs = m_clip->GetEndTime();
		// m_localTimeMs = m_clip->GetEndTime() - 500;
	}

	/*if (loop)
	{
		if (m_localTimeMs > m_clip->GetEndTime())
		{
			m_localTimeMs = m_clip->GetStartTime();
		}
	}*/

	UpdateBlendTree();
}


//----------------------------------------------------------------------------------------------------------
bool AnimationState::IsAtEndOfState()
{
	if ( m_localTimeMs >= m_clip->GetEndTime() )
	{
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
bool AnimationState::HasEndTransition() const
{
	if ( m_endTransition )
	{
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::InitSampledPose()
{
	FbxFileImporter::LoadRestPoseFromFile( "Data/Animations/XBot/TPose.fbx", m_defaultPose );
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::InitBlendTree()
{
	m_blendTree				= new AnimBlendTree();
	AnimClipNode* clipNode	= new AnimClipNode( *m_clip );
	clipNode->m_sampledPose = m_defaultPose;

	m_blendTree->m_rootNode = clipNode;
}


//----------------------------------------------------------------------------------------------------------
void AnimationState::UpdateBlendTree()
{
	float clipStartTimeMs	  = m_clip->GetStartTime();
	float clipEndTimeMs		  = m_clip->GetEndTime();
	float parametricZeroToOne = m_localTimeMs / ( clipEndTimeMs - clipStartTimeMs );

	// DebuggerPrintf( "" );

	AnimBlendNode* clipNode = m_blendTree->m_rootNode;
	clipNode->Update( parametricZeroToOne );
}


//----------------------------------------------------------------------------------------------------------
AnimBlendTree* AnimationState::GetBlendTree() const
{
	return m_blendTree;
}
