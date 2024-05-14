#pragma once

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimCrossFadeController.hpp"
#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Math/Vec2.hpp"


class App;
class Clock;
class Player;
class FbxFileImporter;
class AnimClip;

class GameCrossfadeAnimationWithController : public Game
{
private:
	bool m_showDebugView = false;

public:
	void Startup();
	void Shutdown();
	void Update();
	void Render() const;

	float m_blendValue		   = 0.f;
	float m_blendTimeInSeconds = 0.f;

	float m_animationTimeInMilliSeconds = 0.f;

	AnimClip* m_animClipWalking = nullptr;
	AnimClip* m_animClipRunning = nullptr;
	AnimClip* m_animClipJumping = nullptr;
	AnimClip* m_animClipIdle	= nullptr;
	AnimPose* m_animPose		= nullptr;

	AnimCrossfadeController m_animCrossfadeController;


	enum class AnimState
	{
		Animating,
		Blending,
	} m_animState = AnimState::Animating;

	// void UpdateBlendValue();
	void RenderPose( AnimPose const& pose, Rgba8 color ) const;
};