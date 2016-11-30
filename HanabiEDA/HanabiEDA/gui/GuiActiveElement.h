#ifndef GUI_ACTIVE_ELEMENT_H_
#define GUI_ACTIVE_ELEMENT_H_

#include "GuiElement.h"
#include <allegro5\allegro5.h>

class GuiActiveElement : public GuiElement
{
public:
	GuiActiveElement();	//Initialization
	void SetIsActive(bool is_active);	//React to mouse events?
	virtual bool FeedMouseState(const ALLEGRO_MOUSE_STATE& st, bool* close_contaner, bool* redraw) = 0;	
	//True if mouse interacted with element. 
	//close_container true if this element is inside a menu, and menu should be closed
	//redraw true if redraw is needed
	virtual bool ReleaseMouse() = 0;	//True if should redraw
	bool NeedsExclusiveAttention();	//Returns true if mouse element must be feed to this element.
protected:
	bool is_active;				//Default is true
	bool exclusive_attention;	//Default is false.
};

#endif