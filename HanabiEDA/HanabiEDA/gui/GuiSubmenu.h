#ifndef _GUI_SUBMENU_H_
#define _GUI_SUBMENU_H_

#include "GuiActiveElement.h"
#include <list>

#define GUI_SUBMENU_BORDER_THICKNESS	5
#define GUI_SUBMENU_BORDER_COLOR		al_map_rgb(0,0,0)

using namespace std;

class GuiSubmenu : public GuiActiveElement
{
public:
	GuiSubmenu();
	void SetUseBackground(bool backg_enabled);	//Should draw background?
	void SetBackground(unsigned int width, unsigned int height, float backg_radius, ALLEGRO_COLOR backg_color);	//Customize background
	void SetIsVolatile(bool is_volatile);	//Is menu volatile? (will it close if click outside menu elements or menu background, if enabled)
	void SetIsBlocker(bool is_blocker);	//If true, menu will always interact with mouse
	void AddElement(GuiElement* element);
	virtual bool FeedMouseState(const ALLEGRO_MOUSE_STATE& st, bool *close_contaner, bool* redraw);	//Close container will always be false.
	virtual bool ReleaseMouse();	//True if should redraw
	virtual void Draw();
private:
	bool last_time_button_pressed;
	bool backg_enabled;					//Default is false
	unsigned int width, height;			//Default is 0
	float backg_radius;					//Default 0
	ALLEGRO_COLOR backg_color;			//Default is black
	bool is_volatile, is_blocker;		//Default is false
	list<GuiElement*> children;
};

#endif