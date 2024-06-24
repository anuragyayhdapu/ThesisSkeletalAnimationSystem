#pragma once

#include "Engine/Core/JobSystem.hpp"
#include "Engine/Animation/AnimBlendTree.hpp"
#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Animation/AnimCurve.hpp"
#include "Engine/Core/XmlUtils.hpp"

#include <string>
#include <vector>
#include <map>

class AnimClip;


//----------------------------------------------------------------------------------------------------------
struct JobLoadAnimationClip : public Job
{
	int			   m_jobNum			  = 0;
	std::string m_stateName		   = "";
	AnimClip*	m_animClip		   = nullptr;
	std::string m_clipFileName	   = "";
	bool		m_removeRootMotion = false;
	AnimPose	m_defaultPose;
	AnimBlendTree* m_blendTree = nullptr;

	JobLoadAnimationClip( int jobNum, std::string stateName, std::string clipFileName, bool removeRootMotion )
		: m_jobNum(jobNum), m_stateName( stateName ), m_clipFileName( clipFileName ), m_removeRootMotion( removeRootMotion )
	{
		m_type = JobType::DISK_IO;
	}

	virtual ~JobLoadAnimationClip() {}

	virtual void Execute() override;
};


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

	bool							   m_isLoaded		  = false;
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
//protected:
	AnimBlendTree* m_blendTree = nullptr;
	void		   InitBlendTree();
	void		   UpdateBlendTree();
};