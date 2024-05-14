#pragma once


#include "Game/Game.hpp"
#include "Game/Character.hpp"
#include "Game/ThirdPersonController.hpp"


#include "Engine/Math/AABB3.hpp"

class GameLedgeGrabTest : public Game
{
public:
	void Startup();
	void Shutdown();
	void Update();
	void Render() const;

	bool m_showDebugView = false;

	virtual void PrintDebugScreenMessage() override;	

	//----------------------------------------------------------------------------------------------------------
	Map*  m_map = nullptr;
	void CreateObstacle();
	void RenderObstacle() const;
};