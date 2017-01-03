#include "Gui/Gui.h"
#include <iostream>
#include <fstream>
#include <string>
#include <allegro5\allegro5.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_ttf.h>

//Author: Gonzalo Julian Reina Kiperman


using namespace std;
int main2(void)
{
	//This main is just for testing purposes...
	bool success = true;
	success &= al_init();
	success &= al_init_image_addon();
	success &= al_init_primitives_addon();
	success &= al_init_ttf_addon();
	success &= al_init_font_addon();
	success &= al_install_mouse();
	ifstream file;
	file.open("menu_data/hanabi_game_ui.xml", std::ifstream::in);
	if (!file.is_open())
		return 1;
	Gui gui = Gui(file);
	//GuiButton* button = (GuiButton*) gui.get_element_from_id("button_1");
	//button->set_on_hover_movement_callback(c_back);
	ALLEGRO_EVENT_QUEUE* ev_q = al_create_event_queue();
	ALLEGRO_DISPLAY* disp = gui.get_display();
	al_register_event_source(ev_q, al_get_mouse_event_source());
	al_register_event_source(ev_q, al_get_display_event_source(disp));
	bool redraw = false;
	while (1)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(ev_q, &ev);
		if (ev.any.source == al_get_mouse_event_source() && ev.mouse.display == disp)
		{
			if (ev.mouse.type == ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY)
			{
				gui.force_release_mouse();
				redraw = true;
			}
			else
			{
				ALLEGRO_MOUSE_STATE st;
				al_get_mouse_state(&st);
				if (gui.feed_mouse_event(st))
					redraw = true;
			}
		}
		else if (ev.any.source == al_get_display_event_source(disp))
		{
			if (ev.display.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT)
			{
				gui.force_release_mouse();
				redraw = true;
			}
			else if (ev.display.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN)
				redraw = true;
		}
		if (redraw && al_is_event_queue_empty(ev_q))
		{
			gui.redraw();
			redraw = false;
		}
	}
	while (1);
}
