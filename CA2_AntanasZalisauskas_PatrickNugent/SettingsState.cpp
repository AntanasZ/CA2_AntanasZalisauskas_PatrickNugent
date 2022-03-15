/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "SettingsState.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "StateStack.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	-Edited methods in whole class to include player 2
/// </summary>
SettingsState::SettingsState(StateStack& stack, Context context)
	: State(stack, context)
	, m_gui_container()
{
	m_background_sprite.setTexture(context.textures->Get(Textures::kTitleScreen));

	m_player_1_controls.setFont(context.fonts->Get(Fonts::Main));
	m_player_1_controls.setString("Player 1");
	m_player_1_controls.setPosition(350.f, 250.f);

	m_player_2_controls.setFont(context.fonts->Get(Fonts::Main));
	m_player_2_controls.setString("Player 2");
	m_player_2_controls.setPosition(650.f, 250.f);

	// Build key binding buttons and labels
	AddButtonLabel(PlayerAction::kPlayer1MoveLeft, 300.f, "Move Left", context, true);
	AddButtonLabel(PlayerAction::kPlayer1MoveRight, 350.f, "Move Right", context, true);
	AddButtonLabel(PlayerAction::kPlayer1MoveUp, 400.f, "Jump", context, true);

	AddButtonLabel(PlayerAction::kPlayer2MoveLeft, 300.f, "Move Left", context, false);
	AddButtonLabel(PlayerAction::kPlayer2MoveRight, 350.f, "Move Right", context, false);
	AddButtonLabel(PlayerAction::kPlayer2MoveUp, 400.f, "Jump", context, false);

	//AddButtonLabel(PlayerAction::kMoveDown, 450.f, "Move Down", context);
	//AddButtonLabel(PlayerAction::kFire, 500.f, "Fire", context);
	//AddButtonLabel(PlayerAction::kLaunchMissile, 550.f, "Missile", context);

	UpdateLabels();

	auto back_button = std::make_shared<GUI::Button>(context);
	back_button->setPosition(450.f, 620.f);
	back_button->SetText("Back");
	back_button->SetCallback(std::bind(&SettingsState::RequestStackPop, this));

	m_gui_container.Pack(back_button);
}

void SettingsState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;

	window.draw(m_background_sprite);
	window.draw(m_player_1_controls);
	window.draw(m_player_2_controls);
	window.draw(m_gui_container);
}

bool SettingsState::Update(sf::Time)
{
	return true;
}

bool SettingsState::HandleEvent(const sf::Event& event)
{
	bool is_key_binding = false;

	// Iterate through all key binding buttons to see if they are being pressed, waiting for the user to enter a key
	for (std::size_t i = 0; i < 2 * (static_cast<int>(PlayerAction::kActionCount)); ++i)
	{
		if (m_binding_buttons[i]->IsActive())
		{
			is_key_binding = true;
			if (event.type == sf::Event::KeyReleased)
			{
				// Player 1
				if (i < static_cast<int>(PlayerAction::kActionCount))
					GetContext().keys1->AssignKey(static_cast<PlayerAction>(i), event.key.code);

				// Player 2
				else
					GetContext().keys2->AssignKey(static_cast<PlayerAction>(i - static_cast<int>(PlayerAction::kActionCount)), event.key.code);

				m_binding_buttons[i]->Deactivate();
			}
			break;
		}
	}

	// If pressed button changed key bindings, update labels; otherwise consider other buttons in container
	if (is_key_binding)
		UpdateLabels();
	else
		m_gui_container.HandleEvent(event);

	return false;
}

void SettingsState::UpdateLabels()
{
	for (std::size_t i = 0; i < static_cast<int>(PlayerAction::kActionCount); ++i)
	{
		auto action = static_cast<PlayerAction>(i);

		// Get keys of both players
		sf::Keyboard::Key key1 = GetContext().keys1->GetAssignedKey(action);
		sf::Keyboard::Key key2 = GetContext().keys2->GetAssignedKey(action);

		// Assign both key strings to labels
		m_binding_labels[i]->SetText(Utility::toString(key1));
		m_binding_labels[i + static_cast<int>(PlayerAction::kActionCount)]->SetText(Utility::toString(key2));
	}
}

void SettingsState::AddButtonLabel(PlayerAction action, float y, const std::string& text, Context context, bool is_player_1)
{
	m_binding_buttons[static_cast<int>(action)] = std::make_shared<GUI::Button>(context);
	
	m_binding_buttons[static_cast<int>(action)]->SetText(text);
	m_binding_buttons[static_cast<int>(action)]->SetToggle(true);

	m_binding_labels[static_cast<int>(action)] = std::make_shared<GUI::Label>("", *context.fonts);

	if(is_player_1)
	{
		m_binding_buttons[static_cast<int>(action)]->setPosition(300.f, y);
		m_binding_labels[static_cast<int>(action)]->setPosition(520.f, y + 15.f);
	}
	else
	{
		m_binding_buttons[static_cast<int>(action)]->setPosition(600.f, y);
		m_binding_labels[static_cast<int>(action)]->setPosition(820.f, y + 15.f);
	}
		

	m_gui_container.Pack(m_binding_buttons[static_cast<int>(action)]);
	m_gui_container.Pack(m_binding_labels[static_cast<int>(action)]);
}


