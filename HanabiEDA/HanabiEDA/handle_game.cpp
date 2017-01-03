/*
	Handle Game code

*/

#include "handle_game.h"
#include "card.h"
#include "deck.h"
#include <iostream>
#include <allegro5\allegro5.h>
#include "Net connection\Packages\Package_hanabi.h"

#define HANABI_TOTAL_LIGHTNING_INDICATORS	3
#define HANABI_TOTAL_CLUE_INDICATORS		8
#define HANABI_TOTAL_PLAYER_CARDS			6
#define HANABI_TOTAL_CARDS					50

const char* color_string[] = { "blue","green","red","white","yellow","no color" };	//For loading.

using namespace std;

//############
//# Typedefs #
//############

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

typedef struct
{
	hanabi_gui_elements_t elements;
	Net_connection* connection;
	Gui* game_ui;
	ALLEGRO_EVENT_QUEUE* ev_q;
	bool break_event_loop;
	bool redraw;
}game_data;

//FSM events
typedef enum	//Any event may arrive at any state!!
{
	//<--States expected while in initialization-->
	NAME,			//Received name request
	NAMEIS,			//Received name
	START_INFO,		//Received start info
	SW_WHO_I,		//Software decides local starts
	SW_WHO_YOU,		//Software decides remote starts
	I_START,		//Remote says it starts
	YOU_START,		//Remote says local starts
	//<--States expected while in end of game -->
	LOCAL_PA,		//Local player decides to play again
	LOCAL_GO,		//Local player does not want to play again
	REMOTE_PA,		//Remote player decides to play again
	REMOTE_GO,		//Remote player does not want to play again
	//<--States expected while in game-->
	LOCAL_GIVE_CLUE,		//Local player gives clue
	LOCAL_PLAY,				//Local player plays a card
	LOCAL_DISCARD,			//Local player discards a card
	SW_DRAW_NEXT,			//Draw next card (software tells us this card is not the last one)
	SW_DRAW_LAST,			//Draw next card, (software tells us this card is the last one)
	REMOTE_WE_WON,			//Remote player informs we won
	REMOTE_WE_LOST,			//Remote player informs we lost
	REMOTE_MATCH_IS_OVER,	//Remote player informs match is over
	REMOTE_GIVE_CLUE,		//Local player gives clue
	REMOTE_PLAY,			//Local player plays a card
	REMOTE_DISCARD,			//Local player discards a card
	REMOTE_PLAY_WON,		//Local player plays a card, and we won
	REMOTE_PLAY_LOST,		//Local player plays a card, and we lost
	DRAW_NEXT,				//Received draw next card
	DRAW_LAST,				//Received draw next card, this is the last one
	DRAW_FAKE,				//Received empty draw
	//<--Other events-->
	BAD,			//Inform communication error to remote machine
	ERROR_EV,		//Remote machine informed a communication error		
	QUIT,			//Remote left the game
	LOCAL_QUIT,		//Local left the game
	//<--Control events-->
	ACK,			//ACK
	//<--FSM behaviour related events-->
	GND,					//Identifier of end of common event state blocks list. Not a real event.
}fsm_event_T;

typedef enum {MOUSE,DISPLAY_CLOSE,FSM} event_id;

typedef struct
{
	event_id	ev_id;
	fsm_event_T	fsm_event;
	Package_hanabi* package;
}game_event_t;

//#############################
//# Some functions prototypes #
//#############################

//########################
//# Callbacks prototypes #
//########################

static bool attach_menu_elements(hanabi_gui_elements_t *elements, Gui* game_ui);
static void wait_for_event(game_event_t* ret_event, game_data& g_data);

//#################
//# Main function #
//#################
void handle_game(Gui* game_ui, string user_name, Net_connection* net, bool is_server)
{
	game_data data;
	data.connection = net;
	data.game_ui = game_ui;
	data.ev_q = al_create_event_queue();
	data.break_event_loop = false;
	if (data.ev_q != nullptr)
	{
		al_register_event_source(data.ev_q, al_get_mouse_event_source());
		al_register_event_source(data.ev_q, al_get_display_event_source(data.game_ui->get_display()));
		if (attach_menu_elements(&data.elements, game_ui))
		{
			data.elements.player.local.name->SetText(user_name);
			game_ui->redraw();
			game_event_t ev;
			data.redraw = false;
			while (!data.break_event_loop)
			{
				wait_for_event(&ev, data);
				if (ev.ev_id == MOUSE)
				{
					ALLEGRO_MOUSE_STATE st;
					al_get_mouse_state(&st);
					data.redraw |= data.game_ui->feed_mouse_event(st);
				}
				else if (ev.ev_id == DISPLAY_CLOSE)
				{
					data.elements.exit_menu.menu->SetIsVisible(true);
					data.elements.exit_menu.menu->SetIsActive(true);
					data.redraw = true;
				}
				if (data.redraw && al_is_event_queue_empty(data.ev_q))
				{
					data.redraw = false;
					data.game_ui->redraw();
				}
			}
		}
		else
			cout << "ERROR: Could not load game UI properly" << endl;
		al_destroy_event_queue(data.ev_q);
	}
	else
		cout << "Error: Could not create event allegro event queue for game." << endl;
}

//###################
//# Other functions #
//###################
//Wait for an event
static void wait_for_event(game_event_t* ret_event,game_data& g_data)
{
	bool got_event = false;
	char raw_data[MAX_PACKAGE_SIZE];
	size_t data_size = 0;
	while (!got_event)
	{
		if (!al_is_event_queue_empty(g_data.ev_q))
		{
			ALLEGRO_EVENT ev;
			al_wait_for_event(g_data.ev_q, &ev);
			if (ev.any.source == al_get_mouse_event_source())
			{
				if (ev.mouse.display == g_data.game_ui->get_display())
				{
					ret_event->ev_id = MOUSE;
					got_event = true;
				}
			}
			else if (ev.any.source == al_get_display_event_source(g_data.game_ui->get_display()))
			{
				if (ev.display.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
				{
					ret_event->ev_id = DISPLAY_CLOSE;
					got_event = true;
				}
			}
		}
		else if (false || g_data.connection->receive_data(raw_data, MAX_PACKAGE_SIZE,&data_size))
		{
			got_event = true;
			ret_event->ev_id = FSM;
		}
		if (!got_event)
			al_rest(1.0/60.0);	//Just decrease %CPU
	}
}

//Attach all UI elements to objects in code
static bool attach_menu_elements(hanabi_gui_elements_t *elements, Gui* game_ui)
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

//#############
//# Callbacks #
//#############

//#################################
//## HANABI FINITE STATE MACHINE ##
//#################################

//NOTE: Separated in two parts:
//First: States and actions while NOT IN GAME
//Second: States and actions while IN GAME
//Each part is separated in groups, in each group we have actions first ($) and then states(%)
//This organization of states and events DOES NOT AFFECT the behaviour of the FSM, since this is just an abstract organization. 

typedef void(*action)(game_data& data);

typedef struct state_block
{
	fsm_event_T	block_ev;
	action action;
	const struct state_block* next_state;
}state_block;

typedef struct state_block STATE;

//First, let's define a do nothing function
static void do_nothing(game_data& data) { return; }
//And, common fake event
extern const STATE common[];

//This is the golden function
const STATE* fsm_handler(const STATE * current_state, fsm_event_T ev, game_data& data)
{
	const STATE* st = current_state;
	while (st->block_ev != ev && st->block_ev != GND)
		st++;
	if (st->block_ev != GND)
	{
		st->action(data);
		return st->next_state;
	}
	st = common;
	while (st->block_ev != ev && st->block_ev != GND)
		st++;
	if (st->block_ev != GND)
	{
		st->action(data);
		return st->next_state;
	}
	return current_state;
}



//######################
//# Actions prototypes #
//######################
//Notation: state__event
static void s_wait_nameis__nameis(game_data& data);
static void s_wait_name__name(game_data& data);
static void s_wait_nameis_ack__ack(game_data& data);
static void c_wait_name__name(game_data& data);
static void c_wait_nameis_ack__ack(game_data& data);
static void c_wait_nameis__nameis(game_data& data);
static void c_wait_start_info__start_info(game_data& data);
static void wait_start_info_ack__ack(game_data& data);
static void wait_software_who__sw_who_i(game_data& data);
static void wait_software_who__sw_who_you(game_data& data);
static void wait_i_start_ack__ack(game_data& data);
static void wait_who__i_start(game_data& data);
static void wait_who__you_start(game_data& data);
static void a_wait_remote_play_again_answer__remote_pa(game_data& data);
static void a_wait_remote_play_again_answer__remote_go(game_data& data);
static void a_wait_local_play_again_answer__local_pa(game_data& data);
static void a_wait_local_play_again_answer__local_go(game_data& data);
static void b_wait_local_play_again_answer__local_pa(game_data& data);
static void b_wait_local_play_again_answer__local_go(game_data& data);
static void b_wait_remote_play_again_answer__start_info(game_data& data);
static void b_wait_remote_play_again_answer__remote_go(game_data& data);
static void local_player_turn__local_give_clue(game_data& data);
static void local_player_turn__local_play(game_data& data);
static void local_player_turn__local_discard(game_data& data);
static void wait_remote_player_response__ack(game_data& data);
static void wait_remote_player_response__remote_we_won(game_data& data);
static void wait_remote_player_response__remote_we_lost(game_data& data);
static void wait_sw_draw__sw_draw_next(game_data& data);
static void wait_sw_draw__sw_draw_last(game_data& data);
static void remote_player_turn__remote_give_clue(game_data& data);
static void remote_player_turn__remote_play(game_data& data);
static void remote_player_turn__remote_discard(game_data& data);
static void remote_player_turn__remote_play_won(game_data& data);
static void remote_player_turn__remote_play_lost(game_data& data);
static void wait_draw__draw_next(game_data& data);
static void wait_draw__draw_last();
static void sc_1_remote_player_turn__remote_give_clue(game_data& data);
static void sc_1_remote_player_turn__remote_play(game_data& data);
static void sc_1_remote_player_turn__remote_discard(game_data& data);
static void sc_1_remote_player_turn__remote_play_won(game_data& data);
static void sc_1_remote_player_turn__remote_play_lost(game_data& data);
static void sc_1_wait_draw__draw_fake(game_data& data);
static void sc_1_local_player_turn__local_give_clue(game_data& data);
static void sc_1_local_player_turn__local_play(game_data& data);
static void sc_1_local_player_turn__local_discard(game_data& data);
static void sc_1_wait_remote_player_response__remote_we_won(game_data& data);
static void sc_1_wait_remote_player_response__remote_we_lost(game_data& data);
static void sc_1_wait_remote_player_response__remote_match_is_over(game_data& data);
static void sc_2_local_player_turn__local_give_clue(game_data& data);
static void sc_2_local_player_turn__local_play(game_data& data);
static void sc_2_local_player_turn__local_discard(game_data& data);
static void sc_2_wait_remote_player_response__ack(game_data& data);
static void sc_2_wait_remote_player_response__remote_we_won(game_data& data);
static void sc_2_wait_remote_player_response__remote_we_lost(game_data& data);
static void sc_2_remote_player_turn__remote_give_clue(game_data& data);
static void sc_2_remote_player_turn__remote_discard(game_data& data);
static void sc_2_remote_player_turn__remote_play(game_data& data);
static void sc_2_remote_player_turn__remote_play(game_data& data);
static void sc_2_remote_player_turn__remote_play_won(game_data& data);
static void sc_2_remote_player_turn__remote_play_lost(game_data& data);

//#####################
//# States prototypes #
//#####################
//Notation: state__event

extern const STATE s_wait_nameis[];
extern const STATE s_wait_name[];
extern const STATE s_wait_nameis_ack[];
extern const STATE c_wait_name[];
extern const STATE c_wait_nameis_ack[];
extern const STATE c_wait_nameis[];
extern const STATE c_wait_start_info[];
extern const STATE wait_start_info_ack[];
extern const STATE wait_software_who[];
extern const STATE wait_i_start_ack[];
extern const STATE wait_who[];
extern const STATE a_wait_remote_play_again_answer[];
extern const STATE a_wait_local_play_again_answer[];
extern const STATE b_wait_local_play_again_answer[];
extern const STATE b_wait_remote_play_again_answer[];
extern const STATE local_player_turn[];
extern const STATE wait_remote_player_response[];
extern const STATE wait_sw_draw[];
extern const STATE remote_player_turn[];
extern const STATE wait_draw[];
extern const STATE sc_1_remote_player_turn[];
extern const STATE sc_1_wait_draw[];
extern const STATE sc_1_local_player_turn[];
extern const STATE sc_1_wait_remote_player_response[];
extern const STATE sc_2_local_player_turn[];
extern const STATE sc_2_wait_remote_player_response[];
extern const STATE sc_2_remote_player_turn[];


//<-- While NOT in game!!! -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: HANDSHAKE, branch: Server $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void s_wait_nameis__nameis(game_data& data)
{
	//Send ack
}
static void s_wait_name__name(game_data& data)
{
	//Send nameis
}
static void s_wait_nameis_ack__ack(game_data& data)
{
	//Send start info
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: HANDSHAKE, branch: Server %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

static const STATE s_wait_nameis[] =
{
	{ NAMEIS,s_wait_nameis__nameis,s_wait_name },
	{ GND,NULL,NULL}
};

static const STATE s_wait_name[] =
{
	{ NAME,s_wait_name__name,s_wait_nameis_ack },
	{ GND,do_nothing,NULL}
};

static const STATE s_wait_nameis_ack[] =
{
	{ ACK,s_wait_nameis_ack__ack,wait_start_info_ack },  // --> GO TO INITIALIZATION
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: HANDSHAKE, branch: Client $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void c_wait_name__name(game_data& data)
{
	//Send nameis
}
static void c_wait_nameis_ack__ack(game_data& data)
{
	//Send name
}
static void c_wait_nameis__nameis(game_data& data)
{
	//Send ack
}
static void c_wait_start_info__start_info(game_data& data)
{
	//Send ack
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: HANDSHAKE, branch: Client %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE c_wait_name[] =
{
	{ NAME,c_wait_name__name,c_wait_nameis_ack },
	{ GND,nullptr,nullptr }
};
static const STATE c_wait_nameis_ack[] =
{
	{ ACK,c_wait_nameis_ack__ack,c_wait_nameis },
	{ GND,nullptr,nullptr }
};
static const STATE c_wait_nameis[] =
{
	{ NAMEIS,c_wait_nameis__nameis,c_wait_start_info },
	{ GND,nullptr,nullptr }
};
static const STATE c_wait_start_info[] =
{
	{ START_INFO,c_wait_start_info__start_info,wait_who },	// --> GO TO INITIALIZATION
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: INITIALIZATION, branch: MACHINE_A $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void wait_start_info_ack__ack(game_data& data)
{
	//Send 
}
static void wait_software_who__sw_who_i(game_data& data)
{
	//Send 
}
static void wait_software_who__sw_who_you(game_data& data)
{
	//Send 
}
static void wait_i_start_ack__ack(game_data& data)
{
	//Send 
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: INITIALIZATION, branch: MACHINE_A %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE wait_start_info_ack[] =
{
	{ ACK,wait_start_info_ack__ack,wait_software_who },
	{ GND,nullptr,nullptr }
};
static const STATE wait_software_who[] =
{
	{ SW_WHO_I,wait_software_who__sw_who_i,wait_i_start_ack },
	{ SW_WHO_YOU,wait_software_who__sw_who_you,remote_player_turn },	//  --> GO TO GAME
	{ GND,nullptr,nullptr }
};
static const STATE wait_i_start_ack[] =
{
	{ ACK,wait_i_start_ack__ack,local_player_turn },	// --> GO TO GAME
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: INITIALIZATION, branch: MACHINE_B $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void wait_who__i_start(game_data& data)
{
	//Send 
}
static void wait_who__you_start(game_data& data)
{
	//Send 
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: INITIALIZATION, branch: MACHINE_B %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE wait_who[] =
{
	{ I_START,wait_who__i_start,remote_player_turn },
	{ YOU_START,wait_who__you_start,local_player_turn },	// --> GO TO GAME
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: END OF GAME, branch: A:INFORMER (The one who informed game result) $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void a_wait_remote_play_again_answer__remote_pa(game_data& data)
{
}
static void a_wait_remote_play_again_answer__remote_go(game_data& data)
{
}
static void a_wait_local_play_again_answer__local_pa(game_data& data)
{
}
static void a_wait_local_play_again_answer__local_go(game_data& data)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: END OF GAME, branch: A:INFORMER (The one who informed game result) %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE a_wait_remote_play_again_answer[] =
{
	{ REMOTE_PA,a_wait_remote_play_again_answer__remote_pa,a_wait_local_play_again_answer },
	{ REMOTE_GO,a_wait_remote_play_again_answer__remote_go,NULL },	//break
	{ GND,nullptr,nullptr }
};
static const STATE a_wait_local_play_again_answer[] =
{
	{ LOCAL_PA,a_wait_local_play_again_answer__local_pa,wait_start_info_ack },		//  -->> GO TO INITIALIZATION
	{ LOCAL_GO,a_wait_local_play_again_answer__local_go,NULL },		//break
	{ GND,nullptr,nullptr }
};


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: END OF GAME, branch: B:INFORMED (The one who received information about game result) $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void b_wait_local_play_again_answer__local_pa(game_data& data)
{
}
static void b_wait_local_play_again_answer__local_go(game_data& data)
{
}
static void b_wait_remote_play_again_answer__start_info(game_data& data)
{
}
static void b_wait_remote_play_again_answer__remote_go(game_data& data)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: END OF GAME, branch: B:INFORMED (The one who received information about game result) %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE b_wait_local_play_again_answer[] =
{
	{ LOCAL_PA,b_wait_local_play_again_answer__local_pa,b_wait_remote_play_again_answer },
	{ LOCAL_GO,b_wait_local_play_again_answer__local_go,NULL }, //break
	{ GND,nullptr,nullptr }
};
static const STATE b_wait_remote_play_again_answer[] =
{
	{ START_INFO,b_wait_remote_play_again_answer__start_info,wait_who },		//  --> GO TO INITIALIZATION
	{ REMOTE_GO,b_wait_remote_play_again_answer__remote_go,NULL }, //break
	{ GND,nullptr,nullptr }
};

//<-- While in game -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: IN GAME, branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void local_player_turn__local_give_clue(game_data& data)
{
}
static void local_player_turn__local_play(game_data& data)
{
}
static void local_player_turn__local_discard(game_data& data)
{
}
static void wait_remote_player_response__ack(game_data& data)
{
}
static void wait_remote_player_response__remote_we_won(game_data& data)
{
}
static void wait_remote_player_response__remote_we_lost(game_data& data)
{
}
static void wait_sw_draw__sw_draw_next(game_data& data)
{
}
static void wait_sw_draw__sw_draw_last(game_data& data)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: IN GAME, branch: LOCAL_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE local_player_turn[] =
{
	{ LOCAL_GIVE_CLUE,local_player_turn__local_give_clue,remote_player_turn },
	{ LOCAL_PLAY,local_player_turn__local_play,wait_remote_player_response },
	{ LOCAL_DISCARD,local_player_turn__local_discard,wait_remote_player_response },
	{ GND,nullptr,nullptr },
};
static const STATE wait_remote_player_response[] =
{
	{ ACK,wait_remote_player_response__ack,wait_sw_draw },
	{ REMOTE_WE_WON,wait_remote_player_response__remote_we_won,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_WE_LOST,wait_remote_player_response__remote_we_lost,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ GND,nullptr,nullptr }
};
static const STATE wait_sw_draw[] =
{
	{ SW_DRAW_NEXT,wait_sw_draw__sw_draw_next,remote_player_turn },
	{ SW_DRAW_LAST,wait_sw_draw__sw_draw_last,sc_1_remote_player_turn },	//	--> GO TO GAME FINISHING SCENARIO 1
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: IN GAME, branch: REMOTE_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void remote_player_turn__remote_give_clue(game_data& data)
{
}
static void remote_player_turn__remote_play(game_data& data)
{
}
static void remote_player_turn__remote_discard(game_data& data)
{
}
static void remote_player_turn__remote_play_won(game_data& data)
{
}
static void remote_player_turn__remote_play_lost(game_data& data)
{
}
static void wait_draw__draw_next(game_data& data)
{
}
static void wait_draw__draw_last(game_data& data)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: IN GAME, branch: REMOTE_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE remote_player_turn[] =
{
	{ REMOTE_GIVE_CLUE,remote_player_turn__remote_give_clue,local_player_turn },
	{ REMOTE_PLAY,remote_player_turn__remote_play,wait_draw },
	{ REMOTE_DISCARD,remote_player_turn__remote_discard,wait_draw },
	{ REMOTE_PLAY_WON,remote_player_turn__remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_PLAY_LOST,remote_player_turn__remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ GND,nullptr,nullptr }
};
static const STATE wait_draw[] =
{
	{ DRAW_NEXT,wait_draw__draw_next,local_player_turn },
	{ DRAW_LAST,wait_draw__draw_last,sc_2_local_player_turn },	//	--> GO TO GAME FINISHING SCENARIO 2
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FIRST), branch: REMOTE_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void sc_1_remote_player_turn__remote_give_clue(game_data& data)
{
}
static void sc_1_remote_player_turn__remote_play(game_data& data)
{
}
static void sc_1_remote_player_turn__remote_discard(game_data& data)
{
}
static void sc_1_remote_player_turn__remote_play_won(game_data& data)
{
}
static void sc_1_remote_player_turn__remote_play_lost(game_data& data)
{
}
static void sc_1_wait_draw__draw_fake(game_data& data)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FIRST), branch: REMOTE_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE sc_1_remote_player_turn[] =
{
	{ REMOTE_GIVE_CLUE,sc_1_remote_player_turn__remote_give_clue,sc_1_local_player_turn },
	{ REMOTE_PLAY,sc_1_remote_player_turn__remote_play,sc_1_wait_draw },
	{ REMOTE_DISCARD,sc_1_remote_player_turn__remote_discard,sc_1_wait_draw },
	{ REMOTE_PLAY_WON,sc_1_remote_player_turn__remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_PLAY_LOST,sc_1_remote_player_turn__remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
};
static const STATE sc_1_wait_draw[] =
{
	{ DRAW_FAKE,sc_1_wait_draw__draw_fake,sc_1_local_player_turn },
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 1, (LAST 2 TURNS, REMOTE FIRST) branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void sc_1_local_player_turn__local_give_clue(game_data& data)
{
}
static void sc_1_local_player_turn__local_play(game_data& data)
{
}
static void sc_1_local_player_turn__local_discard(game_data& data)
{
}
static void sc_1_wait_remote_player_response__remote_we_won(game_data& data)
{
}
static void sc_1_wait_remote_player_response__remote_we_lost(game_data& data)
{
}
static void sc_1_wait_remote_player_response__remote_match_is_over(game_data& data)
{
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FISRT), branch: LOCAL_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE sc_1_local_player_turn[] =
{
	{ LOCAL_GIVE_CLUE,sc_1_local_player_turn__local_give_clue,sc_1_wait_remote_player_response },
	{ LOCAL_PLAY,sc_1_local_player_turn__local_play,sc_1_wait_remote_player_response },
	{ LOCAL_DISCARD,sc_1_local_player_turn__local_discard,sc_1_wait_remote_player_response },
};
static const STATE sc_1_wait_remote_player_response[] =
{
	{ REMOTE_WE_WON,sc_1_wait_remote_player_response__remote_we_won,b_wait_local_play_again_answer },				//  --> GO TO END OF GAME
	{ REMOTE_WE_LOST,sc_1_wait_remote_player_response__remote_we_lost,b_wait_local_play_again_answer },				//  --> GO TO END OF GAME
	{ REMOTE_MATCH_IS_OVER,sc_1_wait_remote_player_response__remote_match_is_over,b_wait_local_play_again_answer },	//  --> GO TO END OF GAME
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void sc_2_local_player_turn__local_give_clue(game_data& data)
{
}
static void sc_2_local_player_turn__local_play(game_data& data)
{
}
static void sc_2_local_player_turn__local_discard(game_data& data)
{
}
static void sc_2_wait_remote_player_response__ack(game_data& data)
{
}
static void sc_2_wait_remote_player_response__remote_we_won(game_data& data)
{
}
static void sc_2_wait_remote_player_response__remote_we_lost(game_data& data)
{
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: LOCAL_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE sc_2_local_player_turn[] =
{
	{ LOCAL_GIVE_CLUE,sc_2_local_player_turn__local_give_clue,sc_2_remote_player_turn },
	{ LOCAL_PLAY,sc_2_local_player_turn__local_play,sc_2_wait_remote_player_response },
	{ LOCAL_DISCARD,sc_2_local_player_turn__local_discard,sc_2_wait_remote_player_response },
};
static const STATE sc_2_wait_remote_player_response[] =
{
	{ ACK,sc_2_wait_remote_player_response__ack,sc_2_remote_player_turn },
	{ REMOTE_WE_WON,sc_2_wait_remote_player_response__remote_we_won,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_WE_LOST,sc_2_wait_remote_player_response__remote_we_lost,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
};
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: REMOTE_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void sc_2_remote_player_turn__remote_give_clue(game_data& data)
{
}
static void sc_2_remote_player_turn__remote_discard(game_data& data)
{
}
static void sc_2_remote_player_turn__remote_play(game_data& data)
{
}
static void sc_2_remote_player_turn__remote_play_won(game_data& data)
{
}
static void sc_2_remote_player_turn__remote_play_lost(game_data& data)
{
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: REMOTE_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE sc_2_remote_player_turn[] =
{
	{ REMOTE_GIVE_CLUE,sc_2_remote_player_turn__remote_give_clue,a_wait_remote_play_again_answer },		//	--> GO TO END OF GAME
	{ REMOTE_DISCARD,sc_2_remote_player_turn__remote_discard,a_wait_remote_play_again_answer },			//	--> GO TO END OF GAME	
	{ REMOTE_PLAY,sc_2_remote_player_turn__remote_play,a_wait_remote_play_again_answer },				//	--> GO TO END OF GAME
	{ REMOTE_PLAY_WON,sc_2_remote_player_turn__remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_PLAY_LOST,sc_2_remote_player_turn__remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
};

//#################################
//# Actions common to all states! #
//#################################
static void dummy(game_data& data)
{
};

//This state is actually part of all states.
static const STATE common[] =
{
	{ ERROR_EV,dummy,nullptr },
	{ GND,nullptr,nullptr }
};