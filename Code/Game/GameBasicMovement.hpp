#pragma once

#include "Game/Game.hpp"
#include "Game/Character.hpp"
#include "Game/ThirdPersonController.hpp"


class GameBasicMovement : public Game
{
public:
	void Startup();
	void Shutdown();
	void Update();
	void Render() const;

	bool m_showDebugView = false;


	//----------------------------------------------------------------------------------------------------------
	/*ThirdPersonController* m_controller;
	Character			  m_character;*/

	virtual void PrintDebugScreenMessage() override;	
};