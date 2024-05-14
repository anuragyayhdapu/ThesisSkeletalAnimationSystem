#pragma once

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Math/Vec2.hpp"


class App;
class Clock;
class Player;
class FbxFileImporter;

class GamePoseBlending : public Game
{
private:
	bool m_showDebugView = false;

public:
	void Startup();
	void Shutdown();
	void Update();
	void Render() const;

	AnimPose		 m_pose1;
	AnimPose		 m_pose2;
	FbxFileImporter* fbxFileImporter = nullptr;
	AnimPose		 m_blendedPose;
	float			 m_blendValue = 0.f;

	void UpdateBlendValue();
	void RenderPose( AnimPose const& pose, Rgba8 color ) const;
};