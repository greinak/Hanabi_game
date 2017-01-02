#ifndef HANDLE_GAME_H_
#define HANDLE_GAME_H_

#include "Gui\Gui.h"
#include "Net connection\Net_connection.h"

void handle_game(Gui* game_ui, string user_name, Net_connection* net, bool is_server);

#endif		//HANDLE_HAME_H_