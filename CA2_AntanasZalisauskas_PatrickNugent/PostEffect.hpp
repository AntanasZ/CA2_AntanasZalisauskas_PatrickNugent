/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include <SFML/System/NonCopyable.hpp>


namespace sf
{
	class RenderTarget;
	class RenderTexture;
	class Shader;
}

class PostEffect : sf::NonCopyable
{
public:
	virtual					~PostEffect();
	virtual void			Apply(const sf::RenderTexture& input, sf::RenderTarget& output) = 0;

	static bool				IsSupported();


protected:
	static void				ApplyShader(const sf::Shader& shader, sf::RenderTarget& output);
};

