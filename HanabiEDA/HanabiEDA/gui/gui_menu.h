#ifndef GUI_MENU_H_
#define GUI_MENU_H_

#include "gui_active_element.h"
#include <allegro5\allegro5.h>
#include <list>

#define MENU_BORDER_THICKNESS	5
#define MENU_BORDER_COLOR		al_map_rgb(0,0,0)


//Author: Gonzalo Julian Reina Kiperman

using namespace std;

//This is an implementation of a menu
//A menu is just an element that contains elements
//It will also set a new origin for those elements
//this means, position of menu will be position (0,0) for elements
//Agnle of rotation of menu will be rotation 0 for elements

class GUI_menu : public GUI_active_element
{
public:
	//Variables initialization
	GUI_menu();
	//Sets size of menu
	void set_size(float width, float height);	//Default is (0,0)
	//Rounded edges? I can do that
	void set_menu_rectangle_rounded_radius(float radius); //Default is 0
	//Set background color
	void set_background_color(ALLEGRO_COLOR color);	//Default is black
	//Is menu deactivated when a click outside it occurs?
	void set_is_volatile(bool is_volatile);	//Default is false
	//List of pointers to menu elements
	void set_menu_GUI_element_pointer_list(const list<GUI_element*> &menu_element_list);
	//Get pointer to list of pointers to menu elements
	list<GUI_element*>* get_pointer_to_menu_GUI_element_pointer_list();
	//If menu is blocker, it won't let mouse events reach elements below it (if it's active)
	void set_is_blocker(bool is_blocker);
	//Get element position, relative to menu position. Returns false if element was not found
	bool get_element_position(GUI_element* element, float* x, float* y, float* r);
	//Sets element position, relative to menu position. Returns false if element was not found
	bool set_element_position(GUI_element* element, float x, float y, float r);
	//Draw menu
	virtual void draw();
	//
	virtual bool feed_mouse_state(ALLEGRO_MOUSE_STATE& state, bool* should_close, bool* redraw);
	virtual void tell_not_in_use(bool* redraw);
private:
	bool is_blocker;
	bool is_volatile;
	float width, height, radius;
	ALLEGRO_COLOR color;
	list<GUI_element*> menu_element_list;
	bool	was_button_pressed_last_time;
};
#endif //GUI_MENU_H_