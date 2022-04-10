/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "Component.hpp"
namespace GUI
{
	Component::Component()
	: m_is_selected(false)
	, m_is_active(false)
	{
	}


	bool Component::IsSelected() const
	{
		return m_is_selected;
	}

	void Component::Select()
	{
		m_is_selected = true;
	}

	void Component::Deselect()
	{
		m_is_selected = false;
	}

	bool Component::IsActive() const
	{
		return m_is_active;
	}

	void Component::Activate()
	{
		m_is_active = true;
	}

	void GUI::Component::Deactivate()
	{
		m_is_active = false;
	}

}
