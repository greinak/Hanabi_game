#include "GUI_element.h"


//Author: Gonzalo Julian Reina Kiperman

GUI_element::GUI_element()
{
	this->is_visible = true;
	this->x = this->y = this ->r = 0;
}

void GUI_element::set_position(float x, float y, float r)
{
	this->x = x;
	this->y = y;
	this->r = r;
}

void GUI_element::get_position(float * x, float * y, float* r)
{
	*x = this->x;
	*y = this->y;
	*r = this->r;
}

void GUI_element::set_is_visible(bool is_visible)
{
	this->is_visible = is_visible;
}
