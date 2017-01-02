#include "handle_game.h"
#include "card.h"
#include "deck.h"
#include <iostream>

#define HANABI_TOTAL_LIGHTNING_INDICATORS	3
#define HANABI_TOTAL_CLUE_INDICATORS		8
#define HANABI_TOTAL_PLAYER_CARDS			6
#define HANABI_TOTAL_CARDS					50

const char* color_string[] = { "blue","green","red","white","yellow","no color" };	//For loading.

using namespace std;

typedef struct
{
	GuiImage* deck;	//id="deck"
	GuiImage* center_cards[HANABI_TOTAL_COLORS][HANABI_TOTAL_NUMBERS];	//id="center_card_%COLOR%_%NUMBER%"
	GuiButton*	discard_card_button;		//id="disc_card_button"
	GuiText*		message;								//id="message"

	struct
	{
		GuiButton* lightning_indicators[3];	//id="light_ind_%ID%"
		GuiButton* clue_indicators[3];		//id="clue_ind_%ID%"
	}indicators;

	struct
	{
		GuiSubmenu	*menu;								//id="give_clue_menu"
		GuiButton *color_button[HANABI_TOTAL_COLORS];	//id="clue_button_%COLOR%"
		GuiButton *number_button[HANABI_TOTAL_NUMBERS];	//id="clue_button_%NUMBERS%""
		GuiButton *give_clue_button;						//id="give_clue_button"
	}give_clue_menu;

	struct
	{
		struct
		{
			GuiText	*name;										//id="local_player_name"
			GuiSubmenu *cards_menu;								//id="local_cards_menu"
			GuiButton *player_card[HANABI_TOTAL_PLAYER_CARDS];	//id="local_card_%NUMBER%"
			GuiButton	*bean[HANABI_TOTAL_PLAYER_CARDS];		//id="bean_%NUMBER%"
		}local;

		struct
		{
			GuiText	*name;										//id="remote_player_name"
			GuiSubmenu *cards_menu;								//id="remote_cards_menu"
			GuiImage *player_card[HANABI_TOTAL_PLAYER_CARDS];	//id="remote_card_%NUMBER%"
		}remote;

	}player;

	struct
	{
		GuiImage	*clue_marker[HANABI_TOTAL_PLAYER_CARDS];		//id="card_ind_%NUMBER%"
		GuiButton	*clue_button;								//id="clue_ok"
	}clue;

	struct
	{
		GuiButton	*open_button;				//id="open_disc_card_menu_button"
		GuiSubmenu	*menu;						//id="discarded_cards_menu"
		GuiImage *cards[HANABI_TOTAL_CARDS];		//id="disc_card_%NUMBER%"
	}discarded_cards;

	struct
	{
		GuiSubmenu	*menu;						//id="game_finished_menu"
		GuiText		*score;						//id="game_finished_menu_score"
		GuiText		*score_message;				//id="game_finished_menu_message"
		GuiButton	*play_again_button;			//id="game_finished_menu_play_again"
		GuiButton	*quit_button;				//id="game_finished_menu_quit"

	}game_finished_menu;

	struct
	{
		GuiSubmenu	*menu;		//id="connection_error_menu"
		GuiButton	*ok_button;	//id="connection_error_menu_ok"
	}connection_lost_menu;

	struct
	{
		GuiSubmenu	*menu;		//id="remote_player_left_menu"
		GuiButton	*ok_button;	//id="remote_player_left_menu_ok"
	}remote_player_left_menu;

	struct
	{
		GuiSubmenu	*menu;			//id="exit_menu"
		GuiButton	*ok_button;		//id="quit_ok"
		GuiButton	*cancel_button;	//id="quit_cancel"
	}exit_menu;

}hanabi_gui_elements_t;

//Attach all UI elements to objects in code
bool attach_menu_elements(hanabi_gui_elements_t *elements, Gui* game_ui)
{
	//Note: The idea of defining elements in an xml and then refering to them by id was inspired by Android UI
	bool success = true;
	//Deck
	success &= (elements->deck = dynamic_cast<GuiImage*>(game_ui->get_element_from_id("deck"))) != nullptr;
	//Center cards
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS && success; c++)
	{
		string id_color = string("center_card_") + string(color_string[c]) + "_";
		for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS && success; n++)
		{
			char number = '0' + n + 1;
			string id = id_color + number;
			success &= (elements->center_cards[c][n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
		}
	}
	//discard_card_button
	if(success)
		success &= (elements->discard_card_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("disc_card_button"))) != nullptr;
	//message
	if (success)
		success &= (elements->message = dynamic_cast<GuiText*>(game_ui->get_element_from_id("message"))) != nullptr;
	//Clue indicators
	for (unsigned int n = 0; n < HANABI_TOTAL_CLUE_INDICATORS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("clue_ind_") + c;
		success &= (elements->indicators.clue_indicators[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Lightning indicators
	for (unsigned int n = 0; n < HANABI_TOTAL_LIGHTNING_INDICATORS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("light_ind_") + c;
		success &= (elements->indicators.lightning_indicators[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Give clue menu
	if (success)
		success &= (elements->give_clue_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("give_clue_menu"))) != nullptr;
	//Give clue button
	if (success)
		success &= (elements->give_clue_menu.give_clue_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("give_clue_button"))) != nullptr;
	//Give clue menu color buttons
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS && success; c++)
	{
		string id = string("clue_button_") + color_string[c];
		success &= (elements->give_clue_menu.color_button[c] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Give clue menu number buttons
	for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("clue_button_") + c;
		success &= (elements->give_clue_menu.color_button[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Local player name
	if (success)
		success &= (elements->player.local.name = dynamic_cast<GuiText*>(game_ui->get_element_from_id("local_player_name"))) != nullptr;
	//Remote player name
		if (success)
			success &= (elements->player.remote.name = dynamic_cast<GuiText*>(game_ui->get_element_from_id("remote_player_name"))) != nullptr;
	//Local player cards menu
		if (success)
			success &= (elements->player.local.cards_menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("local_cards_menu"))) != nullptr;
	//Remote player cards menu
	if (success)
		success &= (elements->player.remote.cards_menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("remote_cards_menu"))) != nullptr;
	//Local player cards
	for (unsigned int n = 0; n < HANABI_TOTAL_PLAYER_CARDS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("local_card_") + c;
		success &= (elements->player.local.player_card[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Remote player cards
	for (unsigned int n = 0; n < HANABI_TOTAL_PLAYER_CARDS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("remote_card_") + c;
		success &= (elements->player.remote.player_card[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Player beans
	for (unsigned int n = 0; n < HANABI_TOTAL_PLAYER_CARDS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("bean_") + c;
		success &= (elements->player.local.bean[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Clue markers
	for (unsigned int n = 0; n < HANABI_TOTAL_PLAYER_CARDS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("card_ind_") + c;
		success &= (elements->clue.clue_marker[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Clue OK Button
	if (success)
		success &= (elements->clue.clue_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("clue_ok"))) != nullptr;
	//Open discarded cards menu button
	if (success)
		success &= (elements->discarded_cards.open_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("open_disc_card_menu_button"))) != nullptr;
	//Discarded cards menu
	if (success)
		success &= (elements->discarded_cards.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("discarded_cards_menu"))) != nullptr;
	//Discarded cards CARDS
	for (unsigned int n = 0; n < HANABI_TOTAL_CARDS && success; n++)
	{
		char number[3];
		itoa(n + 1, number,10);
		string id = string("disc_card_") + number;
		success &= (elements->discarded_cards.cards[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Game finished menu
	if (success)
		success &= (elements->game_finished_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("game_finished_menu"))) != nullptr;
	//Game finished score
	if (success)
		success &= (elements->game_finished_menu.score = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_menu_score"))) != nullptr;
	//Game finished score message
	if (success)
		success &= (elements->game_finished_menu.score_message = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_menu_message"))) != nullptr;
	//Game finished play again button
	if (success)
		success &= (elements->game_finished_menu.play_again_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("game_finished_menu_play_again"))) != nullptr;
	//Game finished quit button
	if (success)
		success &= (elements->game_finished_menu.quit_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("game_finished_menu_quit"))) != nullptr;
	//Connection lost menu
	if (success)
		success &= (elements->connection_lost_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("connection_error_menu"))) != nullptr;
	//Connection lost OK button
	if (success)
		success &= (elements->connection_lost_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("connection_error_menu_ok"))) != nullptr;
	//Remote player left menu
	if (success)
		success &= (elements->remote_player_left_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("remote_player_left_menu"))) != nullptr;
	//Remote player left ok button
	if (success)
		success &= (elements->remote_player_left_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("remote_player_left_menu_ok"))) != nullptr;
	return success;
	//Exit menu
	if (success)
		success &= (elements->exit_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("exit_menu"))) != nullptr;
	//Exit OK button
	if (success)
		success &= (elements->exit_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("quit_ok"))) != nullptr;
	//Exit cancel button
	if (success)
		success &= (elements->exit_menu.cancel_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("quit_cancel"))) != nullptr;
	return success;
}

void handle_game(Gui* game_ui, string user_name, Net_connection* net, bool is_server)
{
	hanabi_gui_elements_t elements;
	if (attach_menu_elements(&elements, game_ui))
	{
		elements.player.local.name->SetText(user_name);
		elements.player.remote.name->SetText("The other player");
		elements.message->SetIsVisible(true);
		elements.message->SetText("Hello World!");
		game_ui->redraw();
		while (1);
	}
	else
		cout << "ERROR: Could not load game UI properly" << endl;
}