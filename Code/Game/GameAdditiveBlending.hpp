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

class GameAdditiveBlending : public Game
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

	AnimClip* m_walkingClip = nullptr;
	AnimClip* m_crouchedWalkingClip	 = nullptr;
	AnimClip* m_runningClip		 = nullptr;


	AnimPose* m_walkingPose	= nullptr;
	AnimPose* m_crouchedWalking		= nullptr;
	AnimPose* m_differencePose		= nullptr;
	AnimPose* m_runningPose			= nullptr;
	AnimPose* m_crouchedRunningPose = nullptr;

	enum class AnimState
	{
		Animating,
		Blending,
	} m_animState = AnimState::Animating;

	void AdditiveBlend();
	void RenderPose( AnimPose const& pose, Vec2 offsetXY, Rgba8 color ) const;
};