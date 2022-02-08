/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include <SFML/Graphics/Sprite.hpp>

#include "Entity.hpp"
#include "PlatformType.hpp"
#include "ResourceIdentifiers.hpp"

class Platform : public Entity
{
public:
	Platform(PlatformType type, const TextureHolder& textures);
	unsigned int GetCategory() const override;

	sf::FloatRect GetBoundingRect() const override;

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;

private:
	PlatformType m_type;
	sf::Sprite m_sprite;
};

