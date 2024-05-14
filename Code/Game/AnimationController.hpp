#pragma once

#include "Engine/Animation/AnimPose.hpp"

#include <stack>
#include <string>

class Clock;
class AnimPose;
class AnimationState;
class AnimBlendTree;
class AABB2;
class VertexBuffer;
struct Vec3AnimCurve;


//----------------------------------------------------------------------------------------------------------
class AnimationController
{
public:
	AnimationController();
	~AnimationController();

	void						Startup();
	void						Update();
	void						Render();
	void						Shutdown();

	Clock*						m_animationClock	  = nullptr;
	bool						m_transitionRequested = false;
	std::string					m_nextStateName		  = "UNKOWN STATE";
	std::stack<AnimationState*> m_animationStack	  = {};
	void						UpdateAnimationState();
	void						UpdateTransition();
	AnimPose const				GetSampledPose() const;
	bool						IsCurrentAnimationAtEndOfState() const;
	AnimationState*				GetCurrentAnimationState() const;
	float						GetLocalTimeMsOfCurrentAnimation() const;
	Vec3AnimCurve const&		GetCurrentStateRootMotionTranslation() const;
	Vec3AnimCurve&				GetCurrentStateRootMotionTranslationByReference();

	// debug drawing
	float		  m_debugCrossfadeBlendValue = 0.f;
	std::string	  m_debugFadeInStateName;
	float		  m_debugFadeInLocalTimeMs;
	float		  m_debugFadeInStartTimeMs;
	float		  m_debugFadeInEndTimeMs;
	std::string	  m_debugFadeOutStateName;
	float		  m_debugFadeOutLocalTimeMs;
	float		  m_debugFadeOutStartTimeMs;
	float		  m_debugFadeOutEndTimeMs;
	unsigned int  m_backgroundBoxStartDelimeter = 0;
	unsigned int  m_numbackgroundBoxVerts		= 0;
	VertexBuffer* m_debugUiGeometryBuffer		= nullptr;
	VertexBuffer* m_debugUiTextBuffer			= nullptr;
	void		  CreateStaticDebugUIVerts();
	void		  DebugRender();
	void		  DebugRenderAnimationStateUI( std::string const& animName, float localTimeMs, float startTimeMs, float endTimeMs, AABB2 const& parentBounds );
	void		  DebugRenderCrossfadeUI( AABB2 const& uiBounds );

	// cross fade
	AnimBlendTree*	m_crossfadeBlendTree  = nullptr;
	float			m_crossfadeDurationMs = 0.f;
	float			m_crossfadeTimeLeftMs = 0.f;
	AnimationState* m_crossfadeInState	  = nullptr;
	AnimationState* m_crossfadeOutState	  = nullptr;
	void			InitCrossfade( AnimationState* currentState, AnimationState* nextState, float fadeDurationMs );
	void			UpdateCrossfade();

	// blend tree
	AnimBlendTree* m_parentBlendTree = nullptr;
	void		   InitParentBlendTree();
	void		   UpdateParentBlendTree( AnimBlendTree* newBlendTree );
};