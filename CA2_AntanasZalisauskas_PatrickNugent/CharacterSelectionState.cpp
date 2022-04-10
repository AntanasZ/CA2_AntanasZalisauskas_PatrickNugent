/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "CharacterSelectionState.hpp"
#include "MenuState.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"

/// <summary>
/// Written by: Patrick Nugent
///
///	A menu that allows the player to choose a character and store their selection
/// </summary>
CharacterSelectionState::CharacterSelectionState(StateStack& stack, Context context)
	: State(stack, context)
, m_world(*context.window, *context.fonts, *context.sounds, false)
{
	sf::Texture& texture = context.textures->Get(Textures::kTitleScreen);

	m_background_sprite.setTexture(texture);

	m_select_character_text.setFont(context.fonts->Get(Fonts::Main));
	m_select_character_text.setString("Select your character");
	m_select_character_text.setPosition(315.f, 160.f);
	m_select_character_text.setCharacterSize(40u);
	m_select_character_text.setFillColor(sf::Color::Color(237, 137, 96, 255));

	auto scooby_button = std::make_shared<GUI::Button>(context);
	scooby_button->setPosition(400, 225);
	scooby_button->SetText("Scooby");
	scooby_button->SetCallback([this, context]()
		{
			*context.characterSelection = CharacterType::kScooby;
			RequestStackPop();
			RequestStackPush(*context.modeSelection);
		});

	auto shaggy_button = std::make_shared<GUI::Button>(context);
	shaggy_button->setPosition(400, 300);
	shaggy_button->SetText("Shaggy");
	shaggy_button->SetCallback([this, context]()
		{
			*context.characterSelection = CharacterType::kShaggy;
			RequestStackPop();
			RequestStackPush(*context.modeSelection);
		});

	auto fred_button = std::make_shared<GUI::Button>(context);
	fred_button->setPosition(400, 375);
	fred_button->SetText("Fred");
	fred_button->SetCallback([this, context]()
		{
			*context.characterSelection = CharacterType::kFred;
			RequestStackPop();
			RequestStackPush(*context.modeSelection);
		});

	auto velma_button = std::make_shared<GUI::Button>(context);
	velma_button->setPosition(400, 450);
	velma_button->SetText("Velma");
	velma_button->SetCallback([this, context]()
		{
			*context.characterSelection = CharacterType::kVelma;
			RequestStackPop();
			RequestStackPush(*context.modeSelection);
		});

	auto daphne_button = std::make_shared<GUI::Button>(context);
	daphne_button->setPosition(400, 525);
	daphne_button->SetText("Daphne");
	daphne_button->SetCallback([this, context]()
		{
			*context.characterSelection = CharacterType::kDaphne;
			RequestStackPop();
			RequestStackPush(*context.modeSelection);
		});

	auto exit_button = std::make_shared<GUI::Button>(context);
	exit_button->setPosition(400, 600);
	exit_button->SetText("Back");
	exit_button->SetCallback([this]()
		{
			RequestStackPop();
			RequestStackPush(StateID::kMenu);
		});

	m_gui_container.Pack(scooby_button);
	m_gui_container.Pack(shaggy_button);
	m_gui_container.Pack(fred_button);
	m_gui_container.Pack(velma_button);
	m_gui_container.Pack(daphne_button);
	m_gui_container.Pack(exit_button);
}

void CharacterSelectionState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.setView(window.getDefaultView());
	window.draw(m_background_sprite);
	window.draw(m_gui_container);
	window.draw(m_select_character_text);
}

bool CharacterSelectionState::Update(sf::Time dt)
{
	return true;
}

bool CharacterSelectionState::HandleEvent(const sf::Event& event)
{
	m_gui_container.HandleEvent(event);
	return false;
}
