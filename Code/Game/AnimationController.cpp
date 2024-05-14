#include "Game/App.hpp"
#include "Game/Character.hpp"
#include "Game/MovementState.hpp"
#include "Game/AnimationState.hpp"
#include "Game/AnimationController.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Animation/AnimBlendTree.hpp"
#include "Engine/Animation/AnimBlendNode.hpp"
#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Animation/AnimClip.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


#include <cmath>


//----------------------------------------------------------------------------------------------------------
AnimationController::AnimationController()
{
}


//----------------------------------------------------------------------------------------------------------
AnimationController::~AnimationController()
{
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::Startup()
{
	Clock* gameClock					= g_theGame->m_GameClock;
	m_animationClock					= new Clock( *gameClock );

	AnimationState* firstAnimationState = AnimationState::GetAnimationStateByName( "idle" );
	m_animationStack.push( firstAnimationState );

	InitParentBlendTree();
	CreateStaticDebugUIVerts();
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::Update()
{
	if ( g_theInput->WasKeyJustPressed( 'L' ) )
	{
		m_animationClock->TogglePause();
	}

	UpdateTransition();
	UpdateAnimationState();
	UpdateCrossfade();
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::Render()
{
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::Shutdown()
{
	delete m_animationClock;

	delete m_debugUiTextBuffer;
	delete m_debugUiGeometryBuffer;
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::UpdateTransition()
{
	// transition to another state
	if ( m_transitionRequested )
	{
		AnimationState* currentState = m_animationStack.top();
		Transition*		transition	 = currentState->GetTransitionByName( m_nextStateName );
		if ( transition )
		{
			// go to next animation
			AnimationState* nextAnimationState = AnimationState::GetAnimationStateByName( transition->m_animationState );
			nextAnimationState->m_localTimeMs  = 0.f;
			m_animationStack.pop();
			m_animationStack.push( nextAnimationState );

			// start crossfade
			InitCrossfade( currentState, nextAnimationState, transition->m_fadeDuration );

			// reset transition request
			m_transitionRequested = false;
			m_nextStateName		  = "UNKOWN STATE";
		}
		else
		{
			DebuggerPrintf( "Error: Unable to find transition from %s state to %s\n", currentState->m_name.c_str(), m_nextStateName.c_str() );

			m_transitionRequested = false;
			m_nextStateName		  = "UNKOWN STATE";
		}
	}

	// transition to end state
	AnimationState* currentState = m_animationStack.top();
	if ( currentState->IsAtEndOfState() )
	{
		if ( currentState->HasEndTransition() )
		{
			AnimationState* nextAnimationState = AnimationState::GetAnimationStateByName( currentState->m_endTransition->m_animationState );
			nextAnimationState->m_localTimeMs  = 0.f;

			// end transition present - swap with current animation
			m_animationStack.pop();
			m_animationStack.push( nextAnimationState );

			// InitCrossfade( currentState, nextAnimationState, currentState->m_endTransition->m_fadeDuration );

			if ( !m_crossfadeBlendTree )
				UpdateParentBlendTree( m_animationStack.top()->GetBlendTree() );
		}

		// notify the movement state that the animation has ended
		g_theCharacter->m_movementState->NotifyEndOfAnimationState( false );
	}


	// crossfade animations transition
	if ( m_crossfadeBlendTree )
	{
		// fade out state
		if ( m_crossfadeOutState->IsAtEndOfState() )
		{
			if ( m_crossfadeOutState->HasEndTransition() )
			{
				m_crossfadeOutState				   = AnimationState::GetAnimationStateByName( m_crossfadeOutState->m_endTransition->m_animationState );
				m_crossfadeOutState->m_localTimeMs = 0.f;
			}
		}

		// fade in state
		if ( m_crossfadeInState->IsAtEndOfState() )
		{
			if ( m_crossfadeInState->HasEndTransition() )
			{
				m_crossfadeInState				  = AnimationState::GetAnimationStateByName( m_crossfadeInState->m_endTransition->m_animationState );
				m_crossfadeInState->m_localTimeMs = 0.f;
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::UpdateAnimationState()
{
	if ( !m_crossfadeBlendTree ) // not cross fading
	{
		// update current animation
		AnimationState* currentState = m_animationStack.top();
		currentState->Update();
	}
	else // cross fading
	{
		m_crossfadeInState->Update( true );
		m_crossfadeOutState->Update( true );
	}
}


//----------------------------------------------------------------------------------------------------------
bool AnimationController::IsCurrentAnimationAtEndOfState() const
{
	AnimationState* currentAnimationState = m_animationStack.top();
	if ( currentAnimationState->IsAtEndOfState() )
	{
		return true;
	}
	else
	{
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------
AnimationState* AnimationController::GetCurrentAnimationState() const
{
	return m_animationStack.top();
}


//----------------------------------------------------------------------------------------------------------
float AnimationController::GetLocalTimeMsOfCurrentAnimation() const
{
	return m_animationStack.top()->m_localTimeMs;
}


//----------------------------------------------------------------------------------------------------------
Vec3AnimCurve const& AnimationController::GetCurrentStateRootMotionTranslation() const
{
	AnimationState* currentState = m_animationStack.top();
	return currentState->m_clip->GetRootJointTranslationCurve();
}


//----------------------------------------------------------------------------------------------------------
Vec3AnimCurve& AnimationController::GetCurrentStateRootMotionTranslationByReference()
{
	AnimationState* currentState = m_animationStack.top();
	return currentState->m_clip->GetRootJointTranslationCurveByRefernce();
}


//----------------------------------------------------------------------------------------------------------
AnimPose const AnimationController::GetSampledPose() const
{
	AnimPose pose = m_parentBlendTree->Evaluate();
	return pose;
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::CreateStaticDebugUIVerts()
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> geometryVerts;


	// animation UI
	AABB2 screenBounds	= g_theGame->m_screenCamera.GetOrthographicBounds();
	AABB2 animUiBounds	= screenBounds.GetBoxAtUVs( AABB2( 0.f, 0.6f, 0.2f, 0.7f ) );
	AABB2 ui_data		= animUiBounds.GetBoxAtUVs( AABB2( 0.f, 0.f, 1.f, 0.5f ) );
	AABB2 ui_time_title = ui_data.GetBoxAtUVs( AABB2( 0.f, 0.f, 0.3f, 1.f ) );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, ui_time_title, 20.f, "Time Ms" );

	AABB2 ui_anim_bar_parent = ui_data.GetBoxAtUVs( AABB2( 0.3f, 0.f, 1.f, 1.f ) );
	AABB2 ui_anim_bar		 = ui_anim_bar_parent.GetBoxAtUVs( AABB2( 0.f, 0.3f, 1.f, 1.f ) );
	AABB2 ui_bar			 = ui_anim_bar.GetBoxAtUVs( AABB2( 0.1f, 0.1f, 0.9f, 0.9f ) );

	AddVertsForAABB2( geometryVerts, animUiBounds, Rgba8::BLACK );
	AABB2 animCrossfadeUiBounds = screenBounds.GetBoxAtUVs( AABB2( 0.f, 0.1f, 0.3f, 0.5f ) );
	AddVertsForAABB2( geometryVerts, animCrossfadeUiBounds, Rgba8::BLACK );

	// current animation stack
	float fontSize = 15.f;
	float textMinX = screenBounds.m_mins.x;
	float textMinY = screenBounds.m_maxs.y * 0.83f;
	Vec2  topLeftLinePosition( textMinX, textMinY );
	//

	g_simpleBitmapFont->AddVertsForText2D( textVerts, topLeftLinePosition, fontSize, "Animation State", Rgba8::WHITE );
	// DebugAddScreenText( "Animation State", topLeftLinePosition, fontSize, topLeftAlignment, duration, Rgba8::WHITE );


	// fade in & out text

	AABB2 ui_transitionData = animCrossfadeUiBounds.GetBoxAtUVs( 0.f, 0.5f, 1.f, 1.f );
	AABB2 uiFadeIn			= ui_transitionData.GetBoxAtUVs( 0.f, 0.5f, 1.f, 1.f );
	AABB2 uiFadeInText		= uiFadeIn.GetBoxAtUVs( 0.f, 0.f, 0.3f, 1.f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiFadeInText, 20.f, "Fade In" );

	AABB2 uiFadeOut		= ui_transitionData.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );
	AABB2 uiFadeOutText = uiFadeOut.GetBoxAtUVs( 0.f, 0.f, 0.3f, 1.f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiFadeOutText, 20.f, "Fade Out" );


	// graph
	AABB2 ui_graph = animCrossfadeUiBounds.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );

	// t
	AABB2 uiTData = ui_graph.GetBoxAtUVs( 0.f, 0.f, 0.1f, 1.f );
	AABB2 uiTText = uiTData.GetBoxAtUVs( 0.f, 0.5f, 1.f, 1.f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiTText, 20.f, "t" );

	// fade ms
	AABB2 uiFadeMs = ui_graph.GetBoxAtUVs( 0.1f, 0.f, 1.f, 0.1f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiFadeMs, 20.f, "fade Ms" );

	// y
	AABB2 uiEasingFunc = ui_graph.GetBoxAtUVs( 0.1f, 0.1f, 0.9f, 0.9f );
	Vec2  yStart	   = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.f ) );
	Vec2  yEnd		   = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 1.f ) );
	AddVertsForLineSegment2D( geometryVerts, yStart, yEnd, 5.f, Rgba8::WHITE );
	AABB2 uiYAxisMax = uiEasingFunc.GetBoxAtUVs( 0.f, 0.9f, 0.1f, 1.f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiYAxisMax, 20.f, "1" );

	// x
	Vec2 xStart = uiEasingFunc.GetPointAtUV( Vec2( 0.02f, 0.2f ) );
	Vec2 xEnd	= uiEasingFunc.GetPointAtUV( Vec2( 1.f, 0.2f ) );
	AddVertsForLineSegment2D( geometryVerts, xStart, xEnd, 5.f, Rgba8::WHITE );
	AABB2 uiXAxisMin = uiEasingFunc.GetBoxAtUVs( 0.f, 0.f, 0.1f, 0.1f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiXAxisMin, 20.f, "0" );


	AddVertsForWireframeAABB2D( geometryVerts, ui_bar, 2.f, Rgba8::WHITE );


	// create gpu verts
	m_debugUiTextBuffer				   = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	m_debugUiTextBuffer->m_vertexCount = ( unsigned int ) textVerts.size();
	size_t textDataSize				   = textVerts.size() * sizeof( Vertex_PCU );
	g_theRenderer->CopyCPUToGPU( textVerts.data(), textDataSize, m_debugUiTextBuffer );

	m_debugUiGeometryBuffer				   = g_theRenderer->CreateAndGetVertexBuffer( sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	m_debugUiGeometryBuffer->m_vertexCount = ( unsigned int ) geometryVerts.size();
	size_t geometryDataSize				   = geometryVerts.size() * sizeof( Vertex_PCU );
	g_theRenderer->CopyCPUToGPU( geometryVerts.data(), geometryDataSize, m_debugUiGeometryBuffer );
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::DebugRender()
{
	if ( !g_theApp->CanDebugDraw2D() )
	{
		return;
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexBuffer( m_debugUiGeometryBuffer, m_debugUiGeometryBuffer->m_vertexCount );


	// current playback local time
	Camera const&				screenCamera = g_theGame->m_screenCamera;
	float						fontSize	 = 15.f;
	AABB2						cameraBounds( screenCamera.GetOrthographicBottomLeft(), screenCamera.GetOrthographicTopRight() );
	float						textMinX = cameraBounds.m_mins.x;
	float						textMinY = cameraBounds.m_maxs.y * 0.85f;
	Vec2						topLeftLinePosition( textMinX, textMinY );
	float						duration		 = 0.f;
	Vec2						topLeftAlignment = Vec2( 0.f, 1.f );

	std::stack<AnimationState*> animStackCopy	 = m_animationStack;
	while ( !animStackCopy.empty() )
	{
		AnimationState* animState = animStackCopy.top();
		animStackCopy.pop();

		topLeftLinePosition.y -= fontSize - 2.f;

		DebugAddScreenText( animState->m_name, topLeftLinePosition, fontSize, topLeftAlignment, duration, Rgba8::RED );
	}


	// animation UI
	AABB2			screenBounds   = g_theGame->m_screenCamera.GetOrthographicBounds();
	AABB2			animUiBounds   = screenBounds.GetBoxAtUVs( AABB2( 0.f, 0.6f, 0.2f, 0.7f ) );
	AnimationState* debugAnimState = m_animationStack.top();
	DebugRenderAnimationStateUI( debugAnimState->m_name, debugAnimState->m_localTimeMs, debugAnimState->m_clip->GetStartTime(),
		debugAnimState->m_clip->GetEndTime(), animUiBounds );

	// crossfade UI
	AABB2 animCrossfadeUiBounds = screenBounds.GetBoxAtUVs( AABB2( 0.f, 0.1f, 0.3f, 0.5f ) );
	DebugRenderCrossfadeUI( animCrossfadeUiBounds );

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( &( g_simpleBitmapFont->GetTexture() ) );
	g_theRenderer->DrawVertexBuffer( m_debugUiTextBuffer, m_debugUiTextBuffer->m_vertexCount );
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::InitCrossfade( AnimationState* currentState, AnimationState* nextState, float fadeDurationMs )
{
	if ( m_crossfadeBlendTree )
	{
		delete m_crossfadeBlendTree;
		m_crossfadeBlendTree = nullptr;
	}

	m_crossfadeDurationMs		   = fadeDurationMs;
	m_crossfadeTimeLeftMs		   = fadeDurationMs;

	m_crossfadeOutState			   = currentState;
	m_crossfadeInState			   = nextState;
	AnimBlendNode* fadeOutClipNode = m_crossfadeOutState->GetBlendTree()->m_rootNode;
	// fadeOutClipNode->m_debug	   = true;
	AnimBlendNode* fadeInClipNode = m_crossfadeInState->GetBlendTree()->m_rootNode;
	// fadeInClipNode->m_debug		   = true;
	BinaryLerpBlendNode* lerpNode	 = new BinaryLerpBlendNode();
	lerpNode->m_blendedPose			 = currentState->m_defaultPose;
	lerpNode->m_childNodeA			 = fadeOutClipNode;
	lerpNode->m_childNodeB			 = fadeInClipNode;

	m_crossfadeBlendTree			 = new AnimBlendTree();
	m_crossfadeBlendTree->m_rootNode = lerpNode;

	// parent tree evaluates cross fade
	UpdateParentBlendTree( m_crossfadeBlendTree );
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::UpdateCrossfade()
{
	if ( m_crossfadeBlendTree )
	{
		float deltaSeconds	   = m_animationClock->GetDeltaSeconds();
		float deltaMs		   = deltaSeconds * 1000.f;

		m_crossfadeTimeLeftMs -= deltaMs;
		if ( m_crossfadeTimeLeftMs < 0.f ) // cross fade complete
		{
			delete m_crossfadeBlendTree;
			m_crossfadeBlendTree = nullptr;

			// point parent blend tree back to current state
			AnimationState* currentState = m_animationStack.top();
			UpdateParentBlendTree( currentState->GetBlendTree() );

			return;
		}

		// cross fade in progress
		float blendValue = RangeMap( m_crossfadeTimeLeftMs, m_crossfadeDurationMs, 0.f, 0.f, 1.f );
		if ( isnan( blendValue ) )
		{
			DebuggerPrintf( "ye kaise hua ???" );
		}

		m_debugCrossfadeBlendValue		   = blendValue;
		AnimBlendNode* binaryLerpBlendNode = m_crossfadeBlendTree->m_rootNode;
		binaryLerpBlendNode->Update( blendValue );
	}
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::InitParentBlendTree()
{
	m_parentBlendTree = new AnimBlendTree();

	// parent tree evaluates only one state
	AnimationState* currentState  = m_animationStack.top();
	m_parentBlendTree->m_rootNode = currentState->GetBlendTree()->m_rootNode;
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::UpdateParentBlendTree( AnimBlendTree* newBlendTree )
{
	m_parentBlendTree->m_rootNode = newBlendTree->m_rootNode;
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::DebugRenderAnimationStateUI( std::string const& animName, float localTimeMs, float startTimeMs, float endTimeMs,
	AABB2 const& ui_bounds )
{
	std::vector<Vertex_PCU> textVerts;

	// AABB2 screenBounds = g_theGame->m_screenCamera.GetOrthographicBounds();
	//  AABB2 ui_background = screenBounds.GetBoxAtUVs( AABB2(0.f, 0.f, 0.2f, ) );
	// AABB2 ui_bounds = parentBounds.GetBoxAtUVs( AABB2( 0.f, 0.6f, 0.2f, 0.7f ) );

	AABB2 ui_title = ui_bounds.GetBoxAtUVs( AABB2( 0.f, 0.5f, 1.f, 1.f ) );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, ui_title, 20.f, animName, Rgba8::RED );


	AABB2 ui_data			 = ui_bounds.GetBoxAtUVs( AABB2( 0.f, 0.f, 1.f, 0.5f ) );
	AABB2 ui_anim_bar_parent = ui_data.GetBoxAtUVs( AABB2( 0.3f, 0.f, 1.f, 1.f ) );
	AABB2 ui_anim_bar		 = ui_anim_bar_parent.GetBoxAtUVs( AABB2( 0.f, 0.3f, 1.f, 1.f ) );
	AABB2 ui_bar			 = ui_anim_bar.GetBoxAtUVs( AABB2( 0.1f, 0.1f, 0.9f, 0.9f ) );

	float fill				 = ( localTimeMs / ( endTimeMs - startTimeMs ) );
	AABB2 ui_filledBar		 = ui_bar.GetBoxAtUVs( AABB2( 0.f, 0.f, fill, 1.f ) );

	AABB2 ui_start_time		 = ui_anim_bar_parent.GetBoxAtUVs( AABB2( 0.f, 0.f, 0.2f, 0.3f ) );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, ui_start_time, 20.f, Stringf( "%.f", startTimeMs ) );

	AABB2 ui_currentTime = ui_anim_bar_parent.GetBoxAtUVs( AABB2( 0.4f, 0.f, 0.6f, 0.3f ) );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, ui_currentTime, 20.f, Stringf( "%.f", localTimeMs ) );

	AABB2 ui_endTime = ui_anim_bar_parent.GetBoxAtUVs( AABB2( 0.8f, 0.f, 1.f, 0.3f ) );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, ui_endTime, 20.f, Stringf( "%.f", endTimeMs ) );

	std::vector<Vertex_PCU> verts;
	AddVertsForWireframeAABB2D( verts, ui_bar, 2.f, Rgba8::WHITE );
	AddVertsForAABB2( verts, ui_filledBar, Rgba8::RED );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( &g_simpleBitmapFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}


//----------------------------------------------------------------------------------------------------------
void AnimationController::DebugRenderCrossfadeUI( AABB2 const& uiBounds )
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );
	verts.clear();


	// fade in & out data
	AABB2 ui_transitionData = uiBounds.GetBoxAtUVs( 0.f, 0.5f, 1.f, 1.f );
	AABB2 uiFadeIn			= ui_transitionData.GetBoxAtUVs( 0.f, 0.5f, 1.f, 1.f );
	AABB2 uiFadeOut			= ui_transitionData.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );
	AABB2 uiFadeInData		= uiFadeIn.GetBoxAtUVs( 0.3f, 0.f, 1.f, 1.f );
	AABB2 uiFadeOutdata		= uiFadeOut.GetBoxAtUVs( 0.3f, 0.f, 1.f, 1.f );
	if ( m_crossfadeBlendTree )
	{
		m_debugFadeInStateName	  = m_crossfadeInState->m_name;
		m_debugFadeInLocalTimeMs  = m_crossfadeInState->m_localTimeMs;
		m_debugFadeInStartTimeMs  = m_crossfadeInState->m_clip->GetStartTime();
		m_debugFadeInEndTimeMs	  = m_crossfadeInState->m_clip->GetEndTime();

		m_debugFadeOutStateName	  = m_crossfadeOutState->m_name;
		m_debugFadeOutLocalTimeMs = m_crossfadeOutState->m_localTimeMs;
		m_debugFadeOutStartTimeMs = m_crossfadeOutState->m_clip->GetStartTime();
		m_debugFadeOutEndTimeMs	  = m_crossfadeOutState->m_clip->GetEndTime();
	}

	DebugRenderAnimationStateUI( m_debugFadeInStateName, m_debugFadeInLocalTimeMs, m_debugFadeInStartTimeMs, m_debugFadeInEndTimeMs, uiFadeInData );
	DebugRenderAnimationStateUI( m_debugFadeOutStateName, m_debugFadeOutLocalTimeMs, m_debugFadeOutStartTimeMs, m_debugFadeOutEndTimeMs, uiFadeOutdata );

	// graph
	AABB2 ui_graph = uiBounds.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );

	// t
	AABB2 uiTData  = ui_graph.GetBoxAtUVs( 0.f, 0.f, 0.1f, 1.f );
	AABB2 uiTValue = uiTData.GetBoxAtUVs( 0.f, 0.f, 1.f, 0.5f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiTValue, 20.f, Stringf( "%.1f", m_debugCrossfadeBlendValue ) );


	// easing function
	AABB2 uiEasingFunc = ui_graph.GetBoxAtUVs( 0.1f, 0.1f, 0.9f, 0.9f );


	AABB2 uiXAxisMax   = uiEasingFunc.GetBoxAtUVs( 0.7f, 0.f, 1.f, 0.1f );
	g_simpleBitmapFont->AddVertsForTextInBox2D( textVerts, uiXAxisMax, 20.f, Stringf( "%.1f", m_crossfadeDurationMs ) );

	// lerp data
	Vec2 lerpStart = uiEasingFunc.GetPointAtUV( Vec2( 0.1f, 0.2f ) );
	Vec2 lerpEnd   = uiEasingFunc.GetPointAtUV( Vec2( 1.f, 1.f ) );
	AddVertsForLineSegment2D( verts, lerpStart, lerpEnd, 5.f, Rgba8::WHITE );

	Vec2 lerpPoint = Lerp( lerpStart, lerpEnd, m_debugCrossfadeBlendValue );
	AddVertsForDisc2D( verts, lerpPoint, 10.f, Rgba8::RED );


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_simpleBitmapFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}