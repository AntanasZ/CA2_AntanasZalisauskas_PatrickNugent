/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "Player.hpp"
#include "NetworkProtocol.hpp"
#include <SFML/Network/Packet.hpp>
#include <algorithm>

#include "Character.hpp"

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	- Edited to only move player if they're not stunned
/// </summary>
struct CharacterMover
{
	CharacterMover(float vx, float vy, int identifier) : velocity(vx, vy), character_id(identifier)
	{

	}

	void operator()(Character& character, sf::Time) const
	{
		if (character.GetIdentifier() == character_id && !character.GetStunned())
		{
			character.Accelerate(velocity * character.GetMaxSpeed());
		}

	}

	sf::Vector2f velocity;
	int character_id;
};

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Handles logic for player jumping
///
///	Edited by: Antanas Zalisauskas
///
///	- Edited to only move player if they're not stunned
/// </summary>
struct CharacterJump
{
	CharacterJump(int identifier) : character_id(identifier)
	{
		
	}

	void operator()(Character& character, sf::Time dt) const
	{
		if (character.GetIdentifier() == character_id && character.GetCanJump() && !character.GetStunned()) //if player can jump, jump
		{
			character.ToggleCanJump(false); //set can jump to false

			character.SetVelocity(0.f, -sqrtf(2.0f * 981.f * character.GetJumpHeight()));
		}

	}

	int character_id;
};

struct CharacterFireTrigger
{
	CharacterFireTrigger(int identifier)
		: character_id(identifier)
	{
	}

	void operator() (Character& character, sf::Time) const
	{
		if (character.GetIdentifier() == character_id)
			character.Fire();
	}

	int character_id;
};

Player::Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding)
	: m_key_binding(binding)
	, m_current_mission_status(MissionStatus::kMissionRunning)
	, m_identifier(identifier)
	, m_socket(socket)
{
	// Set initial action bindings
	InitialiseActions();

	// Assign all categories to player's character
	for(auto & pair : m_action_binding)
	{
		pair.second.category = Category::kPlayerCharacter;
	}
		
}

void Player::HandleNetworkEvent(PlayerAction action, CommandQueue& commands)
{
	commands.Push(m_action_binding[action]);
}

void Player::HandleNetworkRealtimeChange(PlayerAction action, bool actionEnabled)
{
	m_action_proxies[action] = actionEnabled;
}


void Player::HandleEvent(const sf::Event& event, CommandQueue& commands)
{
	// Event
	if (event.type == sf::Event::KeyPressed)
	{
		PlayerAction action;
		if (m_key_binding && m_key_binding->CheckAction(event.key.code, action) && !IsRealtimeAction(action))
		{
			// Network connected -> send event over network
			if (m_socket)
			{
				sf::Packet packet;
				packet << static_cast<sf::Int32>(Client::PacketType::PlayerEvent);
				packet << m_identifier;
				packet << static_cast<sf::Int32>(action);
				m_socket->send(packet);
			}

			// Network disconnected -> local event
			else
			{
				commands.Push(m_action_binding[action]);
			}
		}
	}

	// Realtime change (network connected)
	if ((event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) && m_socket)
	{
		PlayerAction action;
		if (m_key_binding && m_key_binding->CheckAction(event.key.code, action) && IsRealtimeAction(action))
		{
			// Send realtime change over network
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::PlayerRealtimeChange);
			packet << m_identifier;
			packet << static_cast<sf::Int32>(action);
			packet << (event.type == sf::Event::KeyPressed);
			m_socket->send(packet);
		}
	}
}

bool Player::IsLocal() const
{
	// No key binding means this player is remote
	return m_key_binding != nullptr;
}

void Player::DisableAllRealtimeActions()
{
	for(auto & action : m_action_proxies)
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::PacketType::PlayerRealtimeChange);
		packet << m_identifier;
		packet << static_cast<sf::Int32>(action.first);
		packet << false;
		m_socket->send(packet);
	}
}

void Player::HandleRealtimeInput(CommandQueue& commands)
{
	// Check if this is a networked game and local player or just a single player game
	if ((m_socket && IsLocal()) || !m_socket)
	{
		// Lookup all actions and push corresponding commands to queue
		std::vector<PlayerAction> activeActions = m_key_binding->GetRealtimeActions();
		for(PlayerAction action : activeActions)
			commands.Push(m_action_binding[action]);
	}
}

void Player::HandleRealtimeNetworkInput(CommandQueue& commands)
{
	if (m_socket && !IsLocal())
	{
		// Traverse all realtime input proxies. Because this is a networked game, the input isn't handled directly
		for(auto pair : m_action_proxies)
		{
			if (pair.second && IsRealtimeAction(pair.first))
				commands.Push(m_action_binding[pair.first]);
		}
	}
}


void Player::SetMissionStatus(MissionStatus status)
{
	m_current_mission_status = status;
}

MissionStatus Player::GetMissionStatus() const
{
	return m_current_mission_status;
}

int Player::GetIdentifier() const
{
	return m_identifier;
}

void Player::InitialiseActions()
{
	m_action_binding[PlayerAction::kMoveLeft].action = DerivedAction<Character>(CharacterMover(-1, 0, m_identifier));
	m_action_binding[PlayerAction::kMoveRight].action = DerivedAction<Character>(CharacterMover(+1, 0, m_identifier));
	m_action_binding[PlayerAction::kMoveUp].action = DerivedAction<Character>(CharacterJump(m_identifier));
	m_action_binding[PlayerAction::kFire].action = DerivedAction<Character>(CharacterFireTrigger(m_identifier));
}


