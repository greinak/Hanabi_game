#include "GuiElement.h"

GuiElement::GuiElement()
{
	this->is_visible = true;
	this->offset_x = this->offset_y = 0;
	this->x = this->y = this->r = 0;
}

void GuiElement::SetPosition(float x, float y, float r)
{
	this->x = x; this->y = y; this->r = r;
}

void GuiElement::SetOffset(float offset_x, float offset_y)
{
	this->offset_x = offset_x; this->offset_y = offset_y;
}

void GuiElement::GetPosition(float * x, float * y, float * r)
{
	(*x) = this->x; (*y) = this->y; (*r) = this->r;
}

void GuiElement::SetIsVisible(bool is_visible)
{
	this->is_visible = is_visible;
}

void GuiElement::TransformToPosition()
{
	//Use transformations to achiveve desired position and rotation
	ALLEGRO_TRANSFORM transform;
	al_copy_transform(&backup, al_get_current_transform());		//Backup current transform
	al_identity_transform(&transform);							//Create new transform
	al_rotate_transform(&transform, r);							//Rotate it
	al_translate_transform(&transform, x, y);					//Translate it
	al_compose_transform(&transform, &backup);					//Compose it with backup
	al_use_transform(&transform);								//Use it
}
