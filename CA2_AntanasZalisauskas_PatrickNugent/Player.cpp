/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "Player.hpp"
#include "Character.hpp"
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
Player::Player(bool is_player_1)
{
	//Set initial key bindings
	if(is_player_1)
	{
		m_key_binding[sf::Keyboard::A] = PlayerAction::kPlayer1MoveLeft;
		m_key_binding[sf::Keyboard::D] = PlayerAction::kPlayer1MoveRight;
		m_key_binding[sf::Keyboard::W] = PlayerAction::kPlayer1MoveUp;
		//m_key_binding[sf::Keyboard::S] = PlayerAction::kMoveDown;
	}
	else
	{
		m_key_binding[sf::Keyboard::Left] = PlayerAction::kPlayer2MoveLeft;
		m_key_binding[sf::Keyboard::Right] = PlayerAction::kPlayer2MoveRight;
		m_key_binding[sf::Keyboard::Up] = PlayerAction::kPlayer2MoveUp;
		//m_key_binding[sf::Keyboard::Down] = PlayerAction::kMoveDown;
	}
	
	//m_key_binding[sf::Keyboard::Space] = PlayerAction::kFire;
	//m_key_binding[sf::Keyboard::M] = PlayerAction::kLaunchMissile

	//Set initial action bindings
	InitialiseActions();

	//Assign all categories to the player's character
	for(auto& pair : m_action_binding)
	{
		if(is_player_1)
			pair.second.category = Category::kPlayerCharacter1;
		else
			pair.second.category = Category::kPlayerCharacter2;
	}
}


void Player::HandleEvent(const sf::Event& event, CommandQueue& commands)
{
	if(event.type == sf::Event::KeyPressed)
	{
		auto found = m_key_binding.find(event.key.code);
		if(found != m_key_binding.end() && !IsRealtimeAction(found->second))
		{
			commands.Push(m_action_binding[found->second]);
		}
	}
}

void Player::HandleRealtimeInput(CommandQueue& commands)
{
	//Check if any keybinding keys are pressed
	for(auto pair: m_key_binding)
	{
		if(sf::Keyboard::isKeyPressed(pair.first) && IsRealtimeAction(pair.second))
		{
			commands.Push(m_action_binding[pair.second]);
		}
	}
}

void Player::AssignKey(PlayerAction action, sf::Keyboard::Key key)
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
}

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

	//m_action_binding[PlayerAction::kMoveDown].action = DerivedAction<Character>(CharacterMover(0, 1));

	/*m_action_binding[PlayerAction::kFire].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time
		)
	{
		a.Fire();
	});

	m_action_binding[PlayerAction::kLaunchMissile].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time
		)
	{
		a.LaunchMissile();
	});*/
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
