#pragma once


#include "Game/Game.hpp"
#include "Game/Character.hpp"
#include "Game/ThirdPersonController.hpp"


#include "Engine/Math/AABB3.hpp"

class GameVaultingTest : public Game
{
public:
	void Startup();
	void Shutdown();
	void Update();
	void Render() const;

	bool m_showDebugView = false;


	//----------------------------------------------------------------------------------------------------------
	/*ThirdPersonController* m_controller = nullptr;
	Character*			  m_character = nullptr;*/

	virtual void PrintDebugScreenMessage() override;	


	//----------------------------------------------------------------------------------------------------------
	AABB3 m_vaultObstacle;
	Map*  m_map = nullptr;
	void CreateObstacle();
	void RenderObstacle() const;
};