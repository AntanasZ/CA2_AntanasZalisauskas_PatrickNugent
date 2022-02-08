/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include <SFML/Graphics/Text.hpp>

#include "State.hpp"
#include "World.hpp"

class GameOverState : public State
{
public:
	GameOverState(StateStack& stack, Context context);

	virtual void Draw();
	virtual bool Update(sf::Time dt);
	virtual bool HandleEvent(const sf::Event& event);


private:
	sf::Text m_game_over_text;
	sf::Time m_elapsed_time;
};

