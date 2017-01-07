#include <iostream>
#include <fstream>
#include <allegro5\allegro.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\timer.h>
#include "Gui\Gui.h"
#include "Net connection\Net_connection.h"
#include "handle_menu.h"
#include "handle_game.h"

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
			Gui* menu = nullptr;
			menu_gui_data.open("menu_data/connect_menu.xml", std::ifstream::in);
			if (menu_gui_data.is_open() && (menu = new Gui(menu_gui_data)) != nullptr && menu->initialized_successfully())
			{
				menu_gui_data.close();
				string name;
				bool is_server;
				Net_connection* net = nullptr;
				if (handle_menu(menu, &name, &net, &is_server))
				{
					delete menu;
					menu = nullptr;
					Gui* game_ui = nullptr;
					ifstream game_ui_data;
					game_ui_data.open("menu_data/hanabi_game_ui.xml", std::ifstream::in);
					if (game_ui_data.is_open() && (game_ui = new Gui(game_ui_data)) != nullptr && game_ui->initialized_successfully())
					{
						game_ui_data.close();
						handle_game(game_ui, name, net, is_server);
					}
					else
						cout << "Error creating game UI" << endl;
					game_ui_data.close();
					delete game_ui;
				}
				else
					exit = true;
			}
			else
				cout << "Error creating menu UI" << endl;
			menu_gui_data.close();
			delete menu;
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