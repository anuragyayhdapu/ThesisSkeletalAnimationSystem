#pragma once

#include "Engine/Animation/AnimBlendTree.hpp"
#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Animation/AnimCurve.hpp"
#include "Engine/Core/XmlUtils.hpp"

#include <string>
#include <vector>
#include <map>

class AnimClip;


//----------------------------------------------------------------------------------------------------------
struct Transition
{
	std::string m_name			 = "UNKOWN TRANSITION";
	std::string m_animationState = "UNKOWN ANIMATION STATE";
	float		m_fadeDuration	 = 0.f;

	Transition( XmlElement const& xmlElement );
};


//----------------------------------------------------------------------------------------------------------
class AnimationState
{
public:
	AnimationState( XmlElement const& xmlElement );

	std::string						   m_name			  = "";
	AnimClip*						   m_clip			  = nullptr;
	std::map<std::string, Transition*> m_transitions	  = {};
	Transition*						   m_endTransition	  = nullptr;
	Transition*						   m_popOutTransition = nullptr;
	Transition*						   GetTransitionByName( std::string const& name ) const;

	// root motion utils
	Vec3AnimCurve GetRootMotionTranslation() const;
	Vec3		  GetFirstKeyframeRootMotionTranslation() const;
	Vec3		  GetLastKeyframeRootMotionTranslation() const;
	Vec3		  GetLastAndFirstKeyframeRootMotionTranslationDelta() const;
	void		  EndRootMotionZPosAtFollowingZValue( float newZValue );
	void		  RemoveRootMotionTranslationOfYPos();

	// static animation registry
	static void									  LoadAnimationStateFromXML( std::string const& xmlFilePath );
	static std::map<std::string, AnimationState*> s_animationStatesRegistery;
	static AnimationState*						  GetAnimationStateByName( std::string const& name );

	// animation pose
	float	 m_localTimeMs = 0.f;
	AnimPose m_defaultPose;
	void	 InitSampledPose();
	void	 Update( bool loop = false );

	// state data
	bool IsAtEndOfState();
	bool HasEndTransition() const;

	// animation blend tree
	AnimBlendTree* GetBlendTree() const;
protected:
	AnimBlendTree* m_blendTree = nullptr;
	void		   InitBlendTree();
	void		   UpdateBlendTree();
};