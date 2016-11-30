#include "GuiButton.h"

GuiButton::GuiButton()
{
	this->base_bitmap = this->hover_bitmap = this->click_bitmap = this->top_bitmap = nullptr;
	this->on_click_down = nullptr;
	this->on_click_movement = nullptr;
	this->on_click_up = nullptr;
	this->on_hover_enter = nullptr;
	this->on_hover_movement = nullptr;
	this->on_hover_exit = nullptr;
	this->use_top_bitmap = false;
	this->user_data = nullptr;
	this->check_bitmap_alpha = false;
	this->is_being_hovered = this->is_being_clicked = false;
}

void GuiButton::SetBitmap(ALLEGRO_BITMAP * bitmap)
{
	this->base_bitmap = bitmap;
}

void GuiButton::SetHoverBitmap(ALLEGRO_BITMAP * bitmap)
{
	this->hover_bitmap = bitmap;
}

void GuiButton::SetClickBitmap(ALLEGRO_BITMAP * bitmap)
{
	this->click_bitmap = bitmap;
}

void GuiButton::SetTopBitmap(ALLEGRO_BITMAP * bitmap)
{
	this->top_bitmap = bitmap;
}

void GuiButton::SetUseTopBitmap(bool use_top_bitmap)
{
	this->use_top_bitmap = use_top_bitmap;
}

void GuiButton::SetUserData(void * user_data)
{
	this->user_data = user_data;
}

void GuiButton::SetCheckBitmapAlpha(bool check_bitmap_alpha)
{
	this->check_bitmap_alpha = check_bitmap_alpha;
}

void GuiButton::SetOnClickDownCallback(GUI_button_callback * on_click_down)
{
	this->on_click_down = on_click_down;
}

void GuiButton::SetOnClickMovementCallback(GUI_button_callback * on_click_movement)
{
	this->on_click_movement = on_click_movement;
}

void GuiButton::SetOnClickUpCallback(GUI_button_callback * on_click_up)
{
	this->on_click_up = on_click_up;
}

void GuiButton::SetOnHoverEnterCallback(GUI_button_callback * on_hover_enter)
{
	this->on_hover_enter = on_hover_enter;
}

void GuiButton::SetOnHoverMovementCallback(GUI_button_callback * on_hover_movement)
{
	this->on_hover_movement = on_hover_movement;
}

void GuiButton::SetOnHoverExitCallback(GUI_button_callback * on_hover_exit)
{
	this->on_hover_exit = on_hover_exit;
}

bool GuiButton::ReleaseMouse()
{
	bool redraw = false;
	bool callback_redraw = false;
	if (is_active && is_being_clicked)
	{
		if (on_click_up != nullptr)	//If there is a callback for click up
			(*on_click_up)(this, true, false, user_data,&callback_redraw);	//Execute click up callback
		is_being_clicked = false;
		redraw |= callback_redraw;
		callback_redraw = false;
		if (click_bitmap != nullptr)
			redraw = true;
	}
	if (is_active && is_being_hovered)	//Still active after first callback?
	{
		if (on_hover_exit != nullptr)	//If there is a callback for click up
			(*on_hover_exit)(this, true, false, user_data, &callback_redraw);	//Execute click up callback
		is_being_hovered = false;
		redraw |= callback_redraw;
		if (hover_bitmap != nullptr)
			redraw = true;
	}
	return redraw;
}

bool GuiButton::FeedMouseState(const ALLEGRO_MOUSE_STATE & st, bool *close_contaner, bool * redraw)
{
	bool interacted_with_mouse = false;		//Interacted with mouse
	exclusive_attention = false;
	(*close_contaner) = false;				//Should container be closed?
	*redraw = false;
	if (is_active && base_bitmap != NULL)	//Is element active? Has it got base bitmap?
	{
		ALLEGRO_TRANSFORM backup, transform;
		float x_mouse, y_mouse;
		x_mouse = st.x;
		y_mouse = st.y;
		al_copy_transform(&backup, al_get_current_transform());	//Backup current transform
		al_identity_transform(&transform);						//Create new transform
		al_rotate_transform(&transform, r);						//Rotate it
		al_translate_transform(&transform, x, y);					//Translate it
		al_compose_transform(&transform, &backup);				//Compose it with backup
		al_invert_transform(&transform);
		//Calculate mouse position from reference point, using bitmap axes
		al_transform_coordinates(&transform, &x_mouse, &y_mouse);
		//Calculate mouse position from bitmap origin, using bitmap axes
		x_mouse += offset_x;
		y_mouse += offset_y;

		//Check if mouse is inside element
		if (
				(0 <= x_mouse && x_mouse <= al_get_bitmap_width(base_bitmap)) &&
				(0 <= y_mouse && y_mouse <= al_get_bitmap_height(base_bitmap)) &&
				(
					(!check_bitmap_alpha) || (al_get_pixel(base_bitmap, x_mouse, y_mouse).a >= ((float)GUI_BUTTON_ALPHA_THRESHOLD/UCHAR_MAX))
				)
			)
		//Is it inside of element?
		{//YES

			//Is mouse button down?
			if (st.buttons & 1)
			{
				//YES
				//Was element being clicked?
				if (is_being_clicked)
				{
					//YES, so this is a click movement!
					exclusive_attention = true;			//On click movement, mouse events must still be reserved
					interacted_with_mouse = true;		//Mouse is interacting with element
					
					if (on_click_movement != nullptr)	//If there is a callback for click movement
						(*close_contaner) |= (*on_click_movement)(this, false, true, user_data, redraw);	//Execute click movement callback
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
							(*close_contaner) |= (*on_hover_exit)(this,false,true,user_data,&redraw2);	//Execute hover exit callback
						if(is_active)	//IF still active after first callback
							if (on_click_down != nullptr)	//If there is a callback for clicking
								(*close_contaner) |= (*on_click_down)(this,false,true,user_data,redraw);	//Execute click down callback
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
						(*close_contaner) |= (*on_click_up)(this, false, true, user_data, redraw);		//Execute click up callback
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
							(*close_contaner) |= (*on_hover_movement)(this, false, true, user_data, redraw);	//Execute hover movement callback
					}
					else
					{
						//No. So this is a hover enter!
						if(hover_bitmap != nullptr)			//If there is a bitmap for when button is being hovered
							*redraw = true;						//Then must redraw, since hover bitmap will be shown
						interacted_with_mouse = true;		//Mouse is interacting with element
						is_being_hovered = true;			//Now being hovered

						if (on_hover_enter != nullptr)	//If there is a callback for hover enter
							(*close_contaner) |= (*on_hover_enter)(this,false,true,user_data,redraw);	//Execute hover enter callback
					}
				}
			}
		}
		else
		{
			//No. Button is outside element
			//Was it being hovered?
			if (is_being_hovered)
			{
				//Yes. So this is a hover exit
				if(hover_bitmap != nullptr)		//If there is a bitmap for when button is being hovered
				*redraw = true;							//Then must redraw, since base bitmap will be shown
				interacted_with_mouse = true;		//Mouse is interacting with element
				is_being_hovered = false;			//No longer hovering

				if (on_hover_exit != nullptr)	//If there is a callback for hover_exit
					(*close_contaner) |= (*on_hover_exit)(this,false,false,user_data,redraw);	//Execute hover exit callback
			}
			else
			{
				//No. It was not being hovered
				//Was it being clicked?
				if (is_being_clicked)
				{
					//Yes, it was being clicked
					//Is mouse button down?
					if (st.buttons & 1)
					{
						//Yes, so this is a click movement! (outside element)
						interacted_with_mouse = true;		//Mouse is interacting with element
						exclusive_attention = true;			//Still needs exclusive attention
						if (on_click_movement != nullptr)	//If there is a callback for click movement
							(*close_contaner) |= (*on_click_movement)(this, false, false, user_data, redraw);	//Execute click movement callback
					}
					else
					{
						//No, so this is a click up!
						if(this->click_bitmap != nullptr)	//If there is a bitmap for when button is clicked down
							*redraw = true;						//Then must redraw, since bitmap will change from clicked to base bitmap
						interacted_with_mouse = true;		//Mouse is interacting with element
						is_being_clicked = false;			//No longer being clicked

						if (on_click_up != nullptr)	//If there is a callback for click up
							(*close_contaner) |= (*on_click_up)(this, false, false, user_data, redraw);	//Execute click up callback
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

void GuiButton::Draw()
{
	if (is_visible && base_bitmap != nullptr)
	{
		//Use transformations to achiveve desired position and rotation
		TransformToPosition();
		if (base_bitmap != nullptr)
			al_draw_bitmap(base_bitmap, -offset_x, -offset_y, 0);
		if (is_being_clicked && click_bitmap != nullptr)
			al_draw_bitmap(click_bitmap, -offset_x, -offset_y, 0);
		else if (is_being_hovered && hover_bitmap != nullptr)
			al_draw_bitmap(hover_bitmap, -offset_x, -offset_y, 0);
		if (use_top_bitmap && top_bitmap != nullptr)
			al_draw_bitmap(top_bitmap, -offset_x, -offset_y, 0);
		al_use_transform(&backup);								//Restore transformation!!
	}
}
