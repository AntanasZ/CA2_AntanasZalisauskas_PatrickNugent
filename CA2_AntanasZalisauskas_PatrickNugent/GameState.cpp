/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "GameState.hpp"

#include <iostream>

#include "Player.hpp"

/// <summary>
/// Edited By: Antanas Zalisauskas
///
///	Included player 2 in constructor, update and handleEvent
/// </summary>

GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, m_world(*context.window, *context.fonts, *context.sounds, false)
, m_player(nullptr, 1, context.keys1)
//, m_player_2(nullptr, 2, context.keys1)
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
	CommandQueue& commands = m_world.GetCommandQueue();
	//CommandQueue& player_2_commands = m_world.GetCommandQueue();
	m_player.HandleRealtimeInput(commands);

	if (m_world.IsGameOver())
	{
		RequestStackPush(StateID::kGameOver);
	}
	return true;
}

bool GameState::HandleEvent(const sf::Event& event)
{
	CommandQueue& commands = m_world.GetCommandQueue();
	m_player.HandleEvent(event, commands);

	//Escape should bring up the Pause Menu
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
	{
		RequestStackPush(StateID::kPause);
	}
	return true;
}