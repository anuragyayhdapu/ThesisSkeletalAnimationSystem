#include "Game/GameCommon.hpp"


//----------------------------------------------------------------------------------------------------------
BitmapFont*			   g_simpleBitmapFont		  = nullptr; // Created by App in LoadFonts()
Game*				   g_theGame				  = nullptr; // Created and Owned by App
AnimationController*   g_theAnimationController	  = nullptr; // Created and owned by Game
ThirdPersonController* g_theThirdPersonController = nullptr; // Created ans owned by Game
Character*			   g_theCharacter			  = nullptr; // Created and owned by Game


//----------------------------------------------------------------------------------------------------------
GameMode	   STARTUP_GAME_MODE   = GAMEMODE_LOAD_FBX_THREAD;
DebugDrawState g_theDebugDrawState = DEBUGDRAW_NONE;