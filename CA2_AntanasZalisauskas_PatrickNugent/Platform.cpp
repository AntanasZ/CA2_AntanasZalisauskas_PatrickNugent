/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "Platform.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

namespace
{
	const std::vector<PlatformData> Table = InitializePlatformData();
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Class for platform entities in the game
/// </summary>
Platform::Platform(PlatformType type, const TextureHolder& textures)
	: Entity(1),
	m_type(type),
	m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture))
{
	Utility::CentreOrigin(m_sprite);
}

unsigned Platform::GetCategory() const
{
	return Category::Type::kPlatform;
}

sf::FloatRect Platform::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

void Platform::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

void Platform::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	Entity::UpdateCurrent(dt, commands);
}
