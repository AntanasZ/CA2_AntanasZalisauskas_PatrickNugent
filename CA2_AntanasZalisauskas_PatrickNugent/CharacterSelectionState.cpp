#include "CharacterSelectionState.hpp"
#include "MenuState.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"

CharacterSelectionState::CharacterSelectionState(StateStack& stack, Context context)
	: State(stack, context)
, m_world(*context.window, *context.fonts, *context.sounds, false)
{
	//sf::Texture& texture = context.textures->Get(Textures::kTitleScreen);

	//m_background_sprite.setTexture(texture);

	auto scooby_button = std::make_shared<GUI::Button>(context);
	scooby_button->setPosition(400, 225);
	scooby_button->SetText("Scooby");
	scooby_button->SetCallback([this]()
		{
			//CharacterType* player = GetContext().playerCharacter;
			//m_world.m_player_character = CharacterType::kShaggy;
			m_world.AddCharacter(1, CharacterType::kScooby);
			RequestStackPop();
			RequestStackPush(StateID::kGame);
		});

	auto shaggy_button = std::make_shared<GUI::Button>(context);
	shaggy_button->setPosition(400, 300);
	shaggy_button->SetText("Shaggy");
	shaggy_button->SetCallback([this]()
		{
			m_world.AddCharacter(1, CharacterType::kShaggy);
			RequestStackPop();
			RequestStackPush(StateID::kGame);
		});

	auto fred_button = std::make_shared<GUI::Button>(context);
	fred_button->setPosition(400, 375);
	fred_button->SetText("Fred");
	fred_button->SetCallback([this]()
		{
			m_world.AddCharacter(1, CharacterType::kFred);
			RequestStackPop();
			RequestStackPush(StateID::kGame);
		});

	auto velma_button = std::make_shared<GUI::Button>(context);
	velma_button->setPosition(400, 450);
	velma_button->SetText("Velma");
	velma_button->SetCallback([this]()
		{
			m_world.AddCharacter(1, CharacterType::kVelma);
			RequestStackPop();
			RequestStackPush(StateID::kGame);
		});

	auto daphne_button = std::make_shared<GUI::Button>(context);
	daphne_button->setPosition(400, 525);
	daphne_button->SetText("Daphne");
	daphne_button->SetCallback([this]()
		{
			m_world.AddCharacter(1, CharacterType::kDaphne);
			RequestStackPop();
			RequestStackPush(StateID::kGame);
		});

	auto exit_button = std::make_shared<GUI::Button>(context);
	exit_button->setPosition(400, 600);
	exit_button->SetText("Back");
	exit_button->SetCallback([this]()
		{
			RequestStackPop();
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