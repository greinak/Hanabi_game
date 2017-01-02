#include <iostream>
#include <fstream>
#include <allegro5\allegro.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include "Gui\Gui.h"
#include "Net connection\Net_connection.h"
#include "handle_menu.h"

using namespace std;

bool initialize();
void destroy();

int main(int argc, char* argv[])
{
	if (initialize())
	{
		bool exit = false;
		while (!exit)
		{
			ifstream menu_gui_data;
			menu_gui_data.open("menu_data/connect_menu.xml", std::ifstream::in);
			if (menu_gui_data.is_open())
			{
				Gui* menu;
				if ((menu = new Gui(menu_gui_data)) != nullptr && menu->initialized_successfully())
				{
					menu_gui_data.close();
					string name;
					bool is_server;
					Net_connection* net;
					if (handle_menu(menu, &name, &net, &is_server))
					{

					}
					else
						exit = true;
					delete menu;
				}
				else
				{
					cerr << "Error: Could not create menu GUI." << endl;
					break;
				}
			}
			else
			{
				cerr << "Error: Could not open menu game ui data." << endl;
				break;
			}

		}
		
		
		destroy();
	}
	cout << "EXITING!!" << endl;
	return 0;
}




void destroy()
{
	al_uninstall_mouse();
	al_shutdown_ttf_addon();
	al_shutdown_font_addon();
	al_shutdown_primitives_addon();
	al_shutdown_image_addon();
	al_uninstall_system();
}

bool initialize()
{
	srand(time(NULL));
	if (al_init())
	{
		if (al_init_image_addon())
		{
			if (al_init_primitives_addon())
			{
				if (al_init_font_addon())
				{
					if (al_init_ttf_addon())
					{
						if (al_install_mouse())
						{
							if (al_install_keyboard())
							{
								cout << "Allegro system initialized successfully" << endl;
								return true;
							}
							else
								cerr << "ERROR: Could not initialize allegro keyboard." << endl;
							al_uninstall_mouse();
						}
						else
							cerr << "ERROR: Could not initialize allegro mouse." << endl;
						al_shutdown_ttf_addon();
					}
					else
						cerr << "ERROR: Could not initialize allegro ttf addon." << endl;
					al_shutdown_font_addon();
				}
				else
					cerr << "ERROR: Could not initialize allegro font addon." << endl;
				al_shutdown_primitives_addon();
			}
			else
				cerr << "ERROR: Could not initialize allegro primitives addon." << endl;
			al_shutdown_image_addon();
		}
		else
			cerr << "ERROR: Could not initialize allegro image addon." << endl;
		al_uninstall_system();
	}
	else
		cerr << "ERROR: Could not initialize allegro." << endl;
	return false;
}