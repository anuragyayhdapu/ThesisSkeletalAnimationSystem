#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "App.hpp"

extern App* g_theApp;


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	g_theApp = new App();
	g_theApp->Startup();

	// May take 5 - 25 minutes to return... will "block" in an infinite loop internally until user quits!
	g_theApp->Run(); 

	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}