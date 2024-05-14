#pragma once

#include "Game/Game.hpp"

class VertexBuffer;
class IndexBuffer;
class Clock;


//----------------------------------------------------------------------------------------------------------
class GameLoadMesh : public Game
{
public:
	GameLoadMesh();
	virtual ~GameLoadMesh() {}

	virtual void Startup() override;
	virtual void Update() override;
	virtual void Render() const override;
	virtual void Shutdown() override;

protected:
	VertexBuffer* m_modelVertexBuffer = nullptr;
	Shader*		  m_spriteLitShader	  = nullptr;
	void		  InitSpriteLitShader();
	void		  LoadMeshData();
	void		  RenderMeshData() const;

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
};