#pragma once

#include "Game/Game.hpp"

class VertexBuffer;
class IndexBuffer;
class Clock;
class AnimClip;


//----------------------------------------------------------------------------------------------------------
class GameSkinning : public Game
{
public:
	GameSkinning();
	virtual ~GameSkinning() {}

	virtual void Startup() override;
	virtual void Update() override;
	virtual void Render() const override;
	virtual void Shutdown() override;

	virtual void PrintDebugScreenMessage() override;	

protected:
	// mesh data
	//VertexBuffer* m_modelVertexBuffer = nullptr;
	std::vector<Vertex_PCUTBN> m_meshVerts;
	//std::vector<unsigned int>  m_jointIds;
	std::vector<std::vector<std::pair<int, float>>> m_vertexJointIdWeightMapping;
	Shader*		  m_spriteLitShader	  = nullptr;
	void		  InitSpriteLitShader();
	void		  LoadMeshData();
	void		  RenderMeshData() const;

	// animation skeletal data
	AnimPose m_bindPose;
	AnimPose  m_secondMeshBindPose;
	AnimPose m_animatedPose;
	float	  m_animatedLocalTimeMs = 0.f;
	AnimClip* m_animatedClip = nullptr;
	void	 LoadAnimPose();
	void	  UpdateAnimatedPose();
	void	 RenderAnimPose() const;

	// light data
	Clock*		   m_lightControlClock	= nullptr;
	bool		   m_isDebugDrawTangent = true;
	LightConstants m_lightConstants;
	VertexBuffer*  m_sunMesh = nullptr;
	Quaternion	   m_sunRotation;
	void		   InitLightConstants();
	void		   CreateSunVerts();
	void		   UpdateLightConstants();
	void		   PrintLightConstants() const;
	void		   RenderSunDirection() const;

	// for screen capture
	void ModifyRunningSlideAnimationClip();
};