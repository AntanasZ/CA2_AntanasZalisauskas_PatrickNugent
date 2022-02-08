/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include <string>
#include <SFML/Graphics/Text.hpp>

#include "Component.hpp"
#include "ResourceIdentifiers.hpp"
namespace GUI
{
	class Label : public GUI::Component
	{
	public:
		typedef std::shared_ptr<Label> Ptr;
	public:
		Label(const std::string& text, const FontHolder& fonts);
		virtual bool IsSelectable() const override;
		void SetText(const std::string& text);
		void HandleEvent(const sf::Event& event) override;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	private:
		sf::Text m_text;
	};
}

