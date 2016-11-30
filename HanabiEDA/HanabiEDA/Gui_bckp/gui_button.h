#ifndef GUI_BUTTON_H_
#define GUI_BUTTON_H_

#include <allegro5\allegro5.h>
#include "gui_active_element.h"

#define ALPHA_THRESHOLD	0.05	//Minimum alpha value to consider mouse is over element
class GUI_button;

//Author: Gonzalo Julian Reina Kiperman

//Callback prototype
typedef struct event_data event_data;
typedef bool(*GUI_button_callback)(GUI_button* source, event_data data, void* user_data, bool* redraw);
//Returns true if container should be closed (MENU)
//Set *redraw to true if must redraw. *redraw initialized in false 

//Struct used to hold event data
struct event_data
{
	bool was_forced;				//True if event was forced by another element above current element, that "obstructed" mouse event.
	float ref_offset_i;				//Relative to reference point, in bitmap axes.
	float ref_offset_j;
	float position_from_origin_i;	//Relative to origin of container, in container axes.
	float position_from_origin_j;
	float absolute_position_x;		//Absolute mouse position, in display axes.
	float absolute_position_y;
	bool mouse_over_element;		//Was mouse over element? invalid when was_forced == true
};

class GUI_button : public GUI_active_element
{
public:
	//Initialize variables
	GUI_button();
	//Base bitmap. nulltpr for no base bitmap
	void set_base_bitmap(ALLEGRO_BITMAP* base_bitmap);	
	//Bitmap to be shown when hovering, drawn after Base bitmap. nullptr in order to disable
	void set_hover_bitmap(ALLEGRO_BITMAP* hover_bitmap);
	//Bitmap to be shown when mouse is clicked, above base bitmap. nullptr in order to disable
	void set_click_bitmap(ALLEGRO_BITMAP* movement_bitmap);
	//Bitmap to be shown always, above base bitmap. nullptr in order to disable
	void set_aux_bitmap(ALLEGRO_BITMAP* aux_bitmap);
	//Use alternate bitmap? Default false
	void set_use_aux_bitmap(bool use_aux_bitmap);
	//Set user data, for callbacks
	void set_user_data(void* user_data);
	//Get user data
	void* get_user_data();
	//Set callback functions. NULL in order to disable callbacks. Initialized in NULL
	//When mouse is clicked down while hovering bitmap
	void set_on_click_down_callback(GUI_button_callback on_click_down);
	//When mouse is clicked down, and then a mouse movement occurs. Even if mouse is no longer over button
	void set_on_click_movement_callback(GUI_button_callback on_click_movement);
	//When mouse button is released, only if mouse is still over button
	void set_on_click_up_callback(GUI_button_callback on_click_up);
	//When mouse enters hovering button (if mouse button is not down). Or after mouse is released over button
	void set_on_hover_enter_callback(GUI_button_callback on_hover_enter);
	//When mouse moves hovering over button, but mouse is not clicked
	void set_on_hover_movement_callback(GUI_button_callback on_hover_movement);
	//When mouse leaves button. Or, if button was clicked down, before click down callback
	void set_on_hover_exit_callback(GUI_button_callback on_hover_exit);
	//Should alpha value be checked for mouse events?
	void set_check_alpha_for_mouse(bool check_alpha);	//Default is false
	//Draw button
	virtual void draw();
	//
	virtual bool feed_mouse_state(ALLEGRO_MOUSE_STATE& state, bool* should_close, bool* redraw);
	virtual void tell_not_in_use(bool* redraw);
private:
	bool is_being_hovered;
	bool is_being_clicked;
	//Bitmap variables
	ALLEGRO_BITMAP *base_bitmap, *hover_bitmap, *click_bitmap, *aux_bitmap;
	//Cursor varialbes
	ALLEGRO_MOUSE_CURSOR *hover_cursor, *click_cursor;
	//Hover actions
	GUI_button_callback on_hover_enter, on_hover_movement, on_hover_exit;
	//Click actions
	GUI_button_callback on_click_down, on_click_movement, on_click_up;
	//Settings
	bool check_alpha, use_aux_bitmap;
	//User data
	void* user_data;
};

#endif //GUI_BUTTON_H_