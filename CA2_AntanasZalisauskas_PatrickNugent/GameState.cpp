/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "GameState.hpp"

#include "Player.hpp"

/// <summary>
/// Edited By: Antanas Zalisauskas
///
///	Included player 2 in constructor, update and handleEvent
/// </summary>

GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, m_world(*context.window, *context.fonts, *context.sounds)
, m_player_1(*context.player1)
, m_player_2(*context.player2)
{
	context.music->Play(MusicThemes::kScoobyDooTheme);
}

void GameState::Draw()
{
	m_world.Draw();
}

bool GameState::Update(sf::Time dt)
{
	if (m_world.IsGameOver())
	{
		RequestStackPush(StateID::kGameOver);
	}

	m_world.Update(dt);
	CommandQueue& player_1_commands = m_world.getCommandQueue();
	CommandQueue& player_2_commands = m_world.getCommandQueue();
	m_player_1.HandleRealtimeInput(player_1_commands);
	m_player_2.HandleRealtimeInput(player_2_commands);

	if (m_world.IsGameOver())
	{
		RequestStackPush(StateID::kGameOver);
	}
	return true;
}

bool GameState::HandleEvent(const sf::Event& event)
{
	CommandQueue& player_1_commands = m_world.getCommandQueue();
	CommandQueue& player_2_commands = m_world.getCommandQueue();
	m_player_1.HandleEvent(event, player_1_commands);
	m_player_2.HandleEvent(event, player_2_commands);

	//Escape should bring up the Pause Menu
	if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
	{
		RequestStackPush(StateID::kPause);
	}
	return true;
}