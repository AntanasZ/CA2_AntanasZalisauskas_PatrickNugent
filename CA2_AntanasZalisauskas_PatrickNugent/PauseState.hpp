/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include "State.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include "Container.hpp"

class PauseState : public State
{
public:
	PauseState(StateStack& stack, Context context, bool lets_updates_through = false);
	~PauseState();

	virtual void		Draw();
	virtual bool		Update(sf::Time dt);
	virtual bool		HandleEvent(const sf::Event& event);


private:
	sf::Sprite			m_background_sprite;
	sf::Text			m_paused_text;
	GUI::Container 		m_gui_container;
	bool				m_let_updates_through;
};

