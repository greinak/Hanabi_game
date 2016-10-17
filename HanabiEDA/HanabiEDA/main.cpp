#include "gui/gui.h"
#include <iostream>
#include <fstream>
#include <string>
#include <allegro5\allegro5.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_ttf.h>

//Author: Gonzalo Julian Reina Kiperman

bool c_back(GUI_button* source, event_data data, void* user_data, bool* redraw)
{
	float x, y, r;
	source->get_position(&x, &y, &r);
	x += 0.1;
	y += 0.1;
	r += 0.001;
	source->set_position(x, y, r);
	*redraw = true;
	return false;
}

using namespace std;
int main(void)
{
	bool success = true;
	success &= al_init();
	success &= al_init_image_addon();
	success &= al_init_primitives_addon();
	success &= al_init_ttf_addon();
	success &= al_init_font_addon();
	success &= al_install_mouse();

	ifstream file;
	file.open("example.xml", std::ifstream::in);
	if (!file.is_open())
		return 1;
	Gui gui = Gui(file);
	GUI_button* button = (GUI_button*) gui.get_element_from_id("button_1");
	button->set_on_hover_movement_callback(c_back);
	while (1)
	{
		ALLEGRO_MOUSE_STATE st;
		al_get_mouse_state(&st);
		if(gui.feed_mouse_event(st))
			gui.redraw(); 
	}
	while (1);
}

