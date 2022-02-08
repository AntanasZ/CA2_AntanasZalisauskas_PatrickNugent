/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "Pickup.hpp"

#include <iostream>
#include <SFML/Graphics/RenderTarget.hpp>

#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

namespace
{
	const std::vector<PickupData> Table = InitializePickupData();
}

Pickup::Pickup(PickupType type, int value, const TextureHolder& textures)
	: Entity(1)
	, m_type(type)
	, m_value(value)
	, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture))
{
	Utility::CentreOrigin(m_sprite);
}

unsigned Pickup::GetCategory() const
{
	return Category::Type::kPickup;
}

sf::FloatRect Pickup::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

void Pickup::Apply(Character& player) const
{
	Table[static_cast<int>(m_type)].m_action(player);
}

void Pickup::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

float Pickup::GetMaxSpeed() const
{
	return Table[static_cast<int>(m_type)].m_speed;
}

int Pickup::GetValue() const
{
	return m_value;
}

void Pickup::UpdateMovementPattern(sf::Time dt)
{
	//Pickup AI
	const std::vector<Direction>& directions = Table[static_cast<int>(m_type)].m_directions;
	if (!directions.empty())
	{
		//Move along the current direction, change direction
		if (m_travelled_distance > directions[m_directions_index].m_distance)
		{
			m_directions_index = (m_directions_index + 1) % directions.size();
			m_travelled_distance = 0.f;
		}

		//Compute velocity from direction
		double radians = Utility::ToRadians(directions[m_directions_index].m_angle + 90.f);
		float vx = GetMaxSpeed() * std::cos(radians);
		float vy = GetMaxSpeed() * std::sin(radians);

		SetVelocity(vx, vy);
		m_travelled_distance += GetMaxSpeed() * dt.asSeconds();

	}
}

void Pickup::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	UpdateMovementPattern(dt);
	Entity::UpdateCurrent(dt, commands);
}
