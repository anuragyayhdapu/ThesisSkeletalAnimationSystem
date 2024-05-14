#include "Game/Material.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/XmlUtils.hpp"


//----------------------------------------------------------------------------------------------------------
Material* Material::LoadFromXMLFile( std::string const& xmlFilePath )
{
	XmlDocument materialDefinitionXml;
	materialDefinitionXml.LoadFile( xmlFilePath.c_str() );

	XmlElement* rootElement	  = materialDefinitionXml.RootElement();

	Material*	material	  = new Material();
	material->m_name		  = ParseXmlAttribute( *rootElement, "name", material->m_name );

	std::string vertexTypeStr = ParseXmlAttribute( *rootElement, "vertexType", "UNKOWN VERTEX TYPE" );
	if ( vertexTypeStr == "Vertex_PCU" )
	{
		material->m_vertexType = VertexType::Vertex_PCU;
	}
	else if ( vertexTypeStr == "Vertex_PCUTBN" )
	{
		material->m_vertexType = VertexType::Vertex_PCUTBN;
	}
	else
	{
		material->m_vertexType = VertexType::UNKOWN;
	}

	std::string shaderFilePath				 = ParseXmlAttribute( *rootElement, "shader", "UNKOWN SHADER" );
	material->m_shader						 = g_theRenderer->CreateOrGetShaderByName( shaderFilePath.c_str(), material->m_vertexType );

	std::string diffuseTextureFilePath		 = ParseXmlAttribute( *rootElement, "diffuseTexture", "UNKOWN DIFFUSE TEXTURE" );
	material->m_diffuseTexture				 = g_theRenderer->CreateOrGetTextureFromFile( diffuseTextureFilePath.c_str() );

	std::string normalTextureFilePath		 = ParseXmlAttribute( *rootElement, "normalTexture", "UNKOWN NORMAL TEXTURE" );
	material->m_normalTexture				 = g_theRenderer->CreateOrGetTextureFromFile( normalTextureFilePath.c_str() );

	std::string specGlossEmitTextureFilePath = ParseXmlAttribute( *rootElement, "specGlossEmitTexture", "UNKOWN SPEC GLOSS EMIT TEXTURE" );
	material->m_specGlossEmitTexture		 = g_theRenderer->CreateOrGetTextureFromFile( specGlossEmitTextureFilePath.c_str() );

	material->m_color						 = ParseXmlAttribute( *rootElement, "color", material->m_color );

	return material;
}