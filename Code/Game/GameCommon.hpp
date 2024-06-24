//-----------------------------------------------------------------------------------------------
// GameCommon.hpp
//
#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

constexpr int TOTAL_NUM_KEYS = 255;

constexpr float DEBUG_LINE_THICKNESS = 0.3f;
constexpr float DEBUG_RING_THICKNESS = 0.3f;


const Vec2 SCREEN_BOTTOM_LEFT_ORTHO( 0.f, 0.f );
const Vec2 SCREEN_TOP_RIGHT_ORTHO( 1600.f, 800.f );

const Vec2 WORLD_BOTTON_LEFT_ORTHO( -1.f, -1.f );
const Vec2 WORLD_TOP_RIGHT_ORTHO( 1.f, 1.f );

extern InputSystem* g_theInput;
extern Renderer*	g_theRenderer;

class Window;
extern Window* g_theWindow;

class BitmapFont;
extern BitmapFont* g_simpleBitmapFont;

class Game;
extern Game* g_theGame;

class App;
extern App* g_theApp;

extern char const* DEFAULT_MODEL;


enum GameMode
{
	GAMEMODE_DEFAULT_POSE,
	GAMEMODE_BASIC_ANIMATION_PLAYBACK,
	GAMEMODE_POSE_BLENDING,
	GAMEMODE_CROSSFADE_ANIMATION_BLENDING_ONE_FRAME,
	GAMEMODE_CROSSFADE_ANIMATION_BLENDING_SMOOTH_LINEAR_MANUAL,
	GAMEMODE_CROSSFADE_ANIMATION_BLENDING_SMOOTH_LINEAR_CONTROLLER,
	GAMEMODE_ADDITIVE_BLENDING,
	GAMEMODE_BASIC_3D_MOVEMENT,
	//GAMEMODE_QUATERNION_UNIT_TEST,
	GAMEMODE_VAULTING_TEST,
	GAMEMODE_LEDGE_GRAB_TEST,
	GAMEMODE_LOADMESH,
	GAMEMODE_SKINNING,
	GAMEMODE_FINAL_SHOWCASE,
	GAMEMODE_LOAD_FBX_THREAD,
	/*GAMEMODE_MOVEMENT_ACCERLERATION,
	GAMEMODE_STRIDE_WHEEL,*/

	NUM_GAME_MODES
};
extern GameMode STARTUP_GAME_MODE;


//----------------------------------------------------------------------------------------------------------
constexpr float CHARACTER_SPEED = 2.f;
constexpr float CHARACTER_TURN_SPEED = 100.f;



//----------------------------------------------------------------------------------------------------------
Vec3 const		  CONTROLLER_FOLLOW_POSITION	= Vec3( -3.5f, 0.f, 1.f );
EulerAngles const CONTROLLER_FOLLOW_ORIENTATION = EulerAngles( 0.f, 30.f, 0.f );


//----------------------------------------------------------------------------------------------------------
enum class MoveState
{
	Idle,
	Walking,
	Running,
	Jumping,
	Crouching,
};


//----------------------------------------------------------------------------------------------------------
class AnimationController;
extern AnimationController* g_theAnimationController;

class ThirdPersonController;
extern ThirdPersonController* g_theThirdPersonController;

class Character;
extern Character* g_theCharacter;


//----------------------------------------------------------------------------------------------------------
enum DebugDrawState
{
	DEBUGDRAW_ALL = 0,
	DEBUGDRAW_3D,
	DEBUGDRAW_2D,
	DEBUGDRAW_NONE,
	NUM_DEBUG_DRAW_STATE
};
extern DebugDrawState g_theDebugDrawState;