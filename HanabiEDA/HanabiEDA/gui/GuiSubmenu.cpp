#include "GuiSubmenu.h"
#include <allegro5\allegro_primitives.h>

GuiSubmenu::GuiSubmenu()
{
	this->backg_enabled = false;
	this->width = this->height = 0;
	this->backg_radius = 0;
	this->backg_color = al_map_rgb(0, 0, 0);
	this->is_volatile = this->is_blocker = false;
	this->last_time_button_pressed = false;
}

void GuiSubmenu::SetUseBackground(bool backg_enabled)
{
	this->backg_enabled = backg_enabled;
}

void GuiSubmenu::SetBackground(unsigned int width, unsigned int height, float backg_radius, ALLEGRO_COLOR backg_color)
{
	this->width = width; this->height = height; this->backg_radius = backg_radius;
	this->backg_color = backg_color;
}

void GuiSubmenu::SetIsVolatile(bool is_volatile)
{
	this->is_volatile = is_volatile;
}

void GuiSubmenu::SetIsBlocker(bool is_blocker)
{
	this->is_blocker = is_blocker;
}

void GuiSubmenu::AddElement(GuiElement * element)
{
	children.push_back(element);	//Front used in drawing...
}

bool GuiSubmenu::FeedMouseState(const ALLEGRO_MOUSE_STATE & st, bool *close_contaner, bool * redraw)
{
	*redraw = false;
	GuiActiveElement* active_menu_element = nullptr;
	bool interacted_with_menu = false;
	bool exclusive_interacted_with_menu = false;
	(*close_contaner) = false;	//This will always be false for a menu
	if (is_active)
	{
		bool redraw_flag = false;
		bool close_menu = false;
		bool close_menu_flag = false;
		//Use transformations to achieve desired position and rotation
		ALLEGRO_TRANSFORM backup, transform, transform2;
		list<GuiElement*>::iterator it;
		GuiElement* interacted_element = nullptr;

		//Step 1: Transform to position
		al_copy_transform(&backup, al_get_current_transform());		//Backup current transform
		al_identity_transform(&transform);							//Create new transformation
		al_rotate_transform(&transform, r);							//Rotate it
		al_translate_transform(&transform, x, y);						//Translate it
		al_compose_transform(&transform, &backup);					//Compose it with backup
																	//Step 2: Transform to origin
		al_identity_transform(&transform2);							//Create new transform
		al_translate_transform(&transform2, -offset_x, -offset_y);			//Translate it so that it matches box origin
		al_compose_transform(&transform2, &transform);					//Compose it
		al_use_transform(&transform2);								//Use it


		exclusive_attention = false;
		//Check if any element needs exclusive attention
		for (it = children.begin(); it != children.end(); ++it)
			if ((active_menu_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
				if ((exclusive_attention = active_menu_element->NeedsExclusiveAttention()))
				{
					exclusive_interacted_with_menu |= active_menu_element->FeedMouseState(st, &close_menu_flag, &redraw_flag);
					*redraw |= redraw_flag;
					exclusive_attention = active_menu_element->NeedsExclusiveAttention();
					close_menu |= close_menu_flag;
					break;
				}
		if (close_menu)
		{
			SetIsActive(false);
			SetIsVisible(false);
			*redraw = true;
			close_menu = false;
		}
		if (!exclusive_attention && is_active)	//If no element need exclusive attention, or one needed but no longer does, and menu is still active
		{
			for (it = children.begin(); it != children.end(); ++it)
				if ((active_menu_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
				{
					if ((interacted_with_menu |= (active_menu_element->FeedMouseState(st, &close_menu, &redraw_flag))))
					{
						interacted_element = active_menu_element;
						*redraw |= redraw_flag;
						close_menu |= close_menu_flag;
						break;	//Pass mouse state to elements until an interaction occurs
					}
				}
			if (interacted_with_menu)	//If an interaction ocurred
			{
				//Start from scratch, list may have changed due to callbacks. Inform other elements they are not in use
				for (it = children.begin(); it != children.end(); ++it)
				{
					if (interacted_element == (*it))
						continue;	//Ommit tell not in use to used element
					if ((active_menu_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
					{
						*redraw |= active_menu_element->ReleaseMouse();
					}
				}
			}
		}
		if (close_menu)
		{
			SetIsActive(false);
			SetIsVisible(false);
			*redraw = true;
		}
		if (!interacted_with_menu && !exclusive_interacted_with_menu)
		{
			//If no interaction occurred, did mouse event occured above menu box?
			float mouse_x = st.x, mouse_y = st.y;
			al_invert_transform(&transform2);
			al_transform_coordinates(&transform2, &mouse_x, &mouse_y);
			if ((0 <= mouse_x && mouse_x <= width) && (0 <= mouse_y && mouse_y <= height))
				interacted_with_menu = true;	//Then mouse event ocurred in menu, so let's consider it caused an interaction. This will "absorb" mouse event
			else if (is_volatile && !last_time_button_pressed && (st.buttons & 1))	//If mouse event was mouse button down and it did not occur in menu, and menu is volatile
			{
				SetIsActive(false);
				SetIsVisible(false);	//Then close menu
				*redraw |= true;
			}
		}
		al_use_transform(&backup);
		last_time_button_pressed = st.buttons & 1;
	}
	return interacted_with_menu || exclusive_interacted_with_menu || (is_active && is_blocker);
}


bool GuiSubmenu::ReleaseMouse()
{
	bool redraw = false;
	if (is_active)
	{
		GuiActiveElement* active_menu_element;
		for (list<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it)	//For all the elements in menu
			if ((active_menu_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
				redraw |= active_menu_element->ReleaseMouse();	//Tell them they are not in use
		if (is_volatile)	//If menu is volatile
		{
			is_active = false;
			is_visible = false;
			redraw = true;
		}
	}
	last_time_button_pressed = false;
	exclusive_attention = false;
	return redraw;
}

void GuiSubmenu::Draw()
{
	if (is_visible)
	{
		ALLEGRO_TRANSFORM transform;
		TransformToPosition();
		//Step 1: Draw box
		if (backg_enabled)
		{
			al_draw_filled_rounded_rectangle(-offset_x, -offset_y, width - offset_x, height - offset_y, backg_radius, backg_radius, backg_color);
			al_draw_rounded_rectangle(-offset_x, -offset_y, width - offset_x, height - offset_y, backg_radius, backg_radius, GUI_SUBMENU_BORDER_COLOR, GUI_SUBMENU_BORDER_THICKNESS);
		}
		//Step 2: Leave origin position-offset
		al_identity_transform(&transform);								//Create new transform
		al_translate_transform(&transform, -offset_x, -offset_y);		//Translate it so that it matches offset point
		al_compose_transform(&transform, al_get_current_transform());	//Compose it
		al_use_transform(&transform);									//Use it
		//Now, draw menu elements
		//Elements must be drawn in reverse order!
		for (list<GuiElement*>::reverse_iterator it = children.rbegin(); it != children.rend(); ++it)
			(*it)->Draw();
		al_use_transform(&backup);	//Restore transform!
	}
}