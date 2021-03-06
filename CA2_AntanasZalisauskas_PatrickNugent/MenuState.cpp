/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "MenuState.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"


MenuState::MenuState(StateStack& stack, Context context)
: State(stack, context)
{
	sf::Texture& texture = context.textures->Get(Textures::kTitleScreen);

	m_background_sprite.setTexture(texture);

	/*auto play_button = std::make_shared<GUI::Button>(context);
	play_button->setPosition(400, 225);
	play_button->SetText("Play");
	play_button->SetCallback([this, context]()
	{
		*context.modeSelection = StateID::kGame;
		RequestStackPush(StateID::kCharacterSelection);
	});*/

	auto host_play_button = std::make_shared<GUI::Button>(context);
	host_play_button->setPosition(400, 200);
	host_play_button->SetText("Host");
	host_play_button->SetCallback([this, context]()
	{
		*context.modeSelection = StateID::kHostGame;
		RequestStackPop();
		RequestStackPush(StateID::kCharacterSelection);
	});

	auto join_play_button = std::make_shared<GUI::Button>(context);
	join_play_button->setPosition(400, 275);
	join_play_button->SetText("Join");
	join_play_button->SetCallback([this, context]()
	{
		*context.modeSelection = StateID::kJoinGame;
		RequestStackPop();
		RequestStackPush(StateID::kCharacterSelection);
	});

	auto settings_button = std::make_shared<GUI::Button>(context);
	settings_button->setPosition(400, 350);
	settings_button->SetText("Settings");
	settings_button->SetCallback([this]()
	{
		RequestStackPush(StateID::kSettings);
	});

	auto exit_button = std::make_shared<GUI::Button>(context);
	exit_button->setPosition(400, 425);
	exit_button->SetText("Exit");
	exit_button->SetCallback([this]()
	{
		RequestStackPop();
	});

	//m_gui_container.Pack(play_button);
	m_gui_container.Pack(host_play_button);
	m_gui_container.Pack(join_play_button);
	m_gui_container.Pack(settings_button);
	m_gui_container.Pack(exit_button);

	// Play menu theme
	context.music->Play(MusicThemes::kMenuTheme);
}

void MenuState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.setView(window.getDefaultView());
	window.draw(m_background_sprite);
	window.draw(m_gui_container);
	
}

bool MenuState::Update(sf::Time dt)
{
	return true;
}

bool MenuState::HandleEvent(const sf::Event& event)
{
	m_gui_container.HandleEvent(event);
	return false;
}

