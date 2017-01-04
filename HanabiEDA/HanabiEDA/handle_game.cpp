/*
	Handle Game code
*/

#include "handle_game.h"
#include "card.h"
#include "deck.h"
#include <iostream>
#include <allegro5\allegro5.h>
#include "Net connection\Packages\Packages.h"
#include "card_skin.h"
#include <queue>
#include <algorithm>

#define HANABI_TOTAL_LIGHTNING_INDICATORS	3
#define HANABI_TOTAL_CLUE_INDICATORS		8

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
	GuiText*		message2;								//id="message"

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
			GuiButton *player_card[HANABI_HAND_SIZE];	//id="local_card_%NUMBER%"
			GuiButton	*bean[HANABI_HAND_SIZE];		//id="bean_%NUMBER%"
		}local;

		struct
		{
			GuiText	*name;										//id="remote_player_name"
			GuiSubmenu *cards_menu;								//id="remote_cards_menu"
			GuiImage *player_card[HANABI_HAND_SIZE];	//id="remote_card_%NUMBER%"
		}remote;

	}player;

	struct
	{
		GuiImage	*clue_marker[HANABI_HAND_SIZE];		//id="card_ind_%NUMBER%"
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

//FSM events

typedef enum
{
	FB_NO_EVENT,
	FB_WHO,
	FB_ERROR
}fsm_feedback_event_T;

typedef enum
{
	USER_DISCARD_CARD,
	USER_PLAY_CARD
}local_user_event_T;

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
	//<--Initial events-->
	SERVER,			//Start FSM as server
	CLIENT,			//Start FSM as client
	//<--FSM behaviour related events-->
	GND,					//Identifier of end of common event state blocks list. Not a real event.
}fsm_event_T;

typedef struct
{
	hanabi_gui_elements_t elements;
	Net_connection* connection;
	Gui* game_ui;
	ALLEGRO_EVENT_QUEUE* ev_q;
	string user_name;
	bool break_event_loop;
	bool redraw;
	fsm_feedback_event_T feedback_event;
	deck card_deck;
	card local_player_card[HANABI_HAND_SIZE];
	card remote_player_card[HANABI_HAND_SIZE];
	card_skin skin;
	unsigned int clues;
	unsigned int lightnings;
	unsigned int color_stack[HANABI_TOTAL_COLORS];
	bool discard_mode_enabled;
	unsigned int local_event_card_offset;
	bool local_event_clue_is_color;
	unsigned int local_event_clue_color_id;
	unsigned int local_event_clue_number_id;
	queue<local_user_event_T> local_event_queue;
	unsigned int discard_count;
}game_data;

typedef enum {MOUSE,DISPLAY_CLOSE,FSM} event_id;

typedef struct
{
	event_id	ev_id;
	fsm_event_T	fsm_event;
	Package_hanabi* package;
}game_event_t;

//Typedefs
typedef void(*action)(game_data& data, Package_hanabi* package);

typedef struct state_block
{
	fsm_event_T	block_ev;
	action action;
	const struct state_block* next_state;
}state_block;

typedef struct state_block STATE;

//FSM HANDLER
const STATE* fsm_handler(const STATE * current_state, fsm_event_T ev, game_data& data, Package_hanabi* package);
//FSM initial state
extern const STATE fsm_start_point[];

//#############################
//# Some functions prototypes #
//#############################

static void initialize_deck(deck& card_deck);
static bool attach_menu_elements(hanabi_gui_elements_t &elements, Gui* game_ui);
static void attach_callbacks_to_elements(hanabi_gui_elements_t &elements, game_data& data);
static void wait_for_event(game_event_t* ret_event, game_data& g_data);

//########################
//# Callbacks prototypes #
//########################
bool bean_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool discarded_cards_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool generic_close_menu_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool discard_card_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool local_user_card_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);

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
	data.user_name = user_name;
	data.feedback_event = FB_NO_EVENT;
	if (data.skin.initialized_successfully())
	{
		if (data.ev_q != nullptr)
		{
			al_register_event_source(data.ev_q, al_get_mouse_event_source());
			al_register_event_source(data.ev_q, al_get_display_event_source(data.game_ui->get_display()));
			if (attach_menu_elements(data.elements, game_ui))
			{
				attach_callbacks_to_elements(data.elements,data);
				data.elements.player.local.name->SetText(data.user_name);
				data.elements.player.local.name->SetIsVisible(true);
				data.elements.message->SetText("Handshake... Please wait...");
				data.elements.message->SetIsVisible(true);
				data.game_ui->redraw();
				data.elements.discarded_cards.open_button->SetIsActive(true);
				data.elements.discarded_cards.open_button->SetIsVisible(true);
				const STATE* state = fsm_start_point;
				if (is_server)
					state = fsm_handler(state, SERVER, data, nullptr);
				else
					state = fsm_handler(state, CLIENT, data, nullptr);
				game_event_t ev;
				while (!data.break_event_loop)
				{
					ev.package = nullptr;		//Always to nullptr if no data
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
					else if (ev.ev_id == FSM)
						state = fsm_handler(state, ev.fsm_event, data, ev.package);
					if (data.redraw && al_is_event_queue_empty(data.ev_q))
					{
						data.redraw = false;
						data.game_ui->redraw();
					}
					delete ev.package;	//delete, if any data, no nullptr, else, nullptr, so OK!
				}
			}
			else
				cout << "ERROR: Could not load game UI properly" << endl;
			al_destroy_event_queue(data.ev_q);
		}
		else
			cout << "Error: Could not create event allegro event queue for game." << endl;
	}
	else
		cout << "Error: Could not load cards skin" << endl;
}

//###################
//# Other functions #
//###################
//Wait for an event

static void wait_for_event(game_event_t* ret_event,game_data& g_data)
{
	bool got_event = false;
	char raw_data[MAX_PACKAGE_SIZE];
	bool connection_ok;
	size_t data_size = 0;

	ret_event->package = nullptr; //If no data, always to nullptr
	ret_event->fsm_event = GND;

	while (!got_event)
	{
		if (g_data.feedback_event != FB_NO_EVENT)
		{
			switch (g_data.feedback_event)
			{
			case FB_WHO:
			{
				ret_event->ev_id = FSM;
				int rand_value = rand();
				//cout << rand_value << endl;	//For debugging
				ret_event->fsm_event = (rand_value%2)?SW_WHO_I:SW_WHO_YOU;	//Pseudorandomly select who starts.
				ret_event->package = nullptr;		//If no data, always to nullptr
				got_event = true;
				break;
			}
			case FB_ERROR:
			default:
				got_event = true;
				ret_event->ev_id = FSM;
				ret_event->fsm_event = ERROR_EV;	//FATAL ERROR.
				ret_event->package = nullptr;		//If no data, always to nullptr
			}
			g_data.feedback_event = FB_NO_EVENT;
		}
		else if (!got_event)
		{
			if (!g_data.local_event_queue.empty())
			{
				local_user_event_T ev = g_data.local_event_queue.front();
				g_data.local_event_queue.pop();
				got_event = true;
				ret_event->ev_id = FSM;
				ret_event->package = nullptr;
				switch (ev)
				{
				case USER_PLAY_CARD:
					ret_event->fsm_event = LOCAL_PLAY;
					break;
				case USER_DISCARD_CARD:
					ret_event->fsm_event = LOCAL_DISCARD;
					break;
				default:
					ret_event->fsm_event = BAD;
					break;
				}
			}
			else if (!got_event && !al_is_event_queue_empty(g_data.ev_q))
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
			else if (!got_event)
			{
				connection_ok = g_data.connection->receive_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
				if (connection_ok)
				{
					if (data_size != 0)
					{
						ret_event->ev_id = FSM;
						bool data_ok = true;
						got_event = true;
						switch (get_package_type_from_raw_data(raw_data, data_size))
						{
							case ACK_P:
							{
								Package_ack* package = new Package_ack;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))

									ret_event->fsm_event = ACK;
								else
									data_ok = false;
								break;
							}
							case NAME_P:
							{
								Package_name* package = new Package_name;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = NAME;
								else
									data_ok = false;
								break;
							}
							case NAME_IS_P:
							{
								Package_name_is* package = new Package_name_is;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = NAMEIS;
								else
									data_ok = false;
								break;
							}
							case START_INFO_P:
							{
								Package_start_info* package = new Package_start_info;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								{
									bool start_info_ok = true;
									card hand_1[HANABI_HAND_SIZE];
									card hand_2[HANABI_HAND_SIZE];
									deck start_info_deck;	//Create deck in order to check if start info is valid.
									package->get_info(hand_1, hand_2);
									initialize_deck(start_info_deck);
									for (unsigned int i = 0; i < HANABI_HAND_SIZE && start_info_ok; i++)
									{
										start_info_ok &= start_info_deck.remove_card(hand_1[i]);
										start_info_ok &= start_info_deck.remove_card(hand_2[i]);
									}
									if (start_info_ok)
										ret_event->fsm_event = START_INFO;
									else
										data_ok = false;
								}
								else
									data_ok = false;
								break;
							}
							case YOU_START_P:
							{
								Package_you_start* package = new Package_you_start;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = YOU_START;
								else
									data_ok = false;
								break;
							}
							case I_START_P:
							{
								Package_i_start* package = new Package_i_start;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = I_START;
								else
									data_ok = false;
								break;
							}
							case PLAY_P:
							{
								Package_play* package = new Package_play;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_PLAY;
								else
									data_ok = false;
								break;
							}
							case DISCARD_P:
							{
								Package_discard* package = new Package_discard;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_DISCARD;
								else
									data_ok = false;
								break;
							}
							case YOU_HAVE_P:
							{
								Package_you_have* package = new Package_you_have;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_GIVE_CLUE;
								else
									data_ok = false;
								break;
							}
							case DRAW_P:
							{
								Package_draw* package = new Package_draw;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								{
									card c;
									package->get_card(&c);
									if (g_data.card_deck.count_cards(c) != 0)
										ret_event->fsm_event = REMOTE_DISCARD;
									else
										data_ok = false;
								}
								else
									data_ok = false;
								break;
							}
							case WE_WON_P:
							{
								Package_we_won* package = new Package_we_won;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_WE_WON;
								else
									data_ok = false;
								break;
							}
							case WE_LOST_P:
							{
								Package_we_lost* package = new Package_we_lost;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_WE_LOST;
								else
									data_ok = false;
								break;
							}
							case MATCH_IS_OVER_P:
							{
								Package_match_is_over* package = new Package_match_is_over;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_MATCH_IS_OVER;
								else
									data_ok = false;
								break;
							}
							case PLAY_AGAIN_P:
							{
								Package_play_again* package = new Package_play_again;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_PA;
								else
									data_ok = false;
								break;
							}
							case GAME_OVER_P:
							{
								Package_game_over* package = new Package_game_over;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = REMOTE_GO;
								else
									data_ok = false;
								break;
							}
							case QUIT_P:
							{
								Package_quit* package = new Package_quit;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = QUIT;
								else
									data_ok = false;
								break;
							}
							case ERROR_P:
							{
								Package_error* package = new Package_error;
								if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
									ret_event->fsm_event = QUIT;
								else
									data_ok = false;
								break;
							}
							default:
							{
								data_ok = false;
								break;
							}
						}
						if (!data_ok)
						{
							delete ret_event->package;		//Free package data!!
							ret_event->fsm_event = BAD;		//BAD, since must inform remote!
							ret_event->package = nullptr;	//If no data, always to nullptr
						}
					}
					else
					{
						//No data received :)
					}
				}
				else
				{
					got_event = true;
					ret_event->ev_id = FSM;
					ret_event->fsm_event = ERROR_EV;	//FATAL CONNECTION ERROR.
					ret_event->package = nullptr;		//If no data, always to nullptr
				}
			}
		}
		if (!got_event)
			al_rest(1.0/60.0);	//Just decrease %CPU
	}
}

//Attach all UI elements to objects in code
static bool attach_menu_elements(hanabi_gui_elements_t &elements, Gui* game_ui)
{
	//Note: The idea of defining elements in an xml and then refering to them by id was inspired by Android UI
	bool success = true;
	//Deck
	success &= (elements.deck = dynamic_cast<GuiImage*>(game_ui->get_element_from_id("deck"))) != nullptr;
	//Center cards
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS && success; c++)
	{
		string id_color = string("center_card_") + string(color_string[c]) + "_";
		for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS && success; n++)
		{
			char number = '0' + n + 1;
			string id = id_color + number;
			success &= (elements.center_cards[c][n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
		}
	}
	//discard_card_button
	if(success)
		success &= (elements.discard_card_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("disc_card_button"))) != nullptr;
	//message
	if (success)
		success &= (elements.message = dynamic_cast<GuiText*>(game_ui->get_element_from_id("message"))) != nullptr;
	//message2
	if (success)
		success &= (elements.message2 = dynamic_cast<GuiText*>(game_ui->get_element_from_id("message2"))) != nullptr;
	//Clue indicators
	for (unsigned int n = 0; n < HANABI_TOTAL_CLUE_INDICATORS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("clue_ind_") + c;
		success &= (elements.indicators.clue_indicators[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Lightning indicators
	for (unsigned int n = 0; n < HANABI_TOTAL_LIGHTNING_INDICATORS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("light_ind_") + c;
		success &= (elements.indicators.lightning_indicators[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Give clue menu
	if (success)
		success &= (elements.give_clue_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("give_clue_menu"))) != nullptr;
	//Give clue button
	if (success)
		success &= (elements.give_clue_menu.give_clue_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("give_clue_button"))) != nullptr;
	//Give clue menu color buttons
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS && success; c++)
	{
		string id = string("clue_button_") + color_string[c];
		success &= (elements.give_clue_menu.color_button[c] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Give clue menu number buttons
	for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("clue_button_") + c;
		success &= (elements.give_clue_menu.color_button[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Local player name
	if (success)
		success &= (elements.player.local.name = dynamic_cast<GuiText*>(game_ui->get_element_from_id("local_player_name"))) != nullptr;
	//Remote player name
		if (success)
			success &= (elements.player.remote.name = dynamic_cast<GuiText*>(game_ui->get_element_from_id("remote_player_name"))) != nullptr;
	//Local player cards menu
		if (success)
			success &= (elements.player.local.cards_menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("local_cards_menu"))) != nullptr;
	//Remote player cards menu
	if (success)
		success &= (elements.player.remote.cards_menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("remote_cards_menu"))) != nullptr;
	//Local player cards
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("local_card_") + c;
		success &= (elements.player.local.player_card[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Remote player cards
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("remote_card_") + c;
		success &= (elements.player.remote.player_card[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Player beans
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("bean_") + c;
		success &= (elements.player.local.bean[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Clue markers
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("card_ind_") + c;
		success &= (elements.clue.clue_marker[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Clue OK Button
	if (success)
		success &= (elements.clue.clue_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("clue_ok"))) != nullptr;
	//Open discarded cards menu button
	if (success)
		success &= (elements.discarded_cards.open_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("open_disc_card_menu_button"))) != nullptr;
	//Discarded cards menu
	if (success)
		success &= (elements.discarded_cards.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("discarded_cards_menu"))) != nullptr;
	//Discarded cards CARDS
	for (unsigned int n = 0; n < HANABI_TOTAL_CARDS && success; n++)
	{
		char number[3];
		itoa(n + 1, number,10);
		string id = string("disc_card_") + number;
		success &= (elements.discarded_cards.cards[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Game finished menu
	if (success)
		success &= (elements.game_finished_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("game_finished_menu"))) != nullptr;
	//Game finished score
	if (success)
		success &= (elements.game_finished_menu.score = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_menu_score"))) != nullptr;
	//Game finished score message
	if (success)
		success &= (elements.game_finished_menu.score_message = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_menu_message"))) != nullptr;
	//Game finished play again button
	if (success)
		success &= (elements.game_finished_menu.play_again_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("game_finished_menu_play_again"))) != nullptr;
	//Game finished quit button
	if (success)
		success &= (elements.game_finished_menu.quit_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("game_finished_menu_quit"))) != nullptr;
	//Connection lost menu
	if (success)
		success &= (elements.connection_lost_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("connection_error_menu"))) != nullptr;
	//Connection lost OK button
	if (success)
		success &= (elements.connection_lost_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("connection_error_menu_ok"))) != nullptr;
	//Remote player left menu
	if (success)
		success &= (elements.remote_player_left_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("remote_player_left_menu"))) != nullptr;
	//Remote player left ok button
	if (success)
		success &= (elements.remote_player_left_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("remote_player_left_menu_ok"))) != nullptr;
	//Exit menu
	if (success)
		success &= (elements.exit_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("exit_menu"))) != nullptr;
	//Exit OK button
	if (success)
		success &= (elements.exit_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("quit_ok"))) != nullptr;
	//Exit cancel button
	if (success)
		success &= (elements.exit_menu.cancel_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("quit_cancel"))) != nullptr;
	return success;
}

static void attach_callbacks_to_elements(hanabi_gui_elements_t &elements, game_data& data)
{
	//Beans
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		elements.player.local.bean[i]->SetOnClickUpCallback(bean_callback);
	//Discarded cards button
	elements.discarded_cards.open_button->SetUserData(&data);
	elements.discarded_cards.open_button->SetOnClickUpCallback(discarded_cards_button_callback);
	//Exit menu
	elements.exit_menu.cancel_button->SetOnClickUpCallback(generic_close_menu_button_callback);
	//Discard button
	elements.discard_card_button->SetUserData(&data);
	elements.discard_card_button->SetOnClickUpCallback(discard_card_button_callback);
	//Clue OK button
	elements.clue.clue_button->SetOnClickUpCallback(generic_close_menu_button_callback);
	//User cards
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
	{
		elements.player.local.player_card[i]->SetUserData(&data);
		elements.player.local.player_card[i]->SetOnClickUpCallback(local_user_card_callback);
		elements.player.local.player_card[i]->SetAuxData(i);
	}
}

static void initialize_deck(deck& card_deck)
{
	card_deck.clear();
	card_color_t color[HANABI_TOTAL_COLORS] = { COLOR_RED,COLOR_GREEN,COLOR_BLUE,COLOR_YELLOW,COLOR_WHITE };
	card_number_t number[HANABI_TOTAL_NUMBERS] = { NUMBER_ONE,NUMBER_TWO,NUMBER_THREE,NUMBER_FOUR,NUMBER_FIVE };
	unsigned int quantity[HANABI_TOTAL_NUMBERS] = { TOTAL_ONES,TOTAL_TWOS,TOTAL_THREES,TOTAL_FOURS,TOTAL_FIVES };
	for (unsigned int i = 0; i < sizeof(color) / sizeof(color[0]); i++)
		for (unsigned int j = 0; j < sizeof(number) / sizeof(number[0]); j++)
			card_deck.add_card(card(color[i], number[j]), quantity[j]);
	card_deck.shuffle();
}

//#############
//# Callbacks #
//#############
bool bean_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		source->SetAuxData(!aux_data);
		source->SetUseTopBitmap(!aux_data);
	}
	return false;
}

bool discarded_cards_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		((game_data*)user_data)->elements.discarded_cards.menu->SetIsVisible(true);
		((game_data*)user_data)->elements.discarded_cards.menu->SetIsActive(true);
		(*redraw) = true;
	}
	return false;
}

bool discard_card_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		((game_data*)user_data)->discard_mode_enabled = !((game_data*)user_data)->discard_mode_enabled;
		source->SetUseTopBitmap(((game_data*)user_data)->discard_mode_enabled);
		(*redraw) = true;
	}
	return false;
}

bool generic_close_menu_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element)
		source->ReleaseMouse();
	return mouse_over_element;	//true will close menu
}

bool local_user_card_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		((game_data*)user_data)->local_event_card_offset = aux_data;
		((game_data*)user_data)->local_event_queue.push(((game_data*)user_data)->discard_mode_enabled ? USER_DISCARD_CARD : USER_PLAY_CARD);
	}
	return false;
}

//#################################
//## HANABI FINITE STATE MACHINE ##
//#################################

//NOTE: Separated in two parts:
//First: States and actions while NOT IN GAME
//Second: States and actions while IN GAME
//Each part is separated in groups, in each group we have actions first ($) and then states(%)
//This organization of states and events DOES NOT AFFECT the behaviour of the FSM, since this is just an abstract organization. 

//First, let's define a do nothing function
static void do_nothing(game_data& data, Package_hanabi* package) { return; }
//And, common fake event
extern const STATE common[];
//And end state
extern const STATE end_state[];

//This is the golden function
const STATE* fsm_handler(const STATE * current_state, fsm_event_T ev, game_data& data, Package_hanabi* package)
{
	const STATE* st = current_state;
	if (st != end_state)	//If end state, end state.
	{
		//State event table
		while (st->block_ev != ev && st->block_ev != GND)
			st++;
		if (st->block_ev != GND)
		{
			st->action(data, package);
			return st->next_state;
		}
		//Common event tablex
		st = common;
		while (st->block_ev != ev && st->block_ev != GND)
			st++;
		{
			st->action(data, package);
			st = st->next_state;
			//In common event table, nullptr means stay in same state
			return (st == nullptr ? current_state : st);
		}
	}
	return current_state;
}


//######################
//# Actions prototypes #
//######################
//Notation: state__event
static void fsm_start_point__server(game_data& data, Package_hanabi* package);										//Done!
static void fsm_start_point__client(game_data& data, Package_hanabi* package);										//Done!
static void s_wait_nameis__nameis(game_data& data, Package_hanabi* package);										//Done!
static void s_wait_name__name(game_data& data, Package_hanabi* package);											//Done!
static void s_wait_nameis_ack__ack(game_data& data, Package_hanabi* package);										//WIP
static void c_wait_name__name(game_data& data, Package_hanabi* package);											//Done!
static void c_wait_nameis_ack__ack(game_data& data, Package_hanabi* package);										//Done!
static void c_wait_nameis__nameis(game_data& data, Package_hanabi* package);										//Done!
static void c_wait_start_info__start_info(game_data& data, Package_hanabi* package);								//WIP
static void wait_start_info_ack__ack(game_data& data, Package_hanabi* package);										//Done!
static void wait_software_who__sw_who_i(game_data& data, Package_hanabi* package);									//Done!
static void wait_software_who__sw_who_you(game_data& data, Package_hanabi* package);								//WIP
static void wait_i_start_ack__ack(game_data& data, Package_hanabi* package);										//WIP
static void wait_who__i_start(game_data& data, Package_hanabi* package);											//WIP
static void wait_who__you_start(game_data& data, Package_hanabi* package);											//WIP
static void a_wait_remote_play_again_answer__remote_pa(game_data& data, Package_hanabi* package);
static void a_wait_remote_play_again_answer__remote_go(game_data& data, Package_hanabi* package);
static void a_wait_local_play_again_answer__local_pa(game_data& data, Package_hanabi* package);
static void a_wait_local_play_again_answer__local_go(game_data& data, Package_hanabi* package);
static void b_wait_local_play_again_answer__local_pa(game_data& data, Package_hanabi* package);
static void b_wait_local_play_again_answer__local_go(game_data& data, Package_hanabi* package);
static void b_wait_remote_play_again_answer__start_info(game_data& data, Package_hanabi* package);
static void b_wait_remote_play_again_answer__remote_go(game_data& data, Package_hanabi* package);
static void local_player_turn__local_give_clue(game_data& data, Package_hanabi* package);
static void local_player_turn__local_play(game_data& data, Package_hanabi* package);
static void local_player_turn__local_discard(game_data& data, Package_hanabi* package);
static void wait_remote_player_response__ack(game_data& data, Package_hanabi* package);
static void wait_remote_player_response__remote_we_won(game_data& data, Package_hanabi* package);
static void wait_remote_player_response__remote_we_lost(game_data& data, Package_hanabi* package);
static void wait_sw_draw__sw_draw_next(game_data& data, Package_hanabi* package);
static void wait_sw_draw__sw_draw_last(game_data& data, Package_hanabi* package);
static void remote_player_turn__remote_give_clue(game_data& data, Package_hanabi* package);
static void remote_player_turn__remote_play(game_data& data, Package_hanabi* package);
static void remote_player_turn__remote_discard(game_data& data, Package_hanabi* package);
static void remote_player_turn__remote_play_won(game_data& data, Package_hanabi* package);
static void remote_player_turn__remote_play_lost(game_data& data, Package_hanabi* package);
static void wait_draw__draw_next(game_data& data, Package_hanabi* package);
static void wait_draw__draw_last(game_data& data, Package_hanabi* package);
static void sc_1_remote_player_turn__remote_give_clue(game_data& data, Package_hanabi* package);
static void sc_1_remote_player_turn__remote_play(game_data& data, Package_hanabi* package);
static void sc_1_remote_player_turn__remote_discard(game_data& data, Package_hanabi* package);
static void sc_1_remote_player_turn__remote_play_won(game_data& data, Package_hanabi* package);
static void sc_1_remote_player_turn__remote_play_lost(game_data& data, Package_hanabi* package);
static void sc_1_wait_draw__draw_fake(game_data& data, Package_hanabi* package);
static void sc_1_local_player_turn__local_give_clue(game_data& data, Package_hanabi* package);
static void sc_1_local_player_turn__local_play(game_data& data, Package_hanabi* package);
static void sc_1_local_player_turn__local_discard(game_data& data, Package_hanabi* package);
static void sc_1_wait_remote_player_response__remote_we_won(game_data& data, Package_hanabi* package);
static void sc_1_wait_remote_player_response__remote_we_lost(game_data& data, Package_hanabi* package);
static void sc_1_wait_remote_player_response__remote_match_is_over(game_data& data, Package_hanabi* package);
static void sc_2_local_player_turn__local_give_clue(game_data& data, Package_hanabi* package);
static void sc_2_local_player_turn__local_play(game_data& data, Package_hanabi* package);
static void sc_2_local_player_turn__local_discard(game_data& data, Package_hanabi* package);
static void sc_2_wait_remote_player_response__ack(game_data& data, Package_hanabi* package);
static void sc_2_wait_remote_player_response__remote_we_won(game_data& data, Package_hanabi* package);
static void sc_2_wait_remote_player_response__remote_we_lost(game_data& data, Package_hanabi* package);
static void sc_2_remote_player_turn__remote_give_clue(game_data& data, Package_hanabi* package);
static void sc_2_remote_player_turn__remote_discard(game_data& data, Package_hanabi* package);
static void sc_2_remote_player_turn__remote_play(game_data& data, Package_hanabi* package);
static void sc_2_remote_player_turn__remote_play(game_data& data, Package_hanabi* package);
static void sc_2_remote_player_turn__remote_play_won(game_data& data, Package_hanabi* package);
static void sc_2_remote_player_turn__remote_play_lost(game_data& data, Package_hanabi* package);
static void local_player_quit__ack(game_data& data, Package_hanabi* package);										//Done!
static void common__error_ev(game_data& data, Package_hanabi* package);												//Done!
static void common__bad(game_data& data, Package_hanabi* package);													//Done!
static void common__quit(game_data& data, Package_hanabi* package);													//Done!
static void common__local_quit(game_data& data, Package_hanabi* package);											//Done!

//#####################
//# States prototypes #
//#####################
//Notation: state__event

//extern const STATE fsm_start_point[]; Defined above
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
extern const STATE local_player_quit[];
//extern const STATE end_state[];	//Defined above


//Usefull functions for fsm actions
static void new_game(game_data& data)
{
	initialize_deck(data.card_deck);
	data.clues = HANABI_TOTAL_CLUE_INDICATORS;
	data.lightnings = 0;
	data.discard_count = 0;
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
		data.color_stack[i] = 0;
}

static void local_player_turn_starts(game_data& data)
{
	data.elements.player.local.cards_menu->SetIsActive(true);
	if (data.clues != 0)
	{
		data.elements.give_clue_menu.menu->SetIsVisible(true);
		data.elements.give_clue_menu.menu->SetIsActive(true);
	}
	data.elements.discard_card_button->SetIsVisible(true);
	data.elements.discard_card_button->SetIsActive(true);
	data.elements.message->SetText("It's your turn.");
	data.elements.message->SetIsVisible(true);
	data.discard_mode_enabled = false;
	data.redraw = true;
}

static void local_player_turn_ends(game_data& data)
{
	data.elements.player.local.cards_menu->ReleaseMouse();
	data.elements.player.local.cards_menu->SetIsActive(false);
	data.elements.give_clue_menu.menu->SetIsVisible(false);
	data.elements.give_clue_menu.menu->SetIsActive(false);
	data.elements.discard_card_button->SetIsVisible(false);
	data.elements.discard_card_button->SetIsActive(false);
	data.elements.message->SetText("");
	data.elements.message->SetIsVisible(false);
	data.discard_mode_enabled = false;
	data.redraw = true;
}

static void remote_player_turn_starts(game_data& data)
{
	data.elements.message->SetText("It's your friend's turn.");
	data.elements.message->SetIsVisible(true);
	data.redraw = true;
}

static void remote_player_turn_ends(game_data& data)
{
	data.elements.message->SetText("");
	data.elements.message->SetIsVisible(false);
	data.redraw = true;
}

static void discard_card(const card& c, game_data& data)
{
	data.elements.discarded_cards.open_button->SetBitmap(data.skin.get_bitmap(c));
	data.elements.discarded_cards.cards[data.discard_count++]->SetDefaultBitmap(data.skin.get_bitmap(c));
	data.redraw = true;
}
static bool play_card(const card& c, game_data& data)
{
	if (data.color_stack[c.get_color()] == c.get_number())
	{
		if ((++data.color_stack[c.get_color()]) == HANABI_TOTAL_NUMBERS && data.clues != HANABI_TOTAL_CLUE_INDICATORS)
			data.elements.indicators.clue_indicators[data.clues++]->SetUseTopBitmap(false);
		data.elements.center_cards[c.get_color()][c.get_number()]->SetDefaultBitmap(data.skin.get_bitmap(c));
		data.elements.center_cards[c.get_color()][c.get_number()]->SetIsVisible(true);
		data.redraw = true;
		return true;
	}
	else
	{
		discard_card(c, data);
		data.elements.indicators.lightning_indicators[data.lightnings++]->SetUseTopBitmap(true);
		data.redraw = true;
	}
	return false;
}


//<-- While NOT in game!!! -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: START $
//$$$$$$$$$$$$$$$$$$$$$$$
static void fsm_start_point__server(game_data& data, Package_hanabi* package)
{
	//Server must break the ice. A good way to start is asking for her name.
	char raw_data[MAX_PACKAGE_SIZE];
	Package_name p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void fsm_start_point__client(game_data& data, Package_hanabi* package)
{
	//DO NOTHING.
}
//%%%%%%%%%%%%%%%%%%%%%%
//% State group: START %
//%%%%%%%%%%%%%%%%%%%%%%
static const STATE fsm_start_point[] =
{
	{ SERVER,fsm_start_point__server,s_wait_nameis },
	{ CLIENT,fsm_start_point__client,c_wait_name },
	{ GND,nullptr,nullptr }
};


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: HANDSHAKE, branch: Server $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void s_wait_nameis__nameis(game_data& data, Package_hanabi* package)
{
	//Received remote name!
	string name;
	Package_name_is *rec_p;
	char raw_data[MAX_PACKAGE_SIZE];
	if ((rec_p = dynamic_cast<Package_name_is*>(package)) != nullptr)
	{
		Package_ack p;
		size_t data_size;
		rec_p->get_name(name);
		data.elements.player.remote.name->SetText(name);
		data.elements.player.remote.name->SetIsVisible(true);
		p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
		size_t sent_bytes;
		data.feedback_event = FB_NO_EVENT;
		if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void s_wait_name__name(game_data& data, Package_hanabi* package)
{
	//Received name request! send name_is
	char raw_data[MAX_PACKAGE_SIZE];
	Package_name_is p;
	p.set_name(data.user_name);
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void s_wait_nameis_ack__ack(game_data& data, Package_hanabi* package)
{
	//Send start info!
	Package_start_info p;
	char raw_data[MAX_PACKAGE_SIZE];
	size_t data_size;
	new_game(data);
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
	{
		data.local_player_card[i] = data.card_deck.pick_top();
		data.remote_player_card[i] = data.card_deck.pick_top();
		data.elements.player.remote.player_card[i]->SetDefaultBitmap(data.skin.get_bitmap(data.remote_player_card[i]));
		data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(data.local_player_card[i]));	//Just for debugging
	}
	data.elements.player.local.cards_menu->SetIsVisible(true);
	data.elements.player.remote.cards_menu->SetIsVisible(true);
	p.set_info(data.remote_player_card, data.local_player_card);
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	data.feedback_event = FB_NO_EVENT;
	size_t sent_bytes;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: HANDSHAKE, branch: Server %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

static const STATE s_wait_nameis[] =
{
	{ NAMEIS,s_wait_nameis__nameis,s_wait_name },
	{ GND,nullptr,nullptr}
};

static const STATE s_wait_name[] =
{
	{ NAME,s_wait_name__name,s_wait_nameis_ack },
	{ GND,nullptr,nullptr}
};

static const STATE s_wait_nameis_ack[] =
{
	{ ACK,s_wait_nameis_ack__ack,wait_start_info_ack },  // --> GO TO INITIALIZATION
	{ GND,nullptr,nullptr }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: HANDSHAKE, branch: Client $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void c_wait_name__name(game_data& data, Package_hanabi* package)
{
	s_wait_name__name(data, package);	//Same as server
}
static void c_wait_nameis_ack__ack(game_data& data, Package_hanabi* package)
{
	fsm_start_point__server(data, package);	//Same as fsm_start_point__server
}
static void c_wait_nameis__nameis(game_data& data, Package_hanabi* package)
{
	s_wait_nameis__nameis(data, package);	//Same as server
}
static void c_wait_start_info__start_info(game_data& data, Package_hanabi* package)
{
	//Received start info!
	Package_start_info *rec_p;
	char raw_data[MAX_PACKAGE_SIZE];
	new_game(data);
	if ((rec_p = dynamic_cast<Package_start_info*>(package)) != nullptr)
	{
		rec_p->get_info(data.local_player_card, data.remote_player_card);
		//Show remote cards!
		for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		{
			data.elements.player.remote.player_card[i]->SetDefaultBitmap(data.skin.get_bitmap(data.remote_player_card[i]));
			data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(data.local_player_card[i]));	//Just for debugging
			data.card_deck.remove_card(data.local_player_card[i]);
			data.card_deck.remove_card(data.remote_player_card[i]);
		}
		data.elements.player.local.cards_menu->SetIsVisible(true);
		data.elements.player.remote.cards_menu->SetIsVisible(true);
		Package_ack p;
		size_t data_size;
		p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
		size_t sent_bytes;
		data.feedback_event = FB_NO_EVENT;
		if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
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
static void wait_start_info_ack__ack(game_data& data, Package_hanabi* package)
{
	data.feedback_event = FB_WHO;
}
static void wait_software_who__sw_who_i(game_data& data, Package_hanabi* package)
{
	//Send I start
	char raw_data[MAX_PACKAGE_SIZE];
	Package_i_start p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;
}
static void wait_software_who__sw_who_you(game_data& data, Package_hanabi* package)
{
	//Send you start
	char raw_data[MAX_PACKAGE_SIZE];
	Package_you_start p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;
	remote_player_turn_starts(data);
}
static void wait_i_start_ack__ack(game_data& data, Package_hanabi* package)
{
	local_player_turn_starts(data);
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
static void wait_who__i_start(game_data& data, Package_hanabi* package)
{
	//Send ack
	char raw_data[MAX_PACKAGE_SIZE];
	Package_ack p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;
	remote_player_turn_starts(data);
}
static void wait_who__you_start(game_data& data, Package_hanabi* package)
{
	local_player_turn_starts(data);
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
static void a_wait_remote_play_again_answer__remote_pa(game_data& data, Package_hanabi* package)
{
}
static void a_wait_remote_play_again_answer__remote_go(game_data& data, Package_hanabi* package)
{
}
static void a_wait_local_play_again_answer__local_pa(game_data& data, Package_hanabi* package)
{
}
static void a_wait_local_play_again_answer__local_go(game_data& data, Package_hanabi* package)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: END OF GAME, branch: A:INFORMER (The one who informed game result) %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE a_wait_remote_play_again_answer[] =
{
	{ REMOTE_PA,a_wait_remote_play_again_answer__remote_pa,a_wait_local_play_again_answer },
	{ REMOTE_GO,a_wait_remote_play_again_answer__remote_go,end_state },	//break
	{ GND,nullptr,nullptr }
};
static const STATE a_wait_local_play_again_answer[] =
{
	{ LOCAL_PA,a_wait_local_play_again_answer__local_pa,wait_start_info_ack },		//  -->> GO TO INITIALIZATION
	{ LOCAL_GO,a_wait_local_play_again_answer__local_go,end_state },		//break
	{ GND,nullptr,nullptr }
};


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: END OF GAME, branch: B:INFORMED (The one who received information about game result) $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void b_wait_local_play_again_answer__local_pa(game_data& data, Package_hanabi* package)
{
}
static void b_wait_local_play_again_answer__local_go(game_data& data, Package_hanabi* package)
{
}
static void b_wait_remote_play_again_answer__start_info(game_data& data, Package_hanabi* package)
{
}
static void b_wait_remote_play_again_answer__remote_go(game_data& data, Package_hanabi* package)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: END OF GAME, branch: B:INFORMED (The one who received information about game result) %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
static const STATE b_wait_local_play_again_answer[] =
{
	{ LOCAL_PA,b_wait_local_play_again_answer__local_pa,b_wait_remote_play_again_answer },
	{ LOCAL_GO,b_wait_local_play_again_answer__local_go,end_state }, //break
	{ GND,nullptr,nullptr }
};
static const STATE b_wait_remote_play_again_answer[] =
{
	{ START_INFO,b_wait_remote_play_again_answer__start_info,wait_who },		//  --> GO TO INITIALIZATION
	{ REMOTE_GO,b_wait_remote_play_again_answer__remote_go,end_state }, //break
	{ GND,nullptr,nullptr }
};

//<-- While in game -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: IN GAME, branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
static void local_player_turn__local_give_clue(game_data& data, Package_hanabi* package)
{
	local_player_turn_ends(data);
}
static void local_player_turn__local_play(game_data& data, Package_hanabi* package)
{
	local_player_turn_ends(data);
	card c = data.local_player_card[data.local_event_card_offset];
	string card_name(color_string[c.get_color()]);
	card_name += "-";
	card_name += '0' + c.get_number() + 1;
	transform(card_name.begin(), card_name.end(), card_name.begin(), toupper);
	string message = string("You played ") + card_name;
	if (play_card(c, data))
		message += " - SUCCESS!";
	else
		message += " - Card discarded :(";
	data.elements.message2->SetText(message.c_str());
	data.elements.message2->SetIsVisible(true);
	data.local_player_card[data.local_event_card_offset] = card();
	data.elements.player.local.player_card[data.local_event_card_offset]->SetIsVisible(false);
	data.redraw = true;
	//Send play package
	char raw_data[MAX_PACKAGE_SIZE];
	Package_play p;
	p.set_card_id(data.local_event_card_offset);
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void local_player_turn__local_discard(game_data& data, Package_hanabi* package)
{
	local_player_turn_ends(data);
	card c = data.local_player_card[data.local_event_card_offset];
	string card_name(color_string[c.get_color()]);
	card_name += "-";
	card_name += '0' + c.get_number() + 1;
	transform(card_name.begin(), card_name.end(), card_name.begin(), toupper);
	string message = string("You discarded ") + card_name;
	discard_card(c, data);
	data.elements.message2->SetText(message.c_str());
	data.elements.message2->SetIsVisible(true);
	data.local_player_card[data.local_event_card_offset] = card();
	data.elements.player.local.player_card[data.local_event_card_offset]->SetIsVisible(false);
	data.redraw = true;
	//Send discard package
	char raw_data[MAX_PACKAGE_SIZE];
	Package_discard p;
	p.set_card_id(data.local_event_card_offset);
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void wait_remote_player_response__ack(game_data& data, Package_hanabi* package)
{
}
static void wait_remote_player_response__remote_we_won(game_data& data, Package_hanabi* package)
{
}
static void wait_remote_player_response__remote_we_lost(game_data& data, Package_hanabi* package)
{
}
static void wait_sw_draw__sw_draw_next(game_data& data, Package_hanabi* package)
{
}
static void wait_sw_draw__sw_draw_last(game_data& data, Package_hanabi* package)
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
static void remote_player_turn__remote_give_clue(game_data& data, Package_hanabi* package)
{
}
static void remote_player_turn__remote_play(game_data& data, Package_hanabi* package)
{
	remote_player_turn_ends(data);
	//Received play info!
	Package_play *rec_p;
	new_game(data);
	if ((rec_p = dynamic_cast<Package_play*>(package)) != nullptr)
	{
		unsigned int card_id;
		rec_p->get_card_id(&card_id);
		card c = data.remote_player_card[card_id];
		string card_name(color_string[c.get_color()]);
		card_name += "-";
		card_name += '0' + c.get_number() + 1;
		transform(card_name.begin(), card_name.end(), card_name.begin(), toupper);
		string message = string("Friend played ") + card_name;
		if (play_card(c, data))
			message += " - SUCCESS!";
		else
			message += " - Card discarded :(";
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);
		data.remote_player_card[card_id] = card();
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;
		//Send ack package
		char raw_data[MAX_PACKAGE_SIZE];
		Package_ack p;
		size_t data_size;
		p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
		size_t sent_bytes;
		data.feedback_event = FB_NO_EVENT;
		if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void remote_player_turn__remote_discard(game_data& data, Package_hanabi* package)
{
	//Received discard info!
	remote_player_turn_ends(data);
	Package_discard *rec_p;
	new_game(data);
	if ((rec_p = dynamic_cast<Package_discard*>(package)) != nullptr)
	{
		unsigned int card_id;
		rec_p->get_card_id(&card_id);
		card c = data.remote_player_card[card_id];
		string card_name(color_string[c.get_color()]);
		card_name += "-";
		card_name += '0' + c.get_number() + 1;
		transform(card_name.begin(), card_name.end(), card_name.begin(), toupper);
		string message = string("Friend discarded ") + card_name;
		discard_card(c, data);
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);
		data.remote_player_card[card_id] = card();
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;
		//Send ack package
		char raw_data[MAX_PACKAGE_SIZE];
		Package_ack p;
		size_t data_size;
		p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
		size_t sent_bytes;
		data.feedback_event = FB_NO_EVENT;
		if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void remote_player_turn__remote_play_won(game_data& data, Package_hanabi* package)
{
}
static void remote_player_turn__remote_play_lost(game_data& data, Package_hanabi* package)
{
}
static void wait_draw__draw_next(game_data& data, Package_hanabi* package)
{
}
static void wait_draw__draw_last(game_data& data, Package_hanabi* package)
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
static void sc_1_remote_player_turn__remote_give_clue(game_data& data, Package_hanabi* package)
{
}
static void sc_1_remote_player_turn__remote_play(game_data& data, Package_hanabi* package)
{
}
static void sc_1_remote_player_turn__remote_discard(game_data& data, Package_hanabi* package)
{
}
static void sc_1_remote_player_turn__remote_play_won(game_data& data, Package_hanabi* package)
{
}
static void sc_1_remote_player_turn__remote_play_lost(game_data& data, Package_hanabi* package)
{
}
static void sc_1_wait_draw__draw_fake(game_data& data, Package_hanabi* package)
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
static void sc_1_local_player_turn__local_give_clue(game_data& data, Package_hanabi* package)
{
}
static void sc_1_local_player_turn__local_play(game_data& data, Package_hanabi* package)
{
}
static void sc_1_local_player_turn__local_discard(game_data& data, Package_hanabi* package)
{
}
static void sc_1_wait_remote_player_response__remote_we_won(game_data& data, Package_hanabi* package)
{
}
static void sc_1_wait_remote_player_response__remote_we_lost(game_data& data, Package_hanabi* package)
{
}
static void sc_1_wait_remote_player_response__remote_match_is_over(game_data& data, Package_hanabi* package)
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
static void sc_2_local_player_turn__local_give_clue(game_data& data, Package_hanabi* package)
{
}
static void sc_2_local_player_turn__local_play(game_data& data, Package_hanabi* package)
{
}
static void sc_2_local_player_turn__local_discard(game_data& data, Package_hanabi* package)
{
}
static void sc_2_wait_remote_player_response__ack(game_data& data, Package_hanabi* package)
{
}
static void sc_2_wait_remote_player_response__remote_we_won(game_data& data, Package_hanabi* package)
{
}
static void sc_2_wait_remote_player_response__remote_we_lost(game_data& data, Package_hanabi* package)
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
static void sc_2_remote_player_turn__remote_give_clue(game_data& data, Package_hanabi* package)
{
}
static void sc_2_remote_player_turn__remote_discard(game_data& data, Package_hanabi* package)
{
}
static void sc_2_remote_player_turn__remote_play(game_data& data, Package_hanabi* package)
{
}
static void sc_2_remote_player_turn__remote_play_won(game_data& data, Package_hanabi* package)
{
}
static void sc_2_remote_player_turn__remote_play_lost(game_data& data, Package_hanabi* package)
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

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: LOCAL PLAYER QUIT $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static void local_player_quit__ack(game_data& data, Package_hanabi* package)
{
	data.break_event_loop = true;
	data.connection->disconnect();
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: LOCAL PLAYER QUIT %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

static const STATE local_player_quit[] =
{
	{ ACK,local_player_quit__ack,end_state },	//break
	{ GND, nullptr, nullptr}
};


//%%%%%%%%%%%%%
//% END STATE %
//%%%%%%%%%%%%%

static const STATE end_state[] =
{
	{GND,nullptr,nullptr}
};

//#################################
//# Actions common to all states! #
//#################################
static void common__error_ev(game_data& data, Package_hanabi* package)
{
	data.break_event_loop = true;
	data.connection->disconnect();
};

static void common__bad(game_data& data, Package_hanabi* package)
{
	//Send error package
	char raw_data[MAX_PACKAGE_SIZE];
	Package_error p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_NO_EVENT;	//Don't care about fatal errors here. We areF over.
	data.break_event_loop = true;
	data.connection->disconnect();
};
static void common__quit(game_data& data, Package_hanabi* package)
{
	//Send ack and quit
	char raw_data[MAX_PACKAGE_SIZE];
	Package_ack p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;
	else
		data.break_event_loop = true;
	data.connection->disconnect();
};
static void common__local_quit(game_data& data, Package_hanabi* package)
{
	//Send quit package
	char raw_data[MAX_PACKAGE_SIZE];
	Package_quit p;
	size_t data_size;
	p.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	size_t sent_bytes;
	data.feedback_event = FB_NO_EVENT;
	if (!data.connection->send_data(raw_data, data_size, &sent_bytes) || data_size != sent_bytes)
		data.feedback_event = FB_ERROR;
};

//This state is actually part of all states.
//nullptr means stay in same state
static const STATE common[] =
{
	{ ERROR_EV,common__error_ev,end_state },
	{ BAD,common__bad,end_state },
	{ QUIT,common__quit,end_state },
	{ LOCAL_QUIT,common__local_quit,local_player_quit },
	{ GND,common__bad,end_state }	//Any other event will execute common__bad and change state to end_state
};