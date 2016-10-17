#ifndef GUI_ACTIVE_ELEMENT_H_
#define GUI_ACTIVE_ELEMENT_H_

#include "gui_element.h"
#include <allegro5\allegro5.h>

//Author: Gonzalo Julian Reina Kiperman

//This is the base class for any GUI screen element that may interact with mouse
class GUI_active_element : public GUI_element
{
public:
	//Initialize variables
	GUI_active_element();
	//Is element active? if so, interactions will be enabled. If disabled, element will be told it is not in use.
	//Redraw may be needed after calling this function, so always redraw after this function
	void set_is_active(bool is_active);	//Default is true
	//Set reference point. You set the position of this point within element. This is also the center of rotation
	void set_reference_point(float ref_x, float ref_y);	//Default is (0,0)
	//Draw element, if it is visible
	virtual void draw() = 0;
	//Feed mouse state.
	//Returns true if mouse interacted with element.
	//should_close indicates if container should be closed
	//This function will set or clear 'need exclusive attention' flag if that is the case
	//It is guaranteed that if funtion returns false (no interaction ocurred), then element ask for attention flag is false
	//redraw true if must redraw. It is guaranteed that redraw is false if returns false (no interaction ocurred)
	//The only exception is when a menu closes because of a volatile flag set to true (interaction does not occur, but need redraw)
	virtual bool feed_mouse_state(ALLEGRO_MOUSE_STATE& state, bool* should_close, bool* redraw) = 0;
	//Tells the element that is not being used.
	virtual void tell_not_in_use(bool* redraw) = 0;
	//Does this element need exclusive attention? (That is, do not feed mouse state to other elements)
	bool need_exclusive_attention();
protected:
	bool exclusive_attention;
	bool is_active;
	float ref_x, ref_y;
};
#endif