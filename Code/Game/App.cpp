#include "Game/Game.hpp"
#include "Game/App.hpp"

#include "Engine/Core/JobSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/AABB2.hpp"


App* g_theApp = nullptr;			// Created and owned by Main_Windows.cpp
Renderer* g_theRenderer = nullptr;	// Created and owned by the App
InputSystem* g_theInput = nullptr;	// used by game code for input queries. App class should own (create, manage, destroy) a single instance of the InputSystem for your game
Window* g_theWindow = nullptr;

App::App()
{
}

App::~App()
{
}

void App::Startup()
{
	// create the event system
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	// create input
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem(inputSystemConfig);

	// create window
	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Skeletal Animation System";
	windowConfig.m_clientAspect = 2.0f;
	g_theWindow = new Window(windowConfig);

	// create renderer
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	// create dev console
	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	// create job system
	JobSystemConfig jobSystemConfig;
	g_theJobSystem = new JobSystem( jobSystemConfig );

	g_theJobSystem->Startup();
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	AddGameKeyText();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();

	// create and startup debug renderer
	DebugRenderConfig debugRendererConfig;
	debugRendererConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(debugRendererConfig);

	LoadGameConfig();
	LoadFonts();
	LoadTextures();
	LoadShaders();

	// subscribe to quit event
	g_theEventSystem->SubscribeToEvent(QUIT_COMMAND, App::EventHandler_CloseWindow);

	// start game
	g_theGame = Game::CreateNewGameOfType( STARTUP_GAME_MODE );
	g_theGame->Startup();
}

void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::Shutdown()
{
	// un-subscribe from quit event
	g_theEventSystem->UnsubscribeFromEvent(QUIT_COMMAND, App::EventHandler_CloseWindow);

	m_isQuitting = false;

	DebugRenderSystemShutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();
	g_theDevConsole->Shutdown();
	g_theJobSystem->Shutdown();

	if (g_theGame != nullptr)
	{
		g_theGame->Shutdown();
	}
	delete g_theGame;			g_theGame = nullptr;
	delete g_theRenderer;		g_theRenderer = nullptr;
	delete g_theWindow;			g_theWindow = nullptr;
	delete g_theInput;			g_theInput = nullptr;
	delete g_theEventSystem;	g_theEventSystem = nullptr;
	delete g_theDevConsole;		g_theDevConsole = nullptr;
}

void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return m_isQuitting;
}

void App::BeginFrame()
{
	Clock::TickSystemClock();

	g_theJobSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theDevConsole->BeginFrame();
	DebugRenderBeginFrame();

	if (g_theGame != nullptr)
		g_theGame->BeginFrame();
}

void App::Update()
{
	// update cursor state
	UpdateCursorState();

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		// quit app
		g_theGame->Shutdown();
		delete g_theGame;
		g_theGame = nullptr;

		HandleQuitRequested();

		return;
	}

	// Hard Reset game on (f8)
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		//bool isDebugViewOn = g_theGame->IsDubugViewOn();
		g_theGame->Shutdown();
		delete g_theGame;

		g_theGame = Game::CreateNewGameOfType( m_currentGameMode );
		g_theGame->Startup();

		return;
	}

	CycleBetweenGameModes();
	CycleBetweenDebugDrawStates();

	if (g_theGame != nullptr)
		g_theGame->Update();
}

void App::Render() const
{
	if (m_isQuitting)
		return;

	if (g_theRenderer == nullptr)
		return;

	if (g_theGame != nullptr)
	{
		g_theGame->Render();
	}

	// render dev console
	if (g_theDevConsole)
	{
		DevConsoleRenderConfig renderConfig;
		if (g_theGame != nullptr)
		{
			renderConfig.m_camera = &g_theGame->m_screenCamera;
		}

		g_theDevConsole->Render(renderConfig);
	}

	RenderTestMouse();
}


void App::RenderTestMouse() const
{
	/*Vec2 cursorClientPos = g_theInput->GetCursorClientPosition();
	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();


	float cursorScreenX = RangeMap(cursorClientPos.x, m_theGame->m_screenCamera.GetOrthographicBottomLeft().x, )*/
}


void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theDevConsole->EndFrame();
	DebugRenderEndFrame();
	g_theJobSystem->EndFrame();

	if (g_theGame != nullptr)
		g_theGame->EndFrame();
}


void App::LoadGameConfig()
{
	XmlDocument gameConfigXml;
	gameConfigXml.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = gameConfigXml.RootElement();
	if (rootElement != nullptr)
	{
		g_gameConfigGlackboard.PopulateFromXmlElementAttributes(*rootElement);
	}
}


void App::LoadFonts()
{
	// test font
	g_simpleBitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	// dev console fonts
	if (g_theDevConsole)
	{
		g_theDevConsole->LoadFonts();
	}
}

void App::LoadTextures()
{
}

void App::LoadShaders()
{
	g_theRenderer->CreateOrGetShaderByName("Data/Shaders/Default");
}


bool App::EventHandler_CloseWindow(EventArgs& eventArgs)
{
	UNUSED(eventArgs);

	if (g_theApp != nullptr)
	{
		g_theApp->HandleQuitRequested();
		return true;
	}

	return false;
}

void App::AddGameKeyText()
{
	if (g_theDevConsole)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_MAJOR_COLOR, "Major Controls");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "---------------");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- W/A/S/D   : XY Movement");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- Q / E     : Z Movement");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- Shift     : Sprint");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- O         : Reset Camera");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- ~         : Open Dev console");
		g_theDevConsole->AddLine(DevConsole::INFO_MAJOR_COLOR, "Other Controls");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "---------------");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- Esc       : Quit");
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR_COLOR, "- P         : Pause");
		g_theDevConsole->AddLine(DevConsole::INFO_MAJOR_COLOR, "Type help for a list of commands");
	}
}


void App::UpdateCursorState()
{
	bool cursorHidden = false;
	bool cursorRelative = false;
	bool isCurrentWindowFocused = g_theWindow->DoesCurrentWindowHaveFocus();

	if (isCurrentWindowFocused &&
		!g_theDevConsole->IsOpen())
	{
		cursorHidden = true;
		cursorRelative = true;
	}

	g_theInput->SetCursorMode(cursorHidden, cursorRelative);
}


//----------------------------------------------------------------------------------------------------------
void App::CycleBetweenGameModes()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_theGame->Shutdown();
		delete g_theGame;

		DebugRenderClear();

		m_currentGameMode = ( GameMode ) ( ( m_currentGameMode + 1 ) % NUM_GAME_MODES );

		g_theGame = Game::CreateNewGameOfType( m_currentGameMode );
		g_theGame->Startup();
	}
	else if ( g_theInput->WasKeyJustPressed( KEYCODE_F4 ) )
	{
		g_theGame->Shutdown();
		delete g_theGame;

		DebugRenderClear();

		if (m_currentGameMode == 0)
		{
			m_currentGameMode = (GameMode) (NUM_GAME_MODES - 1);
		}
		else
		{
			m_currentGameMode = ( GameMode ) ( ( m_currentGameMode - 1 ) % NUM_GAME_MODES );
		}

		g_theGame = Game::CreateNewGameOfType( m_currentGameMode );
		g_theGame->Startup();
	}
}


//----------------------------------------------------------------------------------------------------------
void App::CycleBetweenDebugDrawStates()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F9 ) )
	{
		g_theDebugDrawState = ( DebugDrawState ) ( ( g_theDebugDrawState + 1 ) % NUM_DEBUG_DRAW_STATE );
	}
}


//----------------------------------------------------------------------------------------------------------
bool App::CanDebugDraw3D() const
{
	if (g_theDebugDrawState == DEBUGDRAW_ALL || g_theDebugDrawState == DEBUGDRAW_3D)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------
bool App::CanDebugDraw2D() const
{
	if (g_theDebugDrawState == DEBUGDRAW_ALL || g_theDebugDrawState == DEBUGDRAW_2D)
	{
		return true;
	}
	else
	{
		return false;
	}
}
