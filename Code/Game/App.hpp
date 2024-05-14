#pragma once

#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"


class Game;

class App
{
public:
	App();
	~App();

	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsQuitting() const { return m_isQuitting; }
	bool HandleQuitRequested();

protected:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

protected:
	bool m_isQuitting = false;

	void LoadGameConfig();
	void LoadFonts();
	void LoadTextures();
	void LoadShaders();
	static bool EventHandler_CloseWindow(EventArgs& eventArgs);
	void AddGameKeyText();
	void RenderTestMouse() const;
	void UpdateCursorState();

	GameMode m_currentGameMode = STARTUP_GAME_MODE;
	void CycleBetweenGameModes();

	void CycleBetweenDebugDrawStates();
public:
	bool CanDebugDraw3D() const;
	bool CanDebugDraw2D() const;
};

