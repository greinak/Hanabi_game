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
			ifstream menu_gui_data;	//Gui menu UI file
			Gui* menu = nullptr;
			menu_gui_data.open("menu_data/connect_menu.xml", std::ifstream::in);	//Open it
			cout << "[HANABI_MAIN][INFO] : Opening connect menu..." << endl;
			if (menu_gui_data.is_open() && (menu = new Gui(menu_gui_data)) != nullptr && menu->initialized_successfully())	//Parse it
			{
				//Menu should be open now
				menu_gui_data.close();
				string name;
				bool is_server;
				Net_connection* net = nullptr;
				//Handle menu
				if (handle_menu(menu, &name, &net, &is_server))
				{
					delete menu;
					menu = nullptr;
					Gui* game_ui = nullptr;
					cout << "[HANABI_MAIN][INFO] : Opening game.." << endl;
					ifstream game_ui_data;	//Game UI file
					game_ui_data.open("menu_data/hanabi_game_ui.xml", std::ifstream::in);	//Open it
					if (game_ui_data.is_open() && (game_ui = new Gui(game_ui_data)) != nullptr && game_ui->initialized_successfully())	//Parse it
					{
						//Game UI should be open now
						game_ui_data.close();
						//Go to game!
						handle_game(game_ui, name, net, is_server);
					}
					else
					{
						cerr << "[HANABI_MAIN][ERROR] : Cannot create game menu UI!" << endl;
						exit = true;
						game_ui_data.close();
					}
					game_ui_data.close();
					delete game_ui;
				}
				else
					exit = true;
				delete net;
			}
			else
			{
				cerr << "[HANABI_MAIN][ERROR] : Cannot open connect menu!" << endl;
				exit = true;
				menu_gui_data.close();
			}
			menu_gui_data.close();
			delete menu;
		}
		destroy();
	}
	cout << "[HANABI_MAIN][INFO] : EXITING!" << endl;
	return 0;
}


void destroy()
{
	cout << "[HANABI_MAIN][INFO] : Shutting down system!" << endl;
	al_uninstall_mouse();
	al_shutdown_ttf_addon();
	al_shutdown_font_addon();
	al_shutdown_primitives_addon();
	al_shutdown_image_addon();
	al_uninstall_system();
}

bool initialize()
{
	cout << "[HANABI_MAIN][INFO] : Starting system!" << endl;
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
								cout << "[HANABI_MAIN][INFO] : System initialized successfully" << endl;
								return true;
							}
							else
								cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro keyboard." << endl;
							al_uninstall_mouse();
						}
						else
							cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro mouse." << endl;
						al_shutdown_ttf_addon();
					}
					else
						cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro ttf addon." << endl;
					al_shutdown_font_addon();
				}
				else
					cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro font addon." << endl;
				al_shutdown_primitives_addon();
			}
			else
				cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro primitives addon." << endl;
			al_shutdown_image_addon();
		}
		else
			cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro image addon." << endl;
		al_uninstall_system();
	}
	else
		cerr << "[HANABI_MAIN][ERROR] : Could not initialize allegro." << endl;
	return false;
}