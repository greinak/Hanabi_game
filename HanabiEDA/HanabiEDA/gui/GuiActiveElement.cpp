#include "GuiActiveElement.h"

GuiActiveElement::GuiActiveElement()
{
	this->is_active = true;
	this->exclusive_attention = false;
}

void GuiActiveElement::SetIsActive(bool is_active)
{
	this->is_active = is_active;
}

bool GuiActiveElement::NeedsExclusiveAttention()
{
	return exclusive_attention;
}
