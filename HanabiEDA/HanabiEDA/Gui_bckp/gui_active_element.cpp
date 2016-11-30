#include "gui_active_element.h"

//Author: Gonzalo Julian Reina Kiperman

GUI_active_element::GUI_active_element()
{
	this->is_active = true;
	this->ref_x = this->ref_y = 0;
	this->exclusive_attention = false;
}

void GUI_active_element::set_is_active(bool is_active)
{
	bool dummy;
	if (!is_active)
		this->tell_not_in_use(&dummy);
	this->is_active = is_active;
}

void GUI_active_element::set_reference_point(float ref_x, float ref_y)
{
	this->ref_x = ref_x;
	this->ref_y = ref_y;
}

bool GUI_active_element::need_exclusive_attention()
{
	return exclusive_attention;
}
