#pragma once
#include "State.hpp"
#include "Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include "World.hpp"

class CharacterSelectionState : public State
{
public:
	CharacterSelectionState(StateStack& stack, Context context);
	virtual void Draw();
	virtual bool Update(sf::Time dt);
	virtual bool HandleEvent(const sf::Event& event);

private:
	World m_world;
	sf::Sprite m_background_sprite;
	GUI::Container m_gui_container;
	sf::Text m_select_character_text;
};

