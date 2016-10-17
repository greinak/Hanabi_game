#include "gui_button.h"
#include <allegro5\allegro_primitives.h>


//Author: Gonzalo Julian Reina Kiperman

GUI_button::GUI_button()
{
	//I like to use this :)
	this->base_bitmap = nullptr;
	this->hover_bitmap = nullptr;
	this->click_bitmap = nullptr;
	this->aux_bitmap = nullptr;
	this->user_data = nullptr;
	this->check_alpha = false;
	this->on_click_down = nullptr;
	this->on_click_movement = nullptr;
	this->on_click_up = nullptr;
	this->on_hover_enter = nullptr;
	this->on_hover_movement = nullptr;
	this->on_hover_exit = nullptr;
	this->is_being_hovered = false;
	this->is_being_clicked = false;
	this->use_aux_bitmap = false;
	this->hover_cursor = nullptr;
	this->click_cursor = nullptr;
}

// --> Here we have the logic for mouse events! <--
bool GUI_button::feed_mouse_state(ALLEGRO_MOUSE_STATE& state, bool* should_close, bool* redraw)
{
	bool interacted_with_mouse = false;		//Interacted with mouse
	exclusive_attention = false;
	(*should_close) = false;				//Should container be closed?
	*redraw = false;
	if (is_active && base_bitmap != NULL)	//Is element active? Has it got base bitmap?
	{
		event_data data;
		ALLEGRO_TRANSFORM backup, transform;
		float x_mouse, y_mouse;
		data.was_forced = false;

		//Calculate absolute mouse position
		data.absolute_position_x = state.x;
		data.absolute_position_y = state.y;
		//

		//Calculate mouse position from container origin, using container axes
		al_copy_transform(&transform, al_get_current_transform());
		al_invert_transform(&transform);
		data.position_from_origin_i = state.x;
		data.position_from_origin_j = state.y;
		al_transform_coordinates(&transform, &data.position_from_origin_i, &data.position_from_origin_j);
		//

		//Use transformations to achiveve desired position and rotation
		al_copy_transform(&backup, al_get_current_transform());	//Backup current transform
		al_identity_transform(&transform);						//Create new transform
		al_rotate_transform(&transform, r);						//Rotate it
		al_translate_transform(&transform, x, y);					//Translate it
		al_compose_transform(&transform, &backup);				//Compose it with backup

		//Must invert to achieve what goes on...
		al_invert_transform(&transform);

		//Calculate mouse position from reference point, using bitmap axes
		data.ref_offset_i = state.x;
		data.ref_offset_j = state.y;
		al_transform_coordinates(&transform, &data.ref_offset_i, &data.ref_offset_j);

		//Calculate mouse position from bitmap origin, using bitmap axes
		x_mouse = data.ref_offset_i + ref_x;
		y_mouse = data.ref_offset_j + ref_y;

		//Check if mouse is inside element
		if (
			(0 <= x_mouse && x_mouse <= al_get_bitmap_width(base_bitmap)) &&
			(0 <= y_mouse && y_mouse <= al_get_bitmap_height(base_bitmap)) &&
				(
					!check_alpha || (al_get_pixel(base_bitmap, x_mouse, y_mouse).a >= ALPHA_THRESHOLD)
				)
			) 
		//Is it inside of element?
		{//YES
			data.mouse_over_element = true;
			//Is mouse button down?
			if (state.buttons & 1)
			{
				//YES
				data.mouse_over_element = true;
				//Was element being clicked?
				if (is_being_clicked)
				{
					//YES, so this is a click movement!
					exclusive_attention = true;			//On click movement, mouse events must still be reserved
					interacted_with_mouse = true;		//Mouse is interacting with element
					
					if (on_click_movement != nullptr)	//If there is a callback for click movement
						(*should_close) |= (*on_click_movement)(this, data, user_data, redraw);	//Execute click movement callback
				}
				else
				{	//No.
					//Was element being hovered?
					if (is_being_hovered)
					{		
						bool redraw2 = false;
						//Yes. So this is a click down!
						if(click_bitmap != nullptr)		//If there is a bitmap for when button is being clicked
							*redraw = true;						//Then must redraw. Click bitmap will be shown
						is_being_hovered = false;		//No longer hovering
						is_being_clicked = true;		//Now clicking
						exclusive_attention = true;		//On click down, must reserve mouse events
						interacted_with_mouse = true;	//Mouse is interacting with element

						if (on_hover_exit != nullptr)	//If there is a callback for hover exit
							(*should_close) |= (*on_hover_exit)(this, data, user_data, &redraw2);	//Execute hover exit callback
						if(is_active)	//IF still active after first callback
							if (on_click_down != nullptr)	//If there is a callback for clicking
								(*should_close) |= (*on_click_down)(this, data, user_data, redraw);	//Execute click down callback
						*redraw |= redraw2;
					}
					else
					{
						//No. Then, mouse button was down when it entered element. Ignore this!
						//This may have problems with thouchscreens, so touchscreens must be disabled
					}
				}
			}
			else
			{
				//No. Mouse button is not down
				//Was element being clicked?
				if (is_being_clicked)
				{								
					//Yes. So this is a click up event!
					if(click_bitmap != nullptr)			//If there is a bitmap for when button is being clicked down
						*redraw = true;							//Then must redraw, since click bitmap will be shown
					is_being_clicked = false;			//No longer clicking
					interacted_with_mouse = true;		//Mouse is interacting with element

					if (on_click_up != nullptr)				//If there is a callback for click up
						(*should_close) |= (*on_click_up)(this, data, user_data, redraw);		//Execute click up callback
					//Note: Do not enter hover on click up, wait for next event
					//Note that an element may be below another active element when mouse is clicked up
					//In that case, this element will be clicked up, but another element will receive hover enter
				}
				else
				{
					//No. Element was not being clicked.
					//Was it being hovered?
					if (is_being_hovered)
					{
						//Yes, so this is a hover movement
						interacted_with_mouse = true;		//Mouse is interacting with element

						if (on_hover_movement != nullptr)	//If there is a callback for hover movement
							(*should_close) |= (*on_hover_movement)(this, data, user_data, redraw);	//Execute hover movement callback
					}
					else
					{
						//No. So this is a hover enter!
						if(hover_bitmap != nullptr)			//If there is a bitmap for when button is being hovered
							*redraw = true;						//Then must redraw, since hover bitmap will be shown
						interacted_with_mouse = true;		//Mouse is interacting with element
						is_being_hovered = true;			//Now being hovered

						if (on_hover_enter != nullptr)	//If there is a callback for hover enter
							(*should_close) |= (*on_hover_enter)(this, data, user_data, redraw);	//Execute hover enter callback
					}
				}
			}
		}
		else
		{
			//No. Button is outside element
			data.mouse_over_element = false;
			//Was it being hovered?
			if (is_being_hovered)
			{
				//Yes. So this is a hover exit
				if(hover_bitmap != nullptr)		//If there is a bitmap for when button is being hovered
				*redraw = true;							//Then must redraw, since base bitmap will be shown
				interacted_with_mouse = true;		//Mouse is interacting with element
				is_being_hovered = false;			//No longer hovering

				if (on_hover_exit != nullptr)	//If there is a callback for hover_exit
					(*should_close) |= (*on_hover_exit)(this, data, user_data, redraw);	//Execute hover exit callback
			}
			else
			{
				//No. It was not being hovered
				//Was it being clicked?
				if (is_being_clicked)
				{
					//Yes, it was being clicked
					//Is mouse button down?
					if (state.buttons & 1)
					{
						//Yes, so this is a click movement! (outside element)
						interacted_with_mouse = true;		//Mouse is interacting with element
						exclusive_attention = true;			//Still needs exclusive attention
						if (on_click_movement != nullptr)	//If there is a callback for click movement
							(*should_close) |= (*on_click_movement)(this, data, user_data, redraw);	//Execute click movement callback
					}
					else
					{
						//No, so this is a click up!
						if(this->click_bitmap != nullptr)	//If there is a bitmap for when button is clicked down
							*redraw = true;						//Then must redraw, since bitmap will change from clicked to base bitmap
						interacted_with_mouse = true;		//Mouse is interacting with element
						is_being_clicked = false;			//No longer being clicked

						if (on_click_up != nullptr)	//If there is a callback for click up
							(*should_close) |= (*on_click_up)(this, data, user_data, redraw);	//Execute click up callback
					}
				}
				else
				{
					//Else, not hovering, not clicking, mouse outside element, nothing to do
				}
			}
		}
	}
	return interacted_with_mouse;
}

void GUI_button::tell_not_in_use(bool* redraw)
{
	*redraw = false;
	if (is_active)
	{
		bool redraw2 = false;
		event_data data;
		data.ref_offset_i = 0;
		data.ref_offset_j = 0;
		data.absolute_position_x = 0;
		data.absolute_position_y = 0;
		data.was_forced = true;
		if (is_being_clicked)
		{
			if (on_click_up != nullptr)	//If there is a callback for click up
				(*on_click_up)(this, data, user_data,&redraw2);	//Execute click up callback
			is_being_clicked = false;
			if (click_bitmap != nullptr)
				redraw2 = true;
		}
		if (hover_bitmap != nullptr)
			*redraw = true;
		if(is_active)	//Still active after first callback?
			if (is_being_hovered)
			{
				if (on_hover_exit != nullptr)	//If there is a callback for click up
					(*on_hover_exit)(this, data, user_data,redraw);	//Execute click up callback
				is_being_hovered = false;
			}
		*redraw |= redraw2;
	}
}

void GUI_button::set_base_bitmap(ALLEGRO_BITMAP * base_bitmap)
{
	this->base_bitmap = base_bitmap;
}

void GUI_button::set_hover_bitmap(ALLEGRO_BITMAP * hover_bitmap)
{
	this->hover_bitmap = hover_bitmap;
}

void GUI_button::set_click_bitmap(ALLEGRO_BITMAP * click_bitmap)
{
	this->click_bitmap = click_bitmap;
}

void GUI_button::set_aux_bitmap(ALLEGRO_BITMAP * aux_bitmap)
{
	this->aux_bitmap = aux_bitmap;
}

void GUI_button::set_use_aux_bitmap(bool use_aux_bitmap)
{
	this->use_aux_bitmap = use_aux_bitmap;
}

void GUI_button::set_user_data(void * user_data)
{
	this->user_data = user_data;
}

void * GUI_button::get_user_data()
{
	return user_data;
}

void GUI_button::set_on_click_down_callback(GUI_button_callback on_click_down)
{
	this->on_click_down = on_click_down;
}

void GUI_button::set_on_click_movement_callback(GUI_button_callback on_click_movement)
{
	this->on_click_movement = on_click_movement;
}

void GUI_button::set_on_click_up_callback(GUI_button_callback on_click_up)
{
	this->on_click_up = on_click_up;
}

void GUI_button::set_on_hover_enter_callback(GUI_button_callback on_hover_enter)
{
	this->on_hover_enter = on_hover_enter;
}

void GUI_button::set_on_hover_movement_callback(GUI_button_callback on_hover_movement)
{
	this->on_hover_movement = on_hover_movement;
}

void GUI_button::set_on_hover_exit_callback(GUI_button_callback on_hover_exit)
{
	this->on_hover_exit = on_hover_exit;
}

void GUI_button::set_check_alpha_for_mouse(bool check_alpha)
{
	this->check_alpha = check_alpha;
}

void GUI_button::draw()
{
	if (is_visible)
	{
		if (base_bitmap != nullptr || click_bitmap != nullptr || hover_bitmap != nullptr)
		{
			//Use transformations to achiveve desired position and rotation
			ALLEGRO_TRANSFORM backup, transform;
			al_copy_transform(&backup, al_get_current_transform());	//Backup current transform
			al_identity_transform(&transform);						//Create new transform
			al_rotate_transform(&transform, r);						//Rotate it
			al_translate_transform(&transform, x, y);					//Translate it
			al_compose_transform(&transform, &backup);				//Compose it with backup
			al_use_transform(&transform);								//Use it

			if (base_bitmap != nullptr)
			{
				al_draw_bitmap(base_bitmap, -ref_x, -ref_y, 0);
			}
			if (is_being_clicked)
			{
				if (click_bitmap != nullptr)
					al_draw_bitmap(click_bitmap, -ref_x, -ref_y, 0);
			}
			else if (is_being_hovered)
			{
				if (hover_bitmap != nullptr)
					al_draw_bitmap(hover_bitmap, -ref_x, -ref_y, 0);
			}
			if (aux_bitmap != nullptr)
				al_draw_bitmap(aux_bitmap, -ref_x, -ref_y, 0);
			al_use_transform(&backup);								//Restore transformation!!
		}
	}
}
