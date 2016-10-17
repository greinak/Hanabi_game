#include "gui_menu.h"
#include <allegro5\allegro_primitives.h>

//Author: Gonzalo Julian Reina Kiperman

GUI_menu::GUI_menu() : menu_element_list()
{
	this->color = al_map_rgb(0, 0, 0);
	this->height = this->width = 0;
	this->is_active = true;
	this->is_volatile = false;
	this->radius = 0;
	this->was_button_pressed_last_time = false;
	this->is_blocker = false;
}

bool GUI_menu::feed_mouse_state(ALLEGRO_MOUSE_STATE& state, bool* should_close, bool* redraw)
{
	*redraw = false;
	GUI_active_element* active_menu_element = nullptr;
	bool interacted_with_menu = false;
	bool exclusive_interacted_with_menu = false;
	(*should_close) = false;	//This will always be false for a menu
	if (is_active)
	{
		bool redraw_flag = false;
		bool close_menu_temp = false; //Should this menu be closed?
		bool close_menu = false;
		//Use transformations to achieve desired position and rotation
		ALLEGRO_TRANSFORM backup, transform, transform2;
		list<GUI_element*>::iterator it;
		GUI_element* interacted_element = nullptr;

		//Step 1: Transform to position
		al_copy_transform(&backup, al_get_current_transform());		//Backup current transform
		al_identity_transform(&transform);							//Create new transformation
		al_rotate_transform(&transform, r);							//Rotate it
		al_translate_transform(&transform, x, y);						//Translate it
		al_compose_transform(&transform, &backup);					//Compose it with backup
		//Step 2: Transform to origin
		al_identity_transform(&transform2);							//Create new transform
		al_translate_transform(&transform2, -ref_x, -ref_y);			//Translate it so that it matches box origin
		al_compose_transform(&transform2, &transform);					//Compose it
		al_use_transform(&transform2);								//Use it


		exclusive_attention = false;
		//Check if any element needs exclusive attention
		for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
			if ((active_menu_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
				if ((exclusive_attention = active_menu_element->need_exclusive_attention()))
				{
					exclusive_interacted_with_menu |= active_menu_element->feed_mouse_state(state, &close_menu_temp,&redraw_flag);
					*redraw |= redraw_flag;
					close_menu |= close_menu_temp;
					exclusive_attention = active_menu_element->need_exclusive_attention();
					break;
				}

		if (!exclusive_attention)	//If no element need exclusive attention, or one needed but no longer does
		{
			for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
				if ((active_menu_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
				{
					if ((interacted_with_menu |= (active_menu_element->feed_mouse_state(state, &close_menu_temp, &redraw_flag))))
					{
						interacted_element = active_menu_element;
						*redraw |= redraw_flag;
						break;	//Pass mouse state to elements until an interaction occurs
					}
				}
			if (interacted_with_menu)	//If an interaction ocurred
			{
				//Start from scratch, list may have changed due to callbacks. Inform other elements they are not in use
				for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
				{
					if (interacted_element == (*it))
						continue;	//Ommit tell not in use to used element
					if ((active_menu_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
					{
						active_menu_element->tell_not_in_use(&redraw_flag);
						*redraw |= redraw_flag;
					}
				}
			}
			close_menu |= close_menu_temp;
		}
		if (close_menu)
		{
			set_is_active(false);
			set_is_visible(false);
			*redraw = true;
		}
		if (!interacted_with_menu && !exclusive_interacted_with_menu)
		{
			//If no interaction occurred, did mouse event occured above menu box?
			float mouse_x = state.x, mouse_y = state.y;
			al_invert_transform(&transform2);
			al_transform_coordinates(&transform2, &mouse_x, &mouse_y);
			if ((0 <= mouse_x && mouse_x <= width) && (0 <= mouse_y && mouse_y <= height))
				interacted_with_menu = true;	//Then mouse event ocurred in menu, so let's consider it caused an interaction. This will "absorb" mouse event
			else if (is_volatile && !was_button_pressed_last_time && (state.buttons & 1) )	//If mouse event was mouse button down and it did not occur in menu, and menu is volatile
			{
				set_is_active(false);
				set_is_visible(false);	//Then close menu
				*redraw |= true;
			}
		}
		al_use_transform(&backup);
		was_button_pressed_last_time = state.buttons & 1;
	}
	return interacted_with_menu || exclusive_interacted_with_menu || (is_active && is_blocker);
}

void GUI_menu::tell_not_in_use(bool* redraw)
{
	*redraw = false;
	if (is_active)
	{
		bool redraw_flag = false;
		GUI_active_element* active_menu_element;
		for (list<GUI_element*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)	//For all the elements in menu
			if ((active_menu_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
			{
				active_menu_element->tell_not_in_use(&redraw_flag);	//Tell them they are not in use
				*redraw |= redraw_flag;
			}
		if (is_volatile)	//If menu is volatile
		{
			is_active = false;
			is_visible = false;
			*redraw = true;
		}
	}
	was_button_pressed_last_time = false;
	exclusive_attention = false;
}

void GUI_menu::set_size(float width, float height)
{
	this->width = width;
	this->height = height;
}

void GUI_menu::set_menu_rectangle_rounded_radius(float radius)
{
	this->radius = radius;
}

void GUI_menu::set_background_color(ALLEGRO_COLOR color)
{
	this->color = color;
}

void GUI_menu::set_is_volatile(bool is_volatile)
{
	this->is_volatile = is_volatile;
}

void GUI_menu::set_menu_GUI_element_pointer_list(const list<GUI_element*> &menu_element_list)
{
	this->menu_element_list = menu_element_list;
}

list<GUI_element*>* GUI_menu::get_pointer_to_menu_GUI_element_pointer_list()
{
	return &this->menu_element_list;
}

void GUI_menu::set_is_blocker(bool is_blocker)
{
	this->is_blocker = is_blocker;
}

bool GUI_menu::get_element_position(GUI_element * element, float * x, float * y, float * r)
{
	ALLEGRO_TRANSFORM transform, transform2;
	GUI_menu* menu;
	bool el_found = false;
	//Use transform to calculate absolute position
	al_identity_transform(&transform);									//Create new transformation
	al_rotate_transform(&transform, this->r);								//Rotate it
	al_translate_transform(&transform, this->x, this->y);					//Translate it
	al_identity_transform(&transform2);									//Create new transform
	al_translate_transform(&transform2, -this->ref_x, -this->ref_y);		//Translate it so that it matches box origin
	al_compose_transform(&transform2, &transform);							//Compose it

	//Look for element
	for (list<GUI_element*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
		//If element is inside menu
		if ((*it) == element)
		{
			//Just return it's position
			(*it)->get_position(x, y, r);
			//(Transform it)
			al_transform_coordinates(&transform2, x, y);
			el_found = true;
			break;
		}
		//Also look in menus within menu
		else if ((menu = dynamic_cast<GUI_menu*>(*it)) != nullptr)
			//If element is found in menu within menu
			if (menu->get_element_position(element, x, y, r))
			{
				//Return it's position
				//(Transform it)
				al_transform_coordinates(&transform2, x, y);
				(*r) -= menu->r;
				el_found = true;
				break;
			}
	//If no element was found
	if (!el_found)
		//Set values to zero
		(*x) = (*y) = (*r) = 0;
	//Return element found?
	return el_found;
}

bool GUI_menu::set_element_position(GUI_element * element, float x, float y, float r)
{
	ALLEGRO_TRANSFORM transform, transform2;
	GUI_menu* menu;
	bool el_found = false;
	float new_x = x, new_y = y, new_r = r;
	//Use transform to calculate absolute position
	al_identity_transform(&transform);									//Create new transformation
	al_rotate_transform(&transform, this->r);								//Rotate it
	al_translate_transform(&transform, this->x, this->y);					//Translate it
	al_identity_transform(&transform2);									//Create new transform
	al_translate_transform(&transform2, -this->ref_x, -this->ref_y);		//Translate it so that it matches box origin
	al_compose_transform(&transform2, &transform);							//Compose it
	al_invert_transform(&transform2);									//Invert it

	//Transform x y and r coordinates to new coordinates
	al_transform_coordinates(&transform2, &new_x, &new_y);
	new_r += this->r;

	//Look for element
	for (list<GUI_element*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
		//If element is inside menu
		if ((*it) == element)
		{
			//Just set it's position
			(*it)->set_position(new_x, new_y, new_r);
			el_found = true;
			break;
		}
	//Also look in menus within menu
		else if ((menu = dynamic_cast<GUI_menu*>(*it)) != nullptr)
			//If element is found in menu within menu
			if (menu->set_element_position(element, new_x, new_y, new_r))
			{
				el_found = true;
				break;
			}
	//Return element found?
	return el_found;
}

void GUI_menu::draw()
{
	if (is_visible)
	{
		//Use transformations to achiveve desired position and rotation
		ALLEGRO_TRANSFORM backup, transform, transform2;
		//Step 1: Draw box
		al_copy_transform(&backup, al_get_current_transform());		//Backup current transform
		al_identity_transform(&transform);							//Create new transformation
		al_rotate_transform(&transform, r);							//Rotate it
		al_translate_transform(&transform, x, y);						//Translate it
		al_compose_transform(&transform, &backup);					//Compose it with backup
		al_use_transform(&transform);									//Use it
		if (width != 0 && height != 0)
		{
			al_draw_filled_rounded_rectangle(-ref_x, -ref_y, width - ref_x, height - ref_y, radius, radius, color);
			al_draw_rounded_rectangle(-ref_x, -ref_y, width - ref_x, height - ref_y, radius, radius, MENU_BORDER_COLOR, MENU_BORDER_THICKNESS);
		}
		//Step 2: Leave origin in top left corner in order to draw elements
		al_identity_transform(&transform2);							//Create new transform
		al_translate_transform(&transform2, -ref_x, -ref_y);			//Translate it so that it matches box origin
		al_compose_transform(&transform2, &transform);					//Compose it
		al_use_transform(&transform2);								//Use it
		//Now, draw menu elements
		//Elements must be drawn in reverse order!
		for (list<GUI_element*>::reverse_iterator it = menu_element_list.rbegin(); it != menu_element_list.rend(); ++it)
			(*it)->draw();
		al_use_transform(&backup);
	}
}
