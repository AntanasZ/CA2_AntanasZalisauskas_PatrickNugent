/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "TitleState.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Sleep.hpp>

#include "ResourceHolder.hpp"

TitleState::TitleState(StateStack& stack, Context context)
: State(stack, context)
, m_show_text(true)
, m_text_effect_time(sf::Time::Zero)
{
	m_background_sprite.setTexture(context.textures->Get(Textures::kTitleScreen));
	m_text.setFont(context.fonts->Get(Fonts::Main));
	m_text.setString("Press any key to continue");
	Utility::CentreOrigin(m_text);
	m_text.setPosition(context.window->getView().getSize() / 2.f);
}

void TitleState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.draw(m_background_sprite);

	if(m_show_text)
	{
		window.draw(m_text);
	}
}

bool TitleState::Update(sf::Time dt)
{
	m_text_effect_time += dt;

	if(m_text_effect_time >= sf::seconds(0.5))
	{
		m_show_text = !m_show_text;
		m_text_effect_time = sf::Time::Zero;
	}
	return true;
}

bool TitleState::HandleEvent(const sf::Event& event)
{
	if(event.type == sf::Event::KeyReleased)
	{
		RequestStackPop();
		RequestStackPush(StateID::kMenu);
	}
	return true;
}
