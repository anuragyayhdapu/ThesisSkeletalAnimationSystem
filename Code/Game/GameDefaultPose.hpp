#pragma once

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Math/Vec2.hpp"


class App;
class Clock;
class Player;
class FbxFileImporter;

class GameDefaultPose : public Game
{
private:
	bool m_showDebugView = false;

public:
	void Startup();
	void Shutdown();
	void Update();
	void Render() const;

	AnimPose		 m_pose;
	FbxFileImporter* fbxFileImporter = nullptr;
};