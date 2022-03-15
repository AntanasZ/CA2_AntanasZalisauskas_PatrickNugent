/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "Player.hpp"
#include "Character.hpp"
#include "NetworkProtocol.hpp"
#include <SFML/Network/Packet.hpp>
#include <algorithm>

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	- Edited to only move player if they're not stunned
/// </summary>
struct CharacterMover
{
	CharacterMover(float vx, float vy) : velocity(vx, vy)
	{

	}

	void operator()(Character& character, sf::Time) const
	{
		if(!character.GetStunned())
		{
			character.Accelerate(velocity * character.GetMaxSpeed());
		}
		
	}

	sf::Vector2f velocity;
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
	CharacterJump() = default;

	void operator()(Character& character, sf::Time dt) const
	{
		if(character.GetCanJump() && !character.GetStunned()) //if player can jump, jump
		{
			character.ToggleCanJump(false); //set can jump to false

			character.SetVelocity(0.f, -sqrtf(2.0f * 981.f * character.GetJumpHeight()));
		}
		
	}
};

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	Changed constructor to take in a bool to determine whether the player is player 1
///	and assign appropriate key bindings based on this bool
/// </summary>
/// <param name="is_player_1">Bool value which determines if this player is player 1(true) or player 2(false)</param>
Player::Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding) 
	: m_key_binding(binding)
	, m_identifier(identifier)
	, m_socket(socket)
{
	// Set initial action bindings
	InitialiseActions();

	// Assign all categories to player's aircraft
	for (auto& pair : m_action_binding)
		pair.second.category = Category::kPlayerAircraft;
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

void Player::HandleRealtimeInput(CommandQueue& commands)
{
	// Check if this is a networked game and local player or just a single player game
	if ((m_socket && IsLocal()) || !m_socket)
	{
		// Lookup all actions and push corresponding commands to queue
		std::vector<PlayerAction> activeActions = m_key_binding->GetRealtimeActions();
		for (PlayerAction action : activeActions)
			commands.Push(m_action_binding[action]);
	}
}

void Player::HandleRealtimeNetworkInput(CommandQueue& commands)
{
	if (m_socket && !IsLocal())
	{
		// Traverse all realtime input proxies. Because this is a networked game, the input isn't handled directly
		for (auto pair : m_action_proxies)
		{
			if (pair.second && IsRealtimeAction(pair.first))
				commands.Push(m_action_binding[pair.first]);
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
	for (auto& action : m_action_proxies)
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::PacketType::PlayerRealtimeChange);
		packet << m_identifier;
		packet << static_cast<sf::Int32>(action.first);
		packet << false;
		m_socket->send(packet);
	}
}

/*void Player::AssignKey(PlayerAction action, sf::Keyboard::Key key)
{
	//Remove all keys that are already bound to action
	for(auto itr = m_key_binding.begin(); itr != m_key_binding.end();)
	{
		if(itr->second == action)
		{
			m_key_binding.erase(itr++);
		}
		else
		{
			++itr;
		}
	}
	m_key_binding[key] = action;
}

sf::Keyboard::Key Player::GetAssignedKey(PlayerAction action) const
{
	for(auto pair : m_key_binding)
	{
		if(pair.second == action)
		{
			return pair.first;
		}
	}
	return sf::Keyboard::Unknown;
}*/

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	-Changed DerivedAction to work with Character and CharacterMover
///	-Commented out methods relating to firing missiles and projectiles
/// </summary>
void Player::InitialiseActions()
{
	const float player_speed = 200.f;

	m_action_binding[PlayerAction::kPlayer1MoveLeft].action = DerivedAction<Character>(CharacterMover(-1, 0.f));
	m_action_binding[PlayerAction::kPlayer1MoveRight].action = DerivedAction<Character>(CharacterMover(+1, 0.f));
	m_action_binding[PlayerAction::kPlayer1MoveUp].action = DerivedAction<Character>(CharacterJump());

	m_action_binding[PlayerAction::kPlayer2MoveLeft].action = DerivedAction<Character>(CharacterMover(-1, 0.f));
	m_action_binding[PlayerAction::kPlayer2MoveRight].action = DerivedAction<Character>(CharacterMover(+1, 0.f));
	m_action_binding[PlayerAction::kPlayer2MoveUp].action = DerivedAction<Character>(CharacterJump());
}

bool Player::IsRealtimeAction(PlayerAction action)
{
	switch(action)
	{
	case PlayerAction::kPlayer1MoveLeft:
	case PlayerAction::kPlayer1MoveRight:
	case PlayerAction::kPlayer1MoveUp:
	case PlayerAction::kPlayer2MoveLeft:
	case PlayerAction::kPlayer2MoveRight:
	case PlayerAction::kPlayer2MoveUp:
	//case PlayerAction::kMoveDown:
	//case PlayerAction::kFire:
		return true;
	default:
		return false;
	}
}
