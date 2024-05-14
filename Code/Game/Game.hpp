#pragma once

#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimPose.hpp"
#include "Engine/Math/Vec2.hpp"


class App;
class Clock;
class Player;
class FbxFileImporter;

class Game
{
private:
	bool m_showDebugView = false;

public:
	Game() {}
	virtual ~Game() {}
	virtual void Startup() {}
	virtual void Shutdown() {}

	void		 BeginFrame();
	virtual void Update()		= 0;
	virtual void Render() const = 0;
	void		 EndFrame();

	bool IsDubugViewOn();

	Camera m_screenCamera;

	Rgba8 m_backGroundColor = Rgba8( 38, 38, 38 );
	//Rgba8		 m_backGroundColor = Rgba8( 160, 160, 160 );

	static Game* CreateNewGameOfType( GameMode type );


	Clock* m_GameClock = nullptr;

	void	CreateScene();
	Player* m_player = nullptr;

	void RenderGridLines() const;

	void UpdateGameState();
	void UpdatePlayer();

	void AddBasisAtOrigin();
	virtual void PrintDebugScreenMessage();
	void PrintTitleText(std::string text);

	bool m_singleStepMode = false;

	//----------------------------------------------------------------------------------------------------------
	VertexBuffer* m_gridGpuVerts = nullptr;
	int			  m_numGridVerts	 = 0;

	void CreateGridVerts();
};