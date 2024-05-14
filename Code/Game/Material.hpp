#pragma once


#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>


class Shader;
class Texture;


//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
class Material
{
public:
	std::string		 m_name					= "UNKOWN MATERIAL";
	Shader*			 m_shader				= nullptr;
	VertexType		 m_vertexType			= VertexType::UNKOWN;
	Texture*		 m_diffuseTexture		= nullptr;
	Texture*		 m_normalTexture		= nullptr;
	Texture*		 m_specGlossEmitTexture = nullptr;
	Rgba8			 m_color				= Rgba8::WHITE;

	static Material* LoadFromXMLFile( std::string const& xmlFilePath );
};