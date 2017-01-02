#ifndef HANDLE_MENU_H_
#define HANDLE_MENU_H_

#include "Gui\Gui.h"
#include "Net connection\Net_connection.h"
#include <allegro5\allegro5.h>
#include <string>

using namespace std;

bool handle_menu(Gui* menu, string* name, Net_connection** net, bool* is_server);


#endif	//HANDLE_MENU_H_