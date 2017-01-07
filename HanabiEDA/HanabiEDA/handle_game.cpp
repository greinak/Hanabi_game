#include "handle_game.h"
#include "card.h"
#include "deck.h"
#include "hanabi_messages.h"
#include <iostream>
#include <allegro5\allegro5.h>
#include "Net connection\Packages\Packages.h"
#include "card_skin.h"
#include <queue>
#include <algorithm>

// On visual studio? CTRL+M+L to colaplse all will help!

//Everything is prepared to be read like that! 

#define HANABI_TOTAL_LIGHTNING_INDICATORS	3
#define HANABI_TOTAL_CLUE_INDICATORS		8

//Define CHEAT_SHOW_LOCAL_CARDS to show local player cards!
#define CHEAT_SHOW_LOCAL_CARDS

//Timeout
#define HANABI_TIMEOUT	5	//ONE MINUTE TIMEOUT

using namespace std;

//# Typedefs #

//GUI ELEMENTS!
typedef struct
{
	GuiImage* deck;	//id="deck"
	GuiImage* center_cards[HANABI_TOTAL_COLORS][HANABI_TOTAL_NUMBERS];	//id="center_card_%COLOR%_%NUMBER%"
	GuiButton*	discard_card_button;		//id="disc_card_button"
	GuiText*		message;								//id="message"
	GuiText*		message2;								//id="message"

	struct
	{
		GuiButton* lightning_indicators[HANABI_TOTAL_LIGHTNING_INDICATORS];	//id="light_ind_%ID%"
		GuiButton* clue_indicators[HANABI_TOTAL_CLUE_INDICATORS];		//id="clue_ind_%ID%"
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
		GuiSubmenu	*menu;								//id="show_clue_menu"
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
		GuiText		*title;						//id="game_finished_title" 
		GuiText		*score;						//id="game_finished_menu_score" 
		GuiText		*score_message;				//id="game_finished_menu_message"
		GuiButton	*play_again_button;			//id="game_finished_menu_play_again"
		GuiButton	*quit_button;				//id="game_finished_menu_quit"

	}game_finished_menu;

	struct
	{
		GuiSubmenu	*menu;		//id="connection_error_menu"
		GuiButton	*ok_button;	//id="connection_error_menu_ok"
	}error_menu;

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
//FSM FEEDBACK events
typedef enum
{
	FB_NO_EVENT,
	FB_WHO,
	FB_DRAW,
	FB_ERROR
}fsm_feedback_event_T;
//Local user events
typedef enum
{
	USER_DISCARD_CARD,
	USER_PLAY_CARD,
	USER_GIVE_CLUE,
	USER_QUIT,
	USER_PLAY_AGAIN,
	USER_GAME_OVER,
	USER_EXIT_GAME_NOW
}local_user_event_T;
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
	TIMEOUT,		//Timer timeout
	//<--Control events-->
	ACK,			//ACK
	EXIT_GAME,		//Break event loop
	//<--Initial events-->
	SERVER,			//Start FSM as server
	CLIENT,			//Start FSM as client
	//<--FSM behaviour related events-->
	GND,					//Identifier of end of common event state blocks list. Not a real event.
}fsm_event_T;
//Game data
typedef struct
{
	//Gui and Gui elements
	Gui* game_ui;
	hanabi_gui_elements_t elements;
	bool redraw;
	//Cards textures
	card_skin skin;
	//Network connection
	Net_connection* connection;
	//Allegro stuff
	ALLEGRO_EVENT_QUEUE* ev_q;
	ALLEGRO_TIMER* timer;
	//Game stuff
	card local_player_card[HANABI_HAND_SIZE];
	card remote_player_card[HANABI_HAND_SIZE];
	string user_name;
	deck card_deck;
	unsigned int clues;
	unsigned int lightnings;
	unsigned int color_stack[HANABI_TOTAL_COLORS];
	unsigned int discard_count;
	//Game control variables
	bool discard_mode_enabled;
	bool quit_button_enabled;
	bool exit;
	//FSM action feedback
	fsm_feedback_event_T feedback_event;
	//Local event variables
	bool got_clue;
	bool local_event_clue_is_color;
	unsigned int local_event_clue_id;
	unsigned int local_event_card_offset;
	//Local event queue
	queue<local_user_event_T> local_event_queue;
}game_data;
//Event ID
typedef enum {MOUSE,DISPLAY_CLOSE,FSM} event_id;
//Game event type
typedef struct
{
	event_id	ev_id;
	fsm_event_T	fsm_event;
	Package_hanabi* package;
}game_event_t;
//FSM action
typedef void(*action)(game_data& data, Package_hanabi* package);
//FSM state block
typedef struct state_block
{
	fsm_event_T	block_ev;
	action action;
	const struct state_block* next_state;
}state_block;
//FSM STATE!
typedef struct state_block STATE;

//FSM HANDLER
static const STATE* fsm_handler(const STATE * current_state, fsm_event_T ev, game_data& data, Package_hanabi* package);
//FSM initial state
extern const STATE fsm_start_point[];	//Here is where all starts

//# Some functions prototypes #
static void initialize_deck(deck& card_deck);
static bool attach_menu_elements(hanabi_gui_elements_t &elements, Gui* game_ui);
static void attach_callbacks_to_elements(hanabi_gui_elements_t &elements, game_data& data);
static void wait_for_event(game_event_t* ret_event, game_data& g_data);

//# Callbacks prototypes #
bool bean_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool discarded_cards_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool generic_close_menu_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool discard_card_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool local_user_card_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool give_clue_color_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool give_clue_number_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool give_clue_ok_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool exit_menu_quit_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool exit_game_now_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool play_again_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);
bool game_over_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw);

//# Main game function #
void handle_game(Gui* game_ui, string user_name, Net_connection* net, bool is_server)
{
	game_data data;
	data.connection = net;
	data.game_ui = game_ui;
	data.exit = false;
	data.user_name = user_name;
	data.feedback_event = FB_NO_EVENT;
	if (data.skin.initialized_successfully())
	{
		data.ev_q = al_create_event_queue();
		if (data.ev_q != nullptr)
		{
			data.timer = al_create_timer(HANABI_TIMEOUT);
			if(data.timer != nullptr)
			{
				al_register_event_source(data.ev_q, al_get_mouse_event_source());
				al_register_event_source(data.ev_q, al_get_display_event_source(data.game_ui->get_display()));
				al_register_event_source(data.ev_q, al_get_timer_event_source(data.timer));
				if (attach_menu_elements(data.elements, game_ui))
				{
					attach_callbacks_to_elements(data.elements, data);
					data.elements.player.local.name->SetText(data.user_name);
					data.elements.player.local.name->SetIsVisible(true);
					data.elements.message->SetText("Handshake... Please wait...");
					data.elements.message->SetIsVisible(true);
					data.game_ui->redraw();
					data.quit_button_enabled = true;
					const STATE* state = fsm_start_point;
					if (is_server)
						state = fsm_handler(state, SERVER, data, nullptr);
					else
						state = fsm_handler(state, CLIENT, data, nullptr);
					game_event_t ev;
					while (!data.exit)
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
					cerr << "[GAME_HANDLER][ERROR] : Error identifying menu elements." << endl;
				al_unregister_event_source(data.ev_q,al_get_timer_event_source(data.timer));
				al_destroy_timer(data.timer);
			}
			else
				cerr << "[GAME_HANDLER][ERROR] : Could not create allegro timer for game." << endl;
			al_destroy_event_queue(data.ev_q);
		}
		else
			cerr << "[GAME_HANDLER][ERROR] : Could not create event allegro event queue for game." << endl;
	}
	else
		cerr << "[GAME_HANDLER][ERROR] : Could not load cards skin" << endl;
}

//# Other functions #

//Wait for an event
static void wait_for_event(game_event_t* ret_event,game_data& g_data)
{
	//Got event flag. While false, keep waiting for events
	bool got_event = false;
	//This flag should go false if remote player disconnects!
	bool connection_ok = true;
	//Data buffer
	char raw_data[MAX_PACKAGE_SIZE];
	//Varaible used while receiving data
	size_t data_size = 0;

	ret_event->package = nullptr;	//If no data, always to nullptr. That's the rule
	ret_event->fsm_event = GND;		//Just in order to initialize it to something

	while (!got_event)
	{
		//Feedback events FIRST.
		if (g_data.feedback_event != FB_NO_EVENT)
		{
			switch (g_data.feedback_event)
			{
			case FB_WHO:	//This means we should decide who starts.
			{
				ret_event->ev_id = FSM;
				ret_event->fsm_event = (rand() %2)?SW_WHO_I:SW_WHO_YOU;	//Pseudorandomly select who starts.
				ret_event->package = nullptr;		//If no data, always to nullptr
				got_event = true;
				break;
			}
			case FB_DRAW:	//This means we should draw a card
			{
				ret_event->ev_id = FSM;
				if (g_data.card_deck.size() > 1)
					ret_event->fsm_event = SW_DRAW_NEXT;	//This is not the last card in card deck
				else
					ret_event->fsm_event = SW_DRAW_LAST;	//This is last card in card deck
				ret_event->package = nullptr;		//If no data, always to nullptr
				got_event = true;
				break;
			}
			case FB_ERROR:							//ERROR, just that.
			default:
				got_event = true;
				ret_event->ev_id = FSM;
				ret_event->fsm_event = ERROR_EV;	//FATAL ERROR.
				ret_event->package = nullptr;		//If no data, always to nullptr
			}
			g_data.feedback_event = FB_NO_EVENT;	//Always set value to no event !
		}
		//Local events
		if (!got_event && !g_data.local_event_queue.empty())
		{
			got_event = true;			//If there is a local event, then we know we have an event!
			ret_event->ev_id = FSM;		//This event is for FSM
			local_user_event_T ev = g_data.local_event_queue.front();	//Get event
			g_data.local_event_queue.pop();								//Remove it from queue
			switch(ev)
			{
				//This is just obvious...
			case USER_PLAY_CARD:
				ret_event->fsm_event = LOCAL_PLAY;
				break;
			case USER_DISCARD_CARD:
				ret_event->fsm_event = LOCAL_DISCARD;
				break;
			case USER_GIVE_CLUE:
				ret_event->fsm_event = LOCAL_GIVE_CLUE;
				break;
			case USER_QUIT:
				ret_event->fsm_event = LOCAL_QUIT;
				break;
			case USER_EXIT_GAME_NOW:
				ret_event->fsm_event = EXIT_GAME;	//This means window WILL close!
				break;
			case USER_PLAY_AGAIN:
				ret_event->fsm_event = LOCAL_PA;
				break;
			case USER_GAME_OVER:
				ret_event->fsm_event = LOCAL_GO;
				break;
			default:
				ret_event->fsm_event = BAD;
				break;
			}
		}
		//Allegro mouse and display events
		if (!got_event && !al_is_event_queue_empty(g_data.ev_q))
			{
				ALLEGRO_EVENT ev;
				al_wait_for_event(g_data.ev_q, &ev);
				if (ev.any.source == al_get_mouse_event_source())
				{
					if (ev.mouse.display == g_data.game_ui->get_display())
					{
						//A note about this event:
						//Actually, mouse events are not necesary
						//They are just used as a trigger to send mouse state to gui
						//only when mouse state changes
						ret_event->ev_id = MOUSE;
						got_event = true;
					}
				}
				else if (ev.any.source == al_get_display_event_source(g_data.game_ui->get_display()))
				{
					//Window close button!
					if (ev.display.type == ALLEGRO_EVENT_DISPLAY_CLOSE && g_data.quit_button_enabled)
					{
						ret_event->ev_id = DISPLAY_CLOSE;
						got_event = true;
					}
				}
				else if (ev.timer.type == ALLEGRO_EVENT_TIMER)
				{
					//This mean we had a timeout...
					ret_event->ev_id = FSM;
					ret_event->fsm_event = TIMEOUT;
					ret_event->package = nullptr;
					got_event = true;
				}
			}
		//Connection events
		if (!got_event && g_data.connection->is_connected())
		{
			//This function should return false if remote player disconnected
			connection_ok = g_data.connection->receive_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
			if (connection_ok)
			{
				//True does not mean we have data. It only means connection is ok! Have we got data?
				if (data_size != 0)
				{
					bool data_ok = true;	//This flag will go to false if received data is invalid
					package_type p_type;	//Package identifier
					ret_event->ev_id = FSM;	//We know event is for fsm
					got_event = true;		//And, for good or for bad, we have an event!
					switch ((p_type = get_package_type_from_raw_data(raw_data, data_size)))
					{
						//What have we got here?
						//remember there is an importan data verification in load_raw_data
						//If that function returns true, that means that received package at least makes sense!
						case ACK_P:
						{
							Package_ack* package = new Package_ack;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = ACK;	//ACK!!
							else
								data_ok = false;
							break;
						}
						case NAME_P:
						{
							Package_name* package = new Package_name;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = NAME;	//This is a name request
							else
								data_ok = false;
							break;
						}
						case NAME_IS_P:
						{
							Package_name_is* package = new Package_name_is;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = NAMEIS;	//Name is!! (Again, remember, verification in load_raw_data!)
							else
								data_ok = false;
							break;
						}
						case START_INFO_P:
						{
							Package_start_info* package = new Package_start_info;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
							{
								//Package struct at least makes sense.
								//Are cards given valid for an initial state?
								bool start_info_ok = true;	//Let's assume yes
								card hand_1[HANABI_HAND_SIZE];
								card hand_2[HANABI_HAND_SIZE];
								deck start_info_deck;				//Create deck in order to check if start info is valid.
								package->get_info(hand_1, hand_2);	//Load cards to temp hands
								initialize_deck(start_info_deck);	//Initialize deck
								for (unsigned int i = 0; i < HANABI_HAND_SIZE && start_info_ok; i++)
								{
									start_info_ok &= start_info_deck.remove_card(hand_1[i]);
									start_info_ok &= start_info_deck.remove_card(hand_2[i]);
								}//remove card will return true if card was in deck. This will prevent, for example, two FIVEs of the same color
								if (start_info_ok)
									ret_event->fsm_event = START_INFO;	//Ok, data is perfect! fsm will check if context is addecuate for this package
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
								ret_event->fsm_event = YOU_START;	//You start
							else
								data_ok = false;
							break;
						}
						case I_START_P:
						{
							Package_i_start* package = new Package_i_start;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = I_START;		//I start
							else
								data_ok = false;
							break;
						}
						case PLAY_P:
						{
 							Package_play* package = new Package_play;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
							{
								//Play package... this may trigger different events
								unsigned int i;
								card c;
								//Get card offset
								package->get_card_id(&i);	
								//Load card
								c = g_data.remote_player_card[i];
								//Now c is the played card
								if (g_data.color_stack[c.get_color()] != c.get_number())	//This will be true if card will be discarded if in game
								{
									if (g_data.lightnings == HANABI_TOTAL_LIGHTNING_INDICATORS - 1)	//Can we afford that?
										ret_event->fsm_event = REMOTE_PLAY_LOST;
									else
										ret_event->fsm_event = REMOTE_PLAY;
								}
								else
								{
									//Card wont be discarded (assuming we are in game).
									unsigned int played_cards = 0;
									if (c.get_number() == HANABI_TOTAL_NUMBERS - 1)	//Is it a top card?
									{
										//If yes, how many cards are there in center stacks?
										for (unsigned int j = 0; j < HANABI_TOTAL_COLORS; j++)
											played_cards += g_data.color_stack[j];
										//Is it almost full, but missing one?
										if (played_cards == HANABI_TOTAL_COLORS*HANABI_TOTAL_NUMBERS - 1)
											ret_event->fsm_event = REMOTE_PLAY_WON;	//Then we won
										else
											ret_event->fsm_event = REMOTE_PLAY;		//Else, just play it
									}
									else
										ret_event->fsm_event = REMOTE_PLAY;		//No top card. just play it
								}
							}
							else
								data_ok = false;
							break;
						}
						case DISCARD_P:
						{
							Package_discard* package = new Package_discard;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = REMOTE_DISCARD;	//Discard
							else
								data_ok = false;
							break;
						}
						case YOU_HAVE_P:
						{
							Package_you_have* package = new Package_you_have;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
							{
								//Two possibilities
								if(g_data.clues != 0)
									ret_event->fsm_event = REMOTE_GIVE_CLUE; //to be
								else
									ret_event->fsm_event = BAD;	//or not to be
							}
							else
								data_ok = false;
							break;
						}
						case DRAW_P:
						{
							Package_draw* package = new Package_draw;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
							{
								//This may trigger different events...
								card c;
								package->get_card(&c);
								if (g_data.card_deck.size() == 0 && c == card(NO_COLOR,NO_NUMBER))
									ret_event->fsm_event = DRAW_FAKE;	//No cards left in deck, this is the draw fake card event
								else if (g_data.card_deck.count_cards(c) != 0)	//Else, are there samples of this card in card deck?
								{
									if (g_data.card_deck.size() > 1)
										ret_event->fsm_event = DRAW_NEXT;	//If yes, and this card is not the only one, just draw it
									else
										ret_event->fsm_event = DRAW_LAST;	//If yes, but this card is last one, draw last card!
								}
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
								ret_event->fsm_event = REMOTE_WE_WON;	//We won. FSM will take care of checking this
							else
								data_ok = false;
							break;
						}
						case WE_LOST_P:
						{
							Package_we_lost* package = new Package_we_lost;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = REMOTE_WE_LOST;	//We lost. FSM will take care of checking this
							else
								data_ok = false;
							break;
						}
						case MATCH_IS_OVER_P:
						{
							Package_match_is_over* package = new Package_match_is_over;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = REMOTE_MATCH_IS_OVER;	//Match is over. FSM will take care of checking this
							else
								data_ok = false;
							break;
						}
						case PLAY_AGAIN_P:
						{
							Package_play_again* package = new Package_play_again;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = REMOTE_PA;	//Remote player wants to play again
							else
								data_ok = false;
							break;
						}
						case GAME_OVER_P:
						{
							Package_game_over* package = new Package_game_over;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = REMOTE_GO;	//Remote player does not want to keep on playing
							else
								data_ok = false;
							break;
						}
						case QUIT_P:
						{
							Package_quit* package = new Package_quit;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = QUIT;		//Remote player wants to quit game
							else
								data_ok = false;
							break;
						}
						case ERROR_P:
						{
							Package_error* package = new Package_error;
							if ((ret_event->package = package) != nullptr && package->load_raw_data(raw_data, data_size))
								ret_event->fsm_event = ERROR_EV;	//Remote program detected an error.
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
						//Error?
						delete ret_event->package;		//Free package data!!
						ret_event->package = nullptr;	//If no data, always to nullptr. That's the rule
						ret_event->fsm_event = BAD;		//BAD, since must inform about this to remote!
					}
				}
				else
				{
					//No data received :)
				}
			}
			else
			{
				//If we are here is because remote player disconnected without letting us know
				//This is probably due to an error in the program
				//Or a problem with the connection
				got_event = true;
				ret_event->ev_id = FSM;
				ret_event->fsm_event = ERROR_EV;	//FATAL CONNECTION ERROR.
				ret_event->package = nullptr;		//If no data, always to nullptr!
			}
		}
	}
}

//Attach all UI elements to objects in code
static bool attach_menu_elements(hanabi_gui_elements_t &elements, Gui* game_ui)
{
	//Note: The idea of defining elements in an xml and then refering to them by id was inspired by Android UI
	//It looks like operator &= is not lazy :(

	//In order to undestart this function, read menu_data xml files!

	bool success = true;	//True while we can load stuff

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
	if(success) success &= (elements.discard_card_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("disc_card_button"))) != nullptr;

	//message
	if (success) success &= (elements.message = dynamic_cast<GuiText*>(game_ui->get_element_from_id("message"))) != nullptr;
	//message2
	if (success) success &= (elements.message2 = dynamic_cast<GuiText*>(game_ui->get_element_from_id("message2"))) != nullptr;
	
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
	if (success) success &= (elements.give_clue_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("give_clue_menu"))) != nullptr;
	//Give clue button
	if (success) success &= (elements.give_clue_menu.give_clue_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("give_clue_button"))) != nullptr;
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
		success &= (elements.give_clue_menu.number_button[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	
	//Local player name
	if (success) success &= (elements.player.local.name = dynamic_cast<GuiText*>(game_ui->get_element_from_id("local_player_name"))) != nullptr;
	//Local player cards menu
	if (success) success &= (elements.player.local.cards_menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("local_cards_menu"))) != nullptr;
	//Local player cards
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("local_card_") + c;
		success &= (elements.player.local.player_card[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}

	//Remote player name
	if (success) success &= (elements.player.remote.name = dynamic_cast<GuiText*>(game_ui->get_element_from_id("remote_player_name"))) != nullptr;
	//Remote player cards menu
	if (success) success &= (elements.player.remote.cards_menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("remote_cards_menu"))) != nullptr;
	//Remote player cards
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("remote_card_") + c;
		success &= (elements.player.remote.player_card[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}

	//Player beans (This buttons are dummy, they are a help for local player to memorize clues!)
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("bean_") + c;
		success &= (elements.player.local.bean[n] = dynamic_cast<GuiButton*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}

	//show clue menu
	if (success) success &= (elements.clue.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("show_clue_menu"))) != nullptr;
	//Clue markers
	for (unsigned int n = 0; n < HANABI_HAND_SIZE && success; n++)
	{
		char c = '0' + n + 1;
		string id = string("card_ind_") + c;
		success &= (elements.clue.clue_marker[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}
	//Clue OK Button
	if (success) success &= (elements.clue.clue_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("clue_ok"))) != nullptr;
	
	//Discarded cards menu
	if (success) success &= (elements.discarded_cards.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("discarded_cards_menu"))) != nullptr;
	//Open discarded cards menu button
	if (success) success &= (elements.discarded_cards.open_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("open_disc_card_menu_button"))) != nullptr;
	//Discarded cards CARDS
	for (unsigned int n = 0; n < HANABI_TOTAL_CARDS && success; n++)
	{
		char number[3];
		itoa(n + 1, number,10);
		string id = string("disc_card_") + number;
		success &= (elements.discarded_cards.cards[n] = dynamic_cast<GuiImage*>(game_ui->get_element_from_id(id.c_str()))) != nullptr;
	}

	//Game finished menu
	if (success) success &= (elements.game_finished_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("game_finished_menu"))) != nullptr;
	//Game finished title
	if (success) success &= (elements.game_finished_menu.title = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_title"))) != nullptr;
	//Game finished score
	if (success) success &= (elements.game_finished_menu.score = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_menu_score"))) != nullptr;
	//Game finished score message
	if (success) success &= (elements.game_finished_menu.score_message = dynamic_cast<GuiText*>(game_ui->get_element_from_id("game_finished_menu_message"))) != nullptr;
	//Game finished play again button
	if (success) success &= (elements.game_finished_menu.play_again_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("game_finished_menu_play_again"))) != nullptr;
	//Game finished quit button
	if (success) success &= (elements.game_finished_menu.quit_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("game_finished_menu_quit"))) != nullptr;

	//error menu
	if (success) success &= (elements.error_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("connection_error_menu"))) != nullptr;
	//error OK button
	if (success) success &= (elements.error_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("connection_error_menu_ok"))) != nullptr;
	//Remote player left menu
	if (success) success &= (elements.remote_player_left_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("remote_player_left_menu"))) != nullptr;
	//Remote player left ok button
	if (success) success &= (elements.remote_player_left_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("remote_player_left_menu_ok"))) != nullptr;

	//Exit menu
	if (success) success &= (elements.exit_menu.menu = dynamic_cast<GuiSubmenu*>(game_ui->get_element_from_id("exit_menu"))) != nullptr;
	//Exit OK button
	if (success) success &= (elements.exit_menu.ok_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("quit_ok"))) != nullptr;
	//Exit cancel button
	if (success) success &= (elements.exit_menu.cancel_button = dynamic_cast<GuiButton*>(game_ui->get_element_from_id("quit_cancel"))) != nullptr;


	return success;
}
//Attach callbacks (and user data) to elements
static void attach_callbacks_to_elements(hanabi_gui_elements_t &elements, game_data& data)
{
	//Beans
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		elements.player.local.bean[i]->SetOnClickUpCallback(bean_callback);
	//Clue OK button
	elements.clue.clue_button->SetOnClickUpCallback(generic_close_menu_button_callback);

	//User cards
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
	{
		elements.player.local.player_card[i]->SetUserData(&data);
		elements.player.local.player_card[i]->SetOnClickUpCallback(local_user_card_callback);
		elements.player.local.player_card[i]->SetAuxData(i);
	}

	//Discarded cards button
	elements.discarded_cards.open_button->SetUserData(&data);
	elements.discarded_cards.open_button->SetOnClickUpCallback(discarded_cards_button_callback);

	//Exit menu
	elements.exit_menu.cancel_button->SetOnClickUpCallback(generic_close_menu_button_callback);
	//Quit button
	elements.exit_menu.ok_button->SetUserData(&data);
	elements.exit_menu.ok_button->SetOnClickUpCallback(exit_menu_quit_button_callback);

	//Discard button
	elements.discard_card_button->SetUserData(&data);
	elements.discard_card_button->SetOnClickUpCallback(discard_card_button_callback);

	//Give clue menu
	//Give clue color buttons
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
	{
		elements.give_clue_menu.color_button[i]->SetUserData(&data);
		elements.give_clue_menu.color_button[i]->SetOnClickUpCallback(give_clue_color_button_callback);
		elements.give_clue_menu.color_button[i]->SetAuxData(i);
	}
	//Give clue number buttons
	for (unsigned int i = 0; i < HANABI_TOTAL_NUMBERS; i++)
	{
		elements.give_clue_menu.number_button[i]->SetUserData(&data);
		elements.give_clue_menu.number_button[i]->SetOnClickUpCallback(give_clue_number_button_callback);
		elements.give_clue_menu.number_button[i]->SetAuxData(i);
	}
	//Give clue OK button
	elements.give_clue_menu.give_clue_button->SetUserData(&data);
	elements.give_clue_menu.give_clue_button->SetOnClickUpCallback(give_clue_ok_button_callback);

	//error button
	elements.error_menu.ok_button->SetUserData(&data);
	elements.error_menu.ok_button->SetOnClickUpCallback(exit_game_now_button_callback);

	//Remote player left button
	elements.remote_player_left_menu.ok_button->SetUserData(&data);
	elements.remote_player_left_menu.ok_button->SetOnClickUpCallback(exit_game_now_button_callback);

	//Play again button
	elements.game_finished_menu.play_again_button->SetUserData(&data);
	elements.game_finished_menu.play_again_button->SetOnClickUpCallback(play_again_button_callback);

	//Game over button
	elements.game_finished_menu.quit_button->SetUserData(&data);
	elements.game_finished_menu.quit_button->SetOnClickUpCallback(game_over_button_callback);
}
//Initialize card deck
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

//# Gui Callbacks! #
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
bool give_clue_color_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw) 
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		//Got clue!
		data->got_clue = true;
		//Deselect ALL clue color buttons
		for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
			data->elements.give_clue_menu.color_button[i]->SetUseTopBitmap(false);
		//Deselect ALL clue number buttons
		for (unsigned int i = 0; i < HANABI_TOTAL_NUMBERS; i++)
			data->elements.give_clue_menu.number_button[i]->SetUseTopBitmap(false);
		//Clue is color
		data->local_event_clue_is_color = true;
		data->local_event_clue_id = aux_data;
		//Select current button
		source->SetUseTopBitmap(true);
		//Redraw
		(*redraw) = true;
	}
	return false; 
}
bool give_clue_number_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		//Got clue!
		data->got_clue = true;
		//Deselect ALL clue color buttons
		for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
			data->elements.give_clue_menu.color_button[i]->SetUseTopBitmap(false);
		//Deselect ALL clue number buttons
		for (unsigned int i = 0; i < HANABI_TOTAL_NUMBERS; i++)
			data->elements.give_clue_menu.number_button[i]->SetUseTopBitmap(false);
		//Clue is not color, it is number
		data->local_event_clue_is_color = false;
		data->local_event_clue_id = aux_data;
		//Select current button
		source->SetUseTopBitmap(true);
		//Redraw
		(*redraw) = true;
	}
	return false;
}
bool give_clue_ok_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		if (data->got_clue)
		{
			data->local_event_queue.push(USER_GIVE_CLUE);
		}
		else
		{
			data->elements.message2->SetText("You must select a clue!");
			data->elements.message2->SetIsVisible(true);
			(*redraw) = true;
		}
	}
	return false;
}
bool exit_menu_quit_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		source->ReleaseMouse();
		data->local_event_queue.push(USER_QUIT);
	}
	return mouse_over_element;	//true will close menu
}
bool exit_game_now_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		data->local_event_queue.push(USER_EXIT_GAME_NOW);
	}
	return mouse_over_element;	//true will close menu
}
bool play_again_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		data->local_event_queue.push(USER_PLAY_AGAIN);
	}
	return mouse_over_element;	//true will close menu
}
bool game_over_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, unsigned int aux_data, bool* redraw)
{
	if (mouse_over_element && !forced)
	{
		game_data *data = (game_data*)user_data;
		data->local_event_queue.push(USER_GAME_OVER);
	}
	return mouse_over_element;	//true will close menu
}



//## HANABI FINITE STATE MACHINE ##

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
static const STATE* fsm_handler(const STATE * current_state, fsm_event_T ev, game_data& data, Package_hanabi* package)
{
	const STATE* st = current_state;

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
		st->action(data, package);
		st = st->next_state;
		//In common event table, nullptr means stay in same state
		return (st == nullptr ? current_state : st);
}

//# Actions prototypes #

//Notation: state____event ; HERE WE HAVE ALL ACTIONS!
static void fsm_start_point____server(game_data& data, Package_hanabi* package);										//Done!
static void fsm_start_point____client(game_data& data, Package_hanabi* package);										//Done!
static void s_wait_nameis____nameis(game_data& data, Package_hanabi* package);											//Done!
static void s_wait_name____name(game_data& data, Package_hanabi* package);												//Done!
static void s_wait_nameis_ack____ack(game_data& data, Package_hanabi* package);											//Done!
static void s_wait_nameis_ack____timeout(game_data& data, Package_hanabi* package);										//Done!
static void c_wait_name____name(game_data& data, Package_hanabi* package);												//Done!
static void c_wait_nameis_ack____ack(game_data& data, Package_hanabi* package);											//Done!
static void c_wait_nameis_ack____timeout(game_data& data, Package_hanabi* package);										//Done!				
static void c_wait_nameis____nameis(game_data& data, Package_hanabi* package);											//Done!
static void c_wait_start_info____start_info(game_data& data, Package_hanabi* package);									//Done!
static void wait_start_info_ack____ack(game_data& data, Package_hanabi* package);										//Done!
static void wait_start_info_ack____timeout(game_data& data, Package_hanabi* package);									//Done!
static void wait_software_who____sw_who_i(game_data& data, Package_hanabi* package);									//Done!
static void wait_software_who____sw_who_you(game_data& data, Package_hanabi* package);									//Done!
static void wait_i_start_ack____ack(game_data& data, Package_hanabi* package);											//Done!
static void wait_i_start_ack____timeout(game_data& data, Package_hanabi* package);										//Done!
static void wait_who____i_start(game_data& data, Package_hanabi* package);												//Done!
static void wait_who____you_start(game_data& data, Package_hanabi* package);											//Done!
static void a_wait_remote_play_again_answer____remote_pa(game_data& data, Package_hanabi* package);						//Done!
static void a_wait_remote_play_again_answer____remote_go(game_data& data, Package_hanabi* package);						//Done!
static void a_wait_remote_play_again_answer____local_pa(game_data& data, Package_hanabi* package);						//Done!
static void a_wait_remote_play_again_answer____local_go(game_data& data, Package_hanabi* package);						//Done!
static void a_wait_remote_play_again_answer_local_pa____remote_pa(game_data& data, Package_hanabi* package);			//Done!
static void a_wait_remote_play_again_answer_local_pa____remote_go(game_data& data, Package_hanabi* package);			//Done!
static void a_wait_remote_play_again_answer_local_pa____remote_pa(game_data& data, Package_hanabi* package);			//Done!
static void a_wait_remote_play_again_answer_local_pa____remote_go(game_data& data, Package_hanabi* package);			//Done!
static void a_wait_local_play_again_answer____local_pa(game_data& data, Package_hanabi* package);						//Done!
static void a_wait_local_play_again_answer____local_go(game_data& data, Package_hanabi* package);						//Done!
static void b_wait_local_play_again_answer____local_pa(game_data& data, Package_hanabi* package);						//Done!
static void b_wait_local_play_again_answer____local_go(game_data& data, Package_hanabi* package);						//Done!
static void b_wait_remote_play_again_answer____start_info(game_data& data, Package_hanabi* package);					//Done!
static void b_wait_remote_play_again_answer____remote_go(game_data& data, Package_hanabi* package);						//Done!
static void local_player_turn____local_give_clue(game_data& data, Package_hanabi* package);								//Done!
static void local_player_turn____local_play(game_data& data, Package_hanabi* package);									//Done!
static void local_player_turn____local_discard(game_data& data, Package_hanabi* package);								//Done!
static void wait_remote_player_response____ack(game_data& data, Package_hanabi* package);								//Done!
static void wait_remote_player_response____remote_we_won(game_data& data, Package_hanabi* package);						//Done!
static void wait_remote_player_response____remote_we_lost(game_data& data, Package_hanabi* package);					//Done!
static void wait_remote_player_response____timeout(game_data& data, Package_hanabi* package);							//Done!
static void wait_sw_draw____sw_draw_next(game_data& data, Package_hanabi* package);										//Done!
static void wait_sw_draw____sw_draw_last(game_data& data, Package_hanabi* package);										//Done!
static void remote_player_turn____remote_give_clue(game_data& data, Package_hanabi* package);							//Done!
static void remote_player_turn____remote_play(game_data& data, Package_hanabi* package);								//Done!
static void remote_player_turn____remote_discard(game_data& data, Package_hanabi* package);								//Done!
static void remote_player_turn____remote_play_won(game_data& data, Package_hanabi* package);							//Done!
static void remote_player_turn____remote_play_lost(game_data& data, Package_hanabi* package);							//Done!
static void wait_draw____draw_next(game_data& data, Package_hanabi* package);											//Done!
static void wait_draw____draw_last(game_data& data, Package_hanabi* package);											//Done!
static void wait_draw____timeout(game_data& data, Package_hanabi* package);												//Done!
static void sc_1_remote_player_turn____remote_give_clue(game_data& data, Package_hanabi* package);						//Done!
static void sc_1_remote_player_turn____remote_play(game_data& data, Package_hanabi* package);							//Done!
static void sc_1_remote_player_turn____remote_discard(game_data& data, Package_hanabi* package);						//Done!
static void sc_1_remote_player_turn____remote_play_won(game_data& data, Package_hanabi* package);						//Done!
static void sc_1_remote_player_turn____remote_play_lost(game_data& data, Package_hanabi* package);						//Done!
static void sc_1_wait_draw____draw_fake(game_data& data, Package_hanabi* package);										//Done!
static void sc_1_wait_draw____timeout(game_data& data, Package_hanabi* package);										//Done!
static void sc_1_local_player_turn____local_give_clue(game_data& data, Package_hanabi* package);						//WHAT SHOULD BE DONE HERE?
static void sc_1_local_player_turn____local_play(game_data& data, Package_hanabi* package);								//Done!
static void sc_1_local_player_turn____local_discard(game_data& data, Package_hanabi* package);							//Done!
static void sc_1_wait_remote_player_response____remote_we_won(game_data& data, Package_hanabi* package);				//Done!
static void sc_1_wait_remote_player_response____remote_we_lost(game_data& data, Package_hanabi* package);				//Done!
static void sc_1_wait_remote_player_response____remote_match_is_over(game_data& data, Package_hanabi* package);			//Done!
static void sc_1_wait_remote_palyer_response____timeout(game_data& data, Package_hanabi* package);						//Done!
static void sc_2_local_player_turn____local_give_clue(game_data& data, Package_hanabi* package);						//Done!
static void sc_2_local_player_turn____local_play(game_data& data, Package_hanabi* package);								//Done!
static void sc_2_local_player_turn____local_discard(game_data& data, Package_hanabi* package);							//Done!
static void sc_2_wait_remote_player_response____ack(game_data& data, Package_hanabi* package);							//Done!
static void sc_2_wait_remote_player_response____remote_we_won(game_data& data, Package_hanabi* package);				//Done!
static void sc_2_wait_remote_player_response____remote_we_lost(game_data& data, Package_hanabi* package);				//Done!
static void sc_2_wait_remote_player_response____timeout(game_data& data, Package_hanabi* package);						//Done!
static void sc_2_remote_player_turn____remote_give_clue(game_data& data, Package_hanabi* package);						//WHAT SHOULD BE DONE HERE?
static void sc_2_remote_player_turn____remote_discard(game_data& data, Package_hanabi* package);						//Done!
static void sc_2_remote_player_turn____remote_play(game_data& data, Package_hanabi* package);							//Done!
static void sc_2_remote_player_turn____remote_play_won(game_data& data, Package_hanabi* package);						//Done!
static void sc_2_remote_player_turn____remote_play_lost(game_data& data, Package_hanabi* package);						//Done!
static void local_player_quit____ack(game_data& data, Package_hanabi* package);											//Done!
static void end_state____end_game(game_data& data, Package_hanabi* package);											//Done!
static void common____error_ev(game_data& data, Package_hanabi* package);												//Done!
static void common____bad(game_data& data, Package_hanabi* package);													//Done!
static void common____quit(game_data& data, Package_hanabi* package);													//Done!
static void common____local_quit(game_data& data, Package_hanabi* package);												//Done!

//# States prototypes #

//extern const STATE fsm_start_point[]; Defined above!
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
extern const STATE a_wait_remote_play_again_answer_local_pa[];
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
//extern const STATE end_state[];	//Defined above!


//# Useful functions for FSM actions #

//Prototypes
static void new_game(game_data& data);
static void game_finished(game_data& data);
static void reveal_local_cards(game_data& data);
static void local_player_turn_starts(game_data& data);
static void local_player_turn_ends(game_data& data);
static void remote_player_turn_starts(game_data& data);
static void remote_player_turn_ends(game_data& data);
static void we_are_the_champions_message(game_data& data);
static void we_lost_message(game_data& data);
static void match_is_over_message(game_data& data);
static void discard_card(const card& c, game_data& data);
static bool play_card(const card& c, game_data& data);
static void start_timeout_count(game_data& data);
static void abort_timeout_count(game_data& data);
static void timeout(game_data& data, Package_hanabi* package);
static bool send_package(Package_hanabi& package, Net_connection* connection);

//Called every time a new game should start
static void new_game(game_data& data)
{
	//Initialize card deck
	initialize_deck(data.card_deck);
	//Clues to maximum
	data.clues = HANABI_TOTAL_CLUE_INDICATORS;
	//No lightning
	data.lightnings = 0;
	//No card discarded
	data.discard_count = 0;
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
		data.color_stack[i] = 0;		
	//Show card deck full
	data.elements.deck->SetUseSecondBitmap(true);
	//Enable discarded cards button
	data.elements.discarded_cards.open_button->SetIsActive(true);
	data.elements.discarded_cards.open_button->SetIsVisible(true);
	//Hide messages
	data.elements.message->SetIsVisible(false);
	data.elements.message2->SetIsVisible(false);
}
//Called every time a game finishes
static void game_finished(game_data& data)
{
	//Hide give clue menu
	data.elements.give_clue_menu.menu->SetIsActive(false);
	data.elements.give_clue_menu.menu->SetIsVisible(false);
	//Set all local player cards to visible
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		data.elements.player.local.player_card[i]->SetIsVisible(true);
	//Set all remote player cards to visible
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		data.elements.player.remote.player_card[i]->SetIsVisible(true);
	//Hide local player cards
	data.elements.player.local.cards_menu->SetIsActive(false);
	data.elements.player.local.cards_menu->SetIsVisible(false);
	//Hide remote player cards
	data.elements.player.remote.cards_menu->SetIsVisible(false);
	//Hide discard cards button
	data.elements.discard_card_button->SetIsActive(false);
	data.elements.discard_card_button->SetIsVisible(false);
	//Disable show discarded cards button
	data.elements.discarded_cards.open_button->SetIsActive(false);
	//Hide discarded cards
	data.elements.discarded_cards.menu->SetIsVisible(false);
	data.elements.discarded_cards.menu->SetIsActive(false);
	//Show no card in discarded cards top
	data.elements.discarded_cards.open_button->SetUseTopBitmap(false);
	//Remove discarded cards
	for (unsigned int i = 0; i < HANABI_TOTAL_CARDS; i++)
		data.elements.discarded_cards.cards[i]->SetUseSecondBitmap(false);
	//Remote cards from stacks
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS; c++)
		for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS; n++)
			data.elements.center_cards[c][n]->SetIsVisible(false);
	//Show deck cards empty
	data.elements.deck->SetUseSecondBitmap(false);
	//Enable all clue indicators
	for (unsigned int i = 0; i < HANABI_TOTAL_CLUE_INDICATORS; i++)
		data.elements.indicators.clue_indicators[i]->SetUseTopBitmap(false);
	//Disable all lightning indicators
	for (unsigned int i = 0; i < HANABI_TOTAL_LIGHTNING_INDICATORS; i++)
		data.elements.indicators.lightning_indicators[i]->SetUseTopBitmap(false);
	//Hide messages
	data.elements.message->SetIsVisible(false);
	data.elements.message2->SetIsVisible(false);
	//Redraw
	data.redraw = true;
}
//Show local player cards!
static void reveal_local_cards(game_data& data)
{
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		if (data.local_player_card[i] != card(NO_COLOR, NO_NUMBER))
			//Show card!
			data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(data.local_player_card[i]));
	data.redraw = true;
}
//Each time local player turn starts and ends
static void local_player_turn_starts(game_data& data)
{
	//Log:
	cout << "[GAME_HANDLER][LOG] : Local player turn! his cards: ";
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		cout << "[" << data.local_player_card[i].get_short_name() << "]";
	cout << endl;
	//Enable local player cards
	data.elements.player.local.cards_menu->SetIsActive(true);
	//If got clues, show give clue menu
	if (data.clues != 0)
	{
		data.elements.give_clue_menu.menu->SetIsVisible(true);
		data.elements.give_clue_menu.menu->SetIsActive(true);
	}
	//Discard card button
	data.elements.discard_card_button->SetIsVisible(true);
	data.elements.discard_card_button->SetIsActive(true);
	//Inform user
	data.elements.message->SetText("It's your turn.");
	data.elements.message->SetIsVisible(true);
	//Set discard mode to false
	data.discard_mode_enabled = false;
	//No clue, yet
	data.got_clue = false;
	//Deselect ALL clue color buttons
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
		data.elements.give_clue_menu.color_button[i]->SetUseTopBitmap(false);
	//Deselect ALL clue number buttons
	for (unsigned int i = 0; i < HANABI_TOTAL_NUMBERS; i++)
		data.elements.give_clue_menu.number_button[i]->SetUseTopBitmap(false);
	//Redraw
	data.redraw = true;
}
static void local_player_turn_ends(game_data& data)
{
	//Release mouse in player cards
	data.elements.player.local.cards_menu->ReleaseMouse();
	//Deactivate local player cards
	data.elements.player.local.cards_menu->SetIsActive(false);
	//Hide give clue menu
	data.elements.give_clue_menu.menu->SetIsVisible(false);
	data.elements.give_clue_menu.menu->SetIsActive(false);
	data.got_clue = false;
	//Hide discard card button
	data.elements.discard_card_button->SetIsVisible(false);
	data.elements.discard_card_button->SetIsActive(false);
	data.elements.discard_card_button->SetUseTopBitmap(false);
	data.discard_mode_enabled = false;
	//Remove text
	data.elements.message->SetText("");
	data.elements.message->SetIsVisible(false);
	//Redraw
	data.redraw = true;
}
//Each time remote player turn starts and ends
static void remote_player_turn_starts(game_data& data)
{
	//Log:
	cout << "[GAME_HANDLER][LOG] : Remote player turn! his cards: ";
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		cout << "[" << data.remote_player_card[i].get_short_name() << "]";
	cout << endl;
	//Inform local player it is remote player turns
	data.elements.message->SetText("It's your friend's turn.");
	data.elements.message->SetIsVisible(true);
	data.redraw = true;
}
static void remote_player_turn_ends(game_data& data)
{
	//Remove turn message
	data.elements.message->SetText("");
	data.elements.message->SetIsVisible(false);
	data.redraw = true;
}
//When game is won, lost or match is over
static void we_are_the_champions_message(game_data& data)
{
	unsigned int score = 0;
	char score_ascii[3];
	//Show game finished menu
	data.elements.game_finished_menu.menu->SetIsVisible(true);
	data.elements.game_finished_menu.menu->SetIsActive(true);
	data.elements.game_finished_menu.title->SetText("YOU WON!! Your score:");
	data.elements.game_finished_menu.title->SetIsVisible(true);
	//Calculate score (yep, it should be 50)
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
		score += data.color_stack[i];
	cout << "[GAME_HANDLER][LOG] : Game WON! score: " << score << endl;
	//Show it
	itoa(score, score_ascii, 10);
	data.elements.game_finished_menu.score->SetText(score_ascii);
	data.elements.game_finished_menu.score->SetIsVisible(true);
	//Show International Association of Pyrotechnics (aka IAP) message
	string iap_message = MESSAGE_LEVEL_5;
	data.elements.game_finished_menu.score_message->SetText(iap_message);
	data.elements.game_finished_menu.score_message->SetIsVisible(true);
	data.redraw = true;
	data.feedback_event = FB_NO_EVENT;
	//Menu has option quit, so..
	data.quit_button_enabled = false;
}
static void we_lost_message(game_data& data)
{
	//Show game finished menu
	data.elements.game_finished_menu.menu->SetIsVisible(true);
	data.elements.game_finished_menu.menu->SetIsActive(true);
	data.elements.game_finished_menu.title->SetText("You lost! :(");
	data.elements.game_finished_menu.title->SetIsVisible(true);
	data.elements.game_finished_menu.score->SetIsVisible(false);
	data.elements.game_finished_menu.score_message->SetText(MESSAGE_LOST);
	data.elements.game_finished_menu.score_message->SetIsVisible(true);
	data.redraw = true;
	data.feedback_event = FB_NO_EVENT;
	cout << "[GAME_HANDLER][LOG] : Game LOST! " << endl;
	//Menu has option quit, so..
	data.quit_button_enabled = false;
}
static void match_is_over_message(game_data& data)
{
	unsigned int score = 0;
	char score_ascii[3];
	//Show game finished menu
	data.elements.game_finished_menu.menu->SetIsVisible(true);
	data.elements.game_finished_menu.menu->SetIsActive(true);
	data.elements.game_finished_menu.title->SetText("Match is over. Your score:");
	data.elements.game_finished_menu.title->SetIsVisible(true);
	//Calculate score
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
		score += data.color_stack[i];
	cout << "[GAME_HANDLER][LOG] : Match is over! score: " << score << endl;
	//Show it
	itoa(score, score_ascii, 10);
	data.elements.game_finished_menu.score->SetText(score_ascii);
	data.elements.game_finished_menu.score->SetIsVisible(true);
	//Show International Association of Pyrotechnics (aka IAP) message
	string iap_message;
	if (score <= MAX_LEVEL_0)
		iap_message = MESSAGE_LEVEL_0;
	else if (score <= MAX_LEVEL_1)
		iap_message = MESSAGE_LEVEL_1;
	else if (score <= MAX_LEVEL_2)
		iap_message = MESSAGE_LEVEL_2;
	else if (score <= MAX_LEVEL_3)
		iap_message = MESSAGE_LEVEL_3;
	else if (score <= MAX_LEVEL_4)
		iap_message = MESSAGE_LEVEL_4;
	else if (score <= MAX_LEVEL_5)
		iap_message = MESSAGE_LEVEL_5;
	data.elements.game_finished_menu.score_message->SetText(iap_message);
	data.elements.game_finished_menu.score_message->SetIsVisible(true);
	data.redraw = true;
	data.feedback_event = FB_NO_EVENT;
	//Menu has option quit, so..
	data.quit_button_enabled = false;
}
//Action discard card!
static void discard_card(const card& c, game_data& data)
{
	cout << "[GAME_HANDLER][LOG] : Discarded [" << c.get_short_name() << "]" << endl;
	data.elements.discarded_cards.open_button->SetTopBitmap(data.skin.get_bitmap(c));
	data.elements.discarded_cards.open_button->SetUseTopBitmap(true);
	data.elements.discarded_cards.cards[data.discard_count]->SetSecondBitmap(data.skin.get_bitmap(c));
	data.elements.discarded_cards.cards[data.discard_count++]->SetUseSecondBitmap(true);
	data.redraw = true;
}
//Action send card!
static bool play_card(const card& c, game_data& data)
{
	bool success;
	cout << "[GAME_HANDLER][LOG] : Played [" << c.get_short_name() << "]";
	if (data.color_stack[c.get_color()] == c.get_number())
	{
		if ((++data.color_stack[c.get_color()]) == HANABI_TOTAL_NUMBERS && data.clues != HANABI_TOTAL_CLUE_INDICATORS)
			data.elements.indicators.clue_indicators[data.clues++]->SetUseTopBitmap(false);
		data.elements.center_cards[c.get_color()][c.get_number()]->SetDefaultBitmap(data.skin.get_bitmap(c));
		data.elements.center_cards[c.get_color()][c.get_number()]->SetIsVisible(true);
		data.redraw = true;
		cout << ", It's Super Effective!" << endl;
		success = true;
	}
	else
	{
		cout << ", It's not very effective..." << endl;
		discard_card(c, data);
		if(data.lightnings != HANABI_TOTAL_LIGHTNING_INDICATORS)
			data.elements.indicators.lightning_indicators[data.lightnings++]->SetUseTopBitmap(true);
		data.redraw = true;
		success = false;
	}
	cout << "[GAME_HANDLER][LOG] : Clue indicators: " << data.clues << ", Lightnings: " << data.lightnings << endl;
	return success;
}
//Timeout
static void start_timeout_count(game_data& data)
{
	al_start_timer(data.timer);
}
static void abort_timeout_count(game_data& data)
{
	al_stop_timer(data.timer);
}
static void timeout(game_data& data, Package_hanabi* package)
{
	cerr << "[GAME_HANDLER][ERROR] : TIMEOUT!" << endl;
	//Disable timer
	abort_timeout_count(data);
	//Same as bad
	common____bad(data, package);
}

//Send a package to remote player
static bool send_package(Package_hanabi& package, Net_connection* connection)
{
	char raw_data[MAX_PACKAGE_SIZE];
	size_t data_size;
	size_t sent_bytes;
	package.get_raw_data(raw_data, MAX_PACKAGE_SIZE, &data_size);
	//Next line is the only line that sends data to remote player!
	return (connection->send_data(raw_data, data_size, &sent_bytes) && data_size == sent_bytes);
}

//<-- While NOT in game!!! -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//NOTE THAT THERE ARE ACTION AND STATE GROUPS!

//$ Action group: START $
static void fsm_start_point____server(game_data& data, Package_hanabi* package)
{
	//Server must break the ice. A good way to start is asking for her name.
	Package_name p;
	if(send_package(p,data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	//Should not enable timeout here
	//For compatibility with other implementations
}
static void fsm_start_point____client(game_data& data, Package_hanabi* package)
{
	//DO NOTHING.
	data.feedback_event = FB_NO_EVENT;
}
//% State group: START %
static const STATE fsm_start_point[] =
{
	{ SERVER,fsm_start_point____server,s_wait_nameis },
	{ CLIENT,fsm_start_point____client,c_wait_name },
	{ GND,nullptr,nullptr }
};

//$ Action group: HANDSHAKE, branch: Server $
static void s_wait_nameis____nameis(game_data& data, Package_hanabi* package)
{
	//Received remote name!
	string name;
	Package_name_is *rec_p;
	//Read package..
	if ((rec_p = dynamic_cast<Package_name_is*>(package)) != nullptr)
	{
		Package_ack p;
		//Read name
		rec_p->get_name(name);
		//Show it
		data.elements.player.remote.name->SetText(name);
		data.elements.player.remote.name->SetIsVisible(true);
		//Send ACK
		if (send_package(p, data.connection))
			data.feedback_event = FB_NO_EVENT;
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void s_wait_name____name(game_data& data, Package_hanabi* package)
{
	//Received name request! send name_is with my name
	Package_name_is p;
	//Add name to package
	p.set_name(data.user_name);
	//Send it
	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		//Name is will wait ack
		//Enable timeout timer
		start_timeout_count(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void s_wait_nameis_ack____ack(game_data& data, Package_hanabi* package)
{

	Package_start_info p;
	//Got ack. disable timer
	abort_timeout_count(data);
	//Sending start info, so new game will start
	//prepare data for new game
	new_game(data);
	for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
	{
		//One card for me
		data.local_player_card[i] = data.card_deck.pick_top();
		//One card for you
		data.remote_player_card[i] = data.card_deck.pick_top();
		//See cards
		data.elements.player.remote.player_card[i]->SetDefaultBitmap(data.skin.get_bitmap(data.remote_player_card[i]));
#ifdef CHEAT_SHOW_LOCAL_CARDS
		data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(data.local_player_card[i]));
#else
		data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(card(NO_COLOR,NO_NUMBER)));
#endif
	}
	data.elements.player.local.cards_menu->SetIsVisible(true);
	data.elements.player.remote.cards_menu->SetIsVisible(true);
	p.set_info(data.remote_player_card, data.local_player_card);
	//Send data
	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		//Start info will wait ack
		//Enable timeout timer
		start_timeout_count(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void s_wait_nameis_ack____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: HANDSHAKE, branch: Server %
static const STATE s_wait_nameis[] =
{
	{ NAMEIS,s_wait_nameis____nameis,s_wait_name },
	{ GND,nullptr,nullptr}
};
static const STATE s_wait_name[] =
{
	{ NAME,s_wait_name____name,s_wait_nameis_ack },
	{ GND,nullptr,nullptr}
};
static const STATE s_wait_nameis_ack[] =
{
	{ TIMEOUT, s_wait_nameis_ack____timeout, end_state},		//break
	{ ACK,s_wait_nameis_ack____ack,wait_start_info_ack },		// --> GO TO INITIALIZATION
	{ GND,nullptr,nullptr }
};

//$ Action group: HANDSHAKE, branch: Client $
static void c_wait_name____name(game_data& data, Package_hanabi* package)
{
	s_wait_name____name(data, package);	//Same as server
}
static void c_wait_nameis_ack____ack(game_data& data, Package_hanabi* package)
{
	//Got ack, dissable timer
	abort_timeout_count(data);
	fsm_start_point____server(data, package);	//Same as fsm_start_point____server
}
static void c_wait_nameis_ack____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
static void c_wait_nameis____nameis(game_data& data, Package_hanabi* package)
{
	s_wait_nameis____nameis(data, package);	//Same as server
	//Will not start timer for start info
	//For compatibility with other implementations
}
static void c_wait_start_info____start_info(game_data& data, Package_hanabi* package)
{
	//Received start info!
	//So new game will start.
	Package_start_info *rec_p;
	//Prepare data for new game
	new_game(data);
	if ((rec_p = dynamic_cast<Package_start_info*>(package)) != nullptr)
	{
		Package_ack p;

		//Load cards
		rec_p->get_info(data.local_player_card, data.remote_player_card);
		//Show remote cards!
		for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
		{
			data.elements.player.remote.player_card[i]->SetDefaultBitmap(data.skin.get_bitmap(data.remote_player_card[i]));
#ifdef CHEAT_SHOW_LOCAL_CARDS
			data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(data.local_player_card[i]));
#else
			data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(card(NO_COLOR,NO_NUMBER)));
#endif
			data.card_deck.remove_card(data.local_player_card[i]);
			data.card_deck.remove_card(data.remote_player_card[i]);
		}
		//Now, local cards and remote cards are visible.
		data.elements.player.local.cards_menu->SetIsVisible(true);
		data.elements.player.remote.cards_menu->SetIsVisible(true);

		//Send ACK
		if (send_package(p, data.connection))
		{
			data.feedback_event = FB_NO_EVENT;
			//Wait who starts!
			start_timeout_count(data);
		}
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
//% State group: HANDSHAKE, branch: Client %
static const STATE c_wait_name[] =
{
	{ NAME,c_wait_name____name,c_wait_nameis_ack },
	{ GND,nullptr,nullptr }
};
static const STATE c_wait_nameis_ack[] =
{
	{ ACK,c_wait_nameis_ack____ack,c_wait_nameis },
	{ TIMEOUT,c_wait_nameis_ack____timeout,end_state },		//Break
	{ GND,nullptr,nullptr }
};
static const STATE c_wait_nameis[] =
{
	{ NAMEIS,c_wait_nameis____nameis,c_wait_start_info },
	{ GND,nullptr,nullptr }
};
static const STATE c_wait_start_info[] =
{
	{ START_INFO,c_wait_start_info____start_info,wait_who },	// --> GO TO INITIALIZATION
	{ GND,nullptr,nullptr }
};

//$ Action group: INITIALIZATION, branch: MACHINE_A $
static void wait_start_info_ack____ack(game_data& data, Package_hanabi* package)
{
	//Got ack. disable timer
	abort_timeout_count(data);
	//Ask software to decide who should start the game
	data.feedback_event = FB_WHO;
}
static void wait_start_info_ack____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
static void wait_software_who____sw_who_i(game_data& data, Package_hanabi* package)
{
	//Software decided local player starts.
	//Send I start
	Package_i_start p;
	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		//Must wait ack
		start_timeout_count(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void wait_software_who____sw_who_you(game_data& data, Package_hanabi* package)
{
	//Software decided remote player starts.
	//Send you start
	Package_you_start p;
	if (send_package(p, data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	cout << "[GAME_HANDLER][LOG] : Game started!" << endl;
	remote_player_turn_starts(data);
}
static void wait_i_start_ack____ack(game_data& data, Package_hanabi* package)
{
	//Got ack. disable timer
	abort_timeout_count(data);
	cout << "[GAME_HANDLER][LOG] : Game started!" << endl;
	//Local player turn!!
	local_player_turn_starts(data);
	data.feedback_event = FB_NO_EVENT;
}
static void wait_i_start_ack____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: INITIALIZATION, branch: MACHINE_A %
static const STATE wait_start_info_ack[] =
{
	{ ACK,wait_start_info_ack____ack,wait_software_who },
	{ TIMEOUT,wait_start_info_ack____timeout,end_state },		//Break
	{ GND,nullptr,nullptr }
};
static const STATE wait_software_who[] =
{
	{ SW_WHO_I,wait_software_who____sw_who_i,wait_i_start_ack },
	{ SW_WHO_YOU,wait_software_who____sw_who_you,remote_player_turn },	//  --> GO TO GAME
	{ GND,nullptr,nullptr }
};
static const STATE wait_i_start_ack[] =
{
	{ ACK,wait_i_start_ack____ack,local_player_turn },	// --> GO TO GAME
	{ TIMEOUT, wait_i_start_ack____timeout,end_state},		//break
	{ GND,nullptr,nullptr }
};

//$ Action group: INITIALIZATION, branch: MACHINE_B $
static void wait_who____i_start(game_data& data, Package_hanabi* package)
{	
	//Remote player turn starts
	//Remember remote says who, and he told me "I start"
	Package_ack p;
	//Got who!
	abort_timeout_count(data);
	//Send ack
	if (send_package(p, data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	//Remote turn
	cout << "[GAME_HANDLER][LOG] : Game started!" << endl;
	remote_player_turn_starts(data);
}
static void wait_who____you_start(game_data& data, Package_hanabi* package)
{
	//Local player turn starts
	//Remember remote says who, and he told me "YOU start"
	//Got who!
	abort_timeout_count(data);
	cout << "[GAME_HANDLER][LOG] : Game started!" << endl;
	local_player_turn_starts(data);
	data.feedback_event = FB_NO_EVENT;
}
//% State group: INITIALIZATION, branch: MACHINE_B %
static const STATE wait_who[] =
{
	{ I_START,wait_who____i_start,remote_player_turn },
	{ YOU_START,wait_who____you_start,local_player_turn },	// --> GO TO GAME
	{ GND,nullptr,nullptr }
};

//$ Action group: END OF GAME, branch: A:INFORMER (The one who informed game result) $
static void a_wait_remote_play_again_answer____remote_pa(game_data& data, Package_hanabi* package)
{
	//Do nothing, wait for local player...
	data.feedback_event = FB_NO_EVENT;
}
static void a_wait_remote_play_again_answer____remote_go(game_data& data, Package_hanabi* package)
{
	//Same as quit
	common____quit(data, package);
}
static void a_wait_remote_play_again_answer____local_pa(game_data& data, Package_hanabi* package) 
{
	game_finished(data);
	data.elements.message->SetText("Asking remote player if he wants to play again...");
	data.elements.message->SetIsVisible(true);
	data.elements.message2->SetIsVisible(false);
	//Menu with option quit will close, so...
	data.quit_button_enabled = true;
	data.redraw = true;
	//Wait for remote...
	data.feedback_event = FB_NO_EVENT;
}
static void a_wait_remote_play_again_answer____local_go(game_data& data, Package_hanabi* package) 
{
	game_finished(data);
	//I am waiting for remote player... but I do not want to play again!!
	//Why wait? just quit.
	//Same as local quit
	common____local_quit(data, package);
}
static void a_wait_remote_play_again_answer_local_pa____remote_pa(game_data& data, Package_hanabi* package)
{
	//Inform we want to play again!
	a_wait_local_play_again_answer____local_pa(data, package);
}
static void a_wait_remote_play_again_answer_local_pa____remote_go(game_data& data, Package_hanabi* package)
{
	//Same as quit
	common____quit(data, package);
}
static void a_wait_local_play_again_answer____local_pa(game_data& data, Package_hanabi* package)
{
	//We are here because local player wants to play again!
	game_finished(data);
	data.redraw = true;
	data.elements.message2->SetText("Please wait...");
	data.elements.message2->SetIsVisible(true);
	//We must send start info. This is done here:
	s_wait_nameis_ack____ack(data, nullptr);
	//Now must wait ack...
	start_timeout_count(data);
	//Menu with option quit will close, so...
	data.quit_button_enabled = true;
}
static void a_wait_local_play_again_answer____local_go(game_data& data, Package_hanabi* package)
{
	//We are here because local player does not want to keep on playing
	//Send game over package
	Package_game_over p;

	//Game finished
	game_finished(data);

	//Show quiting message
	data.elements.message->SetText("Quiting...");
	data.elements.message->SetIsVisible(true);
	data.elements.message2->SetIsVisible(false);
	data.quit_button_enabled = false;
	data.redraw = true;
	//Send package
	if (send_package(p, data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	//Now must wait ack...
}
//% State group: END OF GAME, branch: A:INFORMER (The one who informed game result) %
static const STATE a_wait_remote_play_again_answer[] =
{
	{ REMOTE_PA,a_wait_remote_play_again_answer____remote_pa,a_wait_local_play_again_answer },
	{ REMOTE_GO,a_wait_remote_play_again_answer____remote_go,end_state },				//break
	{ LOCAL_PA,a_wait_remote_play_again_answer____local_pa,a_wait_remote_play_again_answer_local_pa },
	{ LOCAL_GO,a_wait_remote_play_again_answer____local_go,local_player_quit },		//wait ack and break
	{ GND,nullptr,nullptr }
};
static const STATE a_wait_remote_play_again_answer_local_pa[] =
{
	{ REMOTE_PA,a_wait_remote_play_again_answer_local_pa____remote_pa,wait_start_info_ack },
	{ REMOTE_GO,a_wait_remote_play_again_answer_local_pa____remote_go,end_state },	//break
};
static const STATE a_wait_local_play_again_answer[] =
{
	{ LOCAL_PA,a_wait_local_play_again_answer____local_pa,wait_start_info_ack },		//  -->> GO TO INITIALIZATION
	{ LOCAL_GO,a_wait_local_play_again_answer____local_go,local_player_quit },		//wait ack and break
	{ GND,nullptr,nullptr }
};

//$ Action group: END OF GAME, branch: B:INFORMED (The one who received information about game result) $
static void b_wait_local_play_again_answer____local_pa(game_data& data, Package_hanabi* package)
{
	//We are here because local player wants to keep on playing
	//Send play again
	Package_play_again p;

	//Game finished
	game_finished(data);

	//Show waiting message
	data.elements.message->SetText("Asking remote player if he wants to play again...");
	data.elements.message->SetIsVisible(true);
	data.elements.message2->SetIsVisible(false);
	data.quit_button_enabled = false;
	data.redraw = true;
	//Send
	if (send_package(p, data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	//Now must wait other player...
	//Menu with option quit will close, so...
	data.quit_button_enabled = true;
}
static void b_wait_local_play_again_answer____local_go(game_data& data, Package_hanabi* package)
{
	//We are here because local player does not want to keep on playing
	//Send game over package
	Package_game_over p;

	//Game finished
	game_finished(data);

	//Show quiting message
	data.elements.message->SetText("Quiting...");
	data.elements.message->SetIsVisible(true);
	data.elements.message2->SetIsVisible(false);
	data.quit_button_enabled = false;
	data.redraw = true;
	//Send
	if (send_package(p, data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	//Now must wait ack...
}
static void b_wait_remote_play_again_answer____start_info(game_data& data, Package_hanabi* package)
{
	//We are here because remote player wants to play again!
	game_finished(data);
	data.redraw = true;
	data.elements.message2->SetText("Please wait...");
	data.elements.message2->SetIsVisible(true);
	//We must process start info. This is done here:
	c_wait_start_info____start_info(data, package);
}
static void b_wait_remote_play_again_answer____remote_go(game_data& data, Package_hanabi* package)
{
	//Same as quit
	common____quit(data, package);
}
//% State group: END OF GAME, branch: B:INFORMED (The one who received information about game result) %
static const STATE b_wait_local_play_again_answer[] =
{
	{ LOCAL_PA,b_wait_local_play_again_answer____local_pa,b_wait_remote_play_again_answer },
	{ LOCAL_GO,b_wait_local_play_again_answer____local_go,local_player_quit}, //break
	{ GND,nullptr,nullptr }
};
static const STATE b_wait_remote_play_again_answer[] =
{
	{ START_INFO,b_wait_remote_play_again_answer____start_info,wait_who },		//  --> GO TO INITIALIZATION
	{ REMOTE_GO,b_wait_remote_play_again_answer____remote_go,end_state }, //break
	{ GND,nullptr,nullptr }
};

//<-- While in game -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//$ Action group: IN GAME, branch: LOCAL_PLAYER $
static void local_player_turn____local_give_clue(game_data& data, Package_hanabi* package)
{
	//We are here because local player wants to give a clue to remote player
	//Sice we are here, we KNOW we have clues. but let's check anyway

	//Local player turn finished
	local_player_turn_ends(data);
	if (data.clues != 0)
	{
		//Ok, we have got clues. give clue!
		Package_you_have p;
		string message = "You gave clue: ";
		string clue;
		//Prepare clue
		card c;
		if (data.local_event_clue_is_color)
		{
			c = card((card_color_t)data.local_event_clue_id, NO_NUMBER);	//Color?
			clue = color_string[data.local_event_clue_id];
		}
		else
		{
			c = card(NO_COLOR, (card_number_t)data.local_event_clue_id);	//Or number?
			clue = (char)('0' + data.local_event_clue_id + 1);
		}
		message += clue;

		cout << "[GAME_HANDLER][LOG] : CLUE: " << clue << endl;
		p.set_clue(c);

		//Show message
		data.elements.message2->SetText(message);
		data.elements.message2->SetIsVisible(true);
		data.redraw = true;


		//Don't forget: USE CLUE
		data.elements.indicators.clue_indicators[--data.clues]->SetUseTopBitmap(true);
		data.redraw = true;


		//Send clue!
		if (send_package(p, data.connection))
			data.feedback_event = FB_NO_EVENT;
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void local_player_turn____local_play(game_data& data, Package_hanabi* package)
{
	//We are here because local player played a card
	card c;
	string card_name, message;
	Package_play p;

	//Local player turn finished
	local_player_turn_ends(data);

	//Pick played card
	c = data.local_player_card[data.local_event_card_offset];

	//Generate string for card
	card_name = c.get_name();
	message = string("You played ") + card_name;

	//Play card and finish string
	if (play_card(c, data))
		message += " - SUCCESS!";
	else
		message += " - Card discarded :(";

	//Show message
	data.elements.message2->SetText(message.c_str());
	data.elements.message2->SetIsVisible(true);

	//Leave blank space where card was
	data.local_player_card[data.local_event_card_offset] = card(NO_COLOR,NO_NUMBER);
	data.elements.player.local.player_card[data.local_event_card_offset]->SetIsVisible(false);
	data.redraw = true;

	//Send play package
	p.set_card_id(data.local_event_card_offset);
	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		//After play, ack. So enable timer
		start_timeout_count(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void local_player_turn____local_discard(game_data& data, Package_hanabi* package)
{
	//We are here because local player discarded a card
	card c;
	string card_name, message;
	Package_discard p;

	//Local player turn finished
	local_player_turn_ends(data);

	//Pick discarded card
	c = data.local_player_card[data.local_event_card_offset];
	//Discard it
	discard_card(c, data);

	//Generate card string
	card_name = c.get_name();

	//Inform card was discarded
	message = string("You discarded ") + card_name;
	data.elements.message2->SetText(message.c_str());
	data.elements.message2->SetIsVisible(true);

	//Leave blank space where card was
	data.local_player_card[data.local_event_card_offset] = card(NO_COLOR,NO_NUMBER);
	data.elements.player.local.player_card[data.local_event_card_offset]->SetIsVisible(false);
	data.redraw = true;
	//Send discard package
	
	p.set_card_id(data.local_event_card_offset);
	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		//After discard, ack. So enable timer
		start_timeout_count(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void wait_remote_player_response____ack(game_data& data, Package_hanabi* package)
{
	//Got ack! disable timer
	abort_timeout_count(data);
	//Remote player ack our move. But wait... Didn't we won? Didn't we lost?
	//Check lightning indicators
	if (data.lightnings != HANABI_TOTAL_LIGHTNING_INDICATORS)
	{
		//Ok, we did not lose. Did we won? count played cards
		unsigned int played_cards = 0;
		for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
			played_cards += data.color_stack[i];
		//are color stacks complete?
		if (played_cards == HANABI_TOTAL_COLORS*HANABI_TOTAL_NUMBERS)
		{
			//WE WON! WE HAVE AN ERROR
			data.feedback_event = FB_ERROR;
		}
		else
			//Everything is OK!
			data.feedback_event = FB_DRAW;	//Next, draw a card
	}
	else
		//WE LOST! WE HAVE AN ERROR;
		data.feedback_event = FB_ERROR;
}
static void wait_remote_player_response____remote_we_won(game_data& data, Package_hanabi* package)
{
	//Equivalent to ack! disable timer
	abort_timeout_count(data);
	//Remote player says we won. Really??
	unsigned int played_cards = 0;
	for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
		played_cards += data.color_stack[i];
	//are color stacks complete?
	if (played_cards == HANABI_TOTAL_COLORS*HANABI_TOTAL_NUMBERS)
	{
		//If open, colapse discarded cards menu
		data.elements.discarded_cards.menu->SetIsActive(false);
		data.elements.discarded_cards.menu->SetIsVisible(false);
		//If user was about to exit game... prevent that
		data.elements.exit_menu.menu->SetIsActive(false);
		data.elements.exit_menu.menu->SetIsVisible(false);
		//Ok, we won!!!!!
		we_are_the_champions_message(data);
		//Show local cards
		reveal_local_cards(data);
		data.feedback_event = FB_NO_EVENT;
	}
	else
		//WE LOST! WE HAVE AN ERROR;
		data.feedback_event = FB_ERROR;
}
static void wait_remote_player_response____remote_we_lost(game_data& data, Package_hanabi* package)
{
	//Equivalent to ack! disable timer
	abort_timeout_count(data);
	//Remote player says we lost. But, did we lose?
	//Check lightning indicators
	if (data.lightnings == HANABI_TOTAL_LIGHTNING_INDICATORS)
	{
		//If open, colapse discarded cards menu
		data.elements.discarded_cards.menu->SetIsActive(false);
		data.elements.discarded_cards.menu->SetIsVisible(false);
		//If user was about to exit game... prevent that
		data.elements.exit_menu.menu->SetIsActive(false);
		data.elements.exit_menu.menu->SetIsVisible(false);
		//Ok, we did lose. 
		we_lost_message(data);
		//Show local cards
		reveal_local_cards(data);
		data.feedback_event = FB_NO_EVENT;
	}
	else
		//WE DID NOT LOSE! WE HAVE AN ERROR;
		data.feedback_event = FB_ERROR;
}
static void wait_remote_player_response____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
static void wait_sw_draw____sw_draw_next(game_data& data, Package_hanabi* package)
{
	//We are here because local user wants to draw a card
	
	card c;
	Package_draw p;

	//Pick card
	c = data.card_deck.pick_top();
	cout << "[GAME_HANDLER][LOG] : Draw card: [" << c.get_short_name() << "]" << endl;
	for(unsigned int i = 0 ; i < HANABI_HAND_SIZE ; i++)
		if (data.local_player_card[i] == card(NO_COLOR, NO_NUMBER))
		{
			//Put it in blank space
			data.local_player_card[i] = c;

#ifdef CHEAT_SHOW_LOCAL_CARDS
			data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(c));
#else
			data.elements.player.local.player_card[i]->SetBitmap(data.skin.get_bitmap(card(NO_COLOR,NO_NUMBER)));
#endif
			data.elements.player.local.player_card[i]->SetIsVisible(true);
			break;
		}
	//Send draw package

	p.set_card(c);
	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		remote_player_turn_starts(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void wait_sw_draw____sw_draw_last(game_data& data, Package_hanabi* package)
{
	//We are here because local user wants to draw a card
	//We already know this card is last deck card
	cout << "[GAME_HANDLER][LOG] : LAST CARD!" << endl;
	data.elements.deck->SetUseSecondBitmap(false);	//Show empty deck
	data.redraw = true;
	wait_sw_draw____sw_draw_next(data, package);	//Same as draw next
}
//% State group: IN GAME, branch: LOCAL_PLAYER %
static const STATE local_player_turn[] =
{
	{ LOCAL_GIVE_CLUE,local_player_turn____local_give_clue,remote_player_turn },
	{ LOCAL_PLAY,local_player_turn____local_play,wait_remote_player_response },
	{ LOCAL_DISCARD,local_player_turn____local_discard,wait_remote_player_response },
	{ GND,nullptr,nullptr },
};
static const STATE wait_remote_player_response[] =
{
	{ ACK,wait_remote_player_response____ack,wait_sw_draw },
	{ REMOTE_WE_WON,wait_remote_player_response____remote_we_won,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_WE_LOST,wait_remote_player_response____remote_we_lost,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ TIMEOUT,wait_remote_player_response____timeout,end_state },											//  break
	{ GND,nullptr,nullptr }
};
static const STATE wait_sw_draw[] =
{
	{ SW_DRAW_NEXT,wait_sw_draw____sw_draw_next,remote_player_turn },
	{ SW_DRAW_LAST,wait_sw_draw____sw_draw_last,sc_1_remote_player_turn },	//	--> GO TO GAME FINISHING SCENARIO 1
	{ GND,nullptr,nullptr }
};

//$ Action group: IN GAME, branch: REMOTE_PLAYER $
static void remote_player_turn____remote_give_clue(game_data& data, Package_hanabi* package)
{
	//We are here because remote player gave us a clue
	//Let's process data
	Package_you_have *rec_p;
	//We know remote user turn finished
	//Have we got clues?
	if (data.clues != 0)
	{
		if ((rec_p = dynamic_cast<Package_you_have*>(package)) != nullptr)
		{
			card c;
			string message;
			string clue;
			//Don't forget: USE CLUE
			data.elements.indicators.clue_indicators[--data.clues]->SetUseTopBitmap(true);
			//Remote player turn finished
			remote_player_turn_ends(data);
			//And now is local turn
			local_player_turn_starts(data);

			//Read clue
			rec_p->get_clue(&c);

			//Prepare message
			message = "Remote says: You have ";

			//Number or color
			if (c.get_color() != NO_COLOR)
			{
				//Clue is color
				clue = color_string[c.get_color()];
				message += clue;
				for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
					if (data.local_player_card[i].get_color() == c.get_color())
						data.elements.clue.clue_marker[i]->SetIsVisible(true);
					else
						data.elements.clue.clue_marker[i]->SetIsVisible(false);
			}
			else
			{
				//Clue is number
				clue = (char)('0' + c.get_number() + 1);
				message += (string("Number ") + clue);
				for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
					if (data.local_player_card[i].get_number() == c.get_number())
						data.elements.clue.clue_marker[i]->SetIsVisible(true);
					else
						data.elements.clue.clue_marker[i]->SetIsVisible(false);
			}

			cout << "[GAME_HANDLER][LOG] : CLUE: " << clue << endl;
			//Show message.
			data.elements.message2->SetText(message.c_str());
			data.elements.message2->SetIsVisible(true);


			//SHOW CLUE
			data.elements.clue.menu->SetIsVisible(true);
			data.elements.clue.menu->SetIsActive(true);


			data.redraw = true;
			data.feedback_event = FB_NO_EVENT;
		}
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void remote_player_turn____remote_play(game_data& data, Package_hanabi* package)
{
	//We are here because remote player played a card
	//Let's process data
	Package_play *rec_p;
	//We know remote user turn finished
	remote_player_turn_ends(data);
	if ((rec_p = dynamic_cast<Package_play*>(package)) != nullptr)
	{
		unsigned int card_id;
		card c;
		string card_name, message;
		Package_ack p;

		//Card played?
		rec_p->get_card_id(&card_id);
		c = data.remote_player_card[card_id];

		//Compute card string
		card_name = c.get_name();

		//Prepare string..
		message = string("Friend played ") + card_name;

		//Play card and finish string
		if (play_card(c, data))
			message += " - SUCCESS!";
		else
			message += " - Card discarded :(";
		//Show message
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);
		//Leave black sapce where played card was
		data.remote_player_card[card_id] = card(NO_COLOR,NO_NUMBER);
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;
		//Send ack package
		if (send_package(p, data.connection))
		{
			data.feedback_event = FB_NO_EVENT;
			//After play, will receive draw package, so enable timeout
			start_timeout_count(data);
		}
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void remote_player_turn____remote_discard(game_data& data, Package_hanabi* package)
{
	//We are here because remote player discarded a card
	//Let's process data
	Package_discard *rec_p;
	//We know remote user turn finished
	remote_player_turn_ends(data);
	if ((rec_p = dynamic_cast<Package_discard*>(package)) != nullptr)
	{
		unsigned int card_id;
		card c;
		string card_name, message;
		Package_ack p;

		//Card discarded?
		rec_p->get_card_id(&card_id);
		c = data.remote_player_card[card_id];
		discard_card(c, data);

		//Compute card string
		card_name = c.get_name();

		//Prepare message to show
		message = string("Friend discarded ") + card_name;

		//Show message.
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);

		//Leave a blank space where card was
		data.remote_player_card[card_id] = card(NO_COLOR,NO_NUMBER);
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;

		//Send ack package
		if (send_package(p, data.connection))
		{
			data.feedback_event = FB_NO_EVENT;
			//After discard, will receive draw package, so enable timeout
			start_timeout_count(data);
		}
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void remote_player_turn____remote_play_won(game_data& data, Package_hanabi* package)
{
	//We are here because remote player played a card. And yes, we won.
	//Let's process data
	Package_play *rec_p;
	//We know remote user turn finished
	remote_player_turn_ends(data);
	if ((rec_p = dynamic_cast<Package_play*>(package)) != nullptr)
	{
		unsigned int card_id;
		card c;
		string card_name, message;
		Package_we_won p;

		//Card played?
		rec_p->get_card_id(&card_id);
		c = data.remote_player_card[card_id];

		//Compute card string
		card_name = c.get_name();

		//Prepare string..
		message = string("Friend played ") + card_name;

		//Play card and finish string
		if (play_card(c, data))
			message += " - SUCCESS!";
		else
			message += " - Card discarded :(";
		//Show message
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);
		//Leave black sapce where played card was
		data.remote_player_card[card_id] = card(NO_COLOR, NO_NUMBER);
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;

		//If open, colapse discarded cards menu
		data.elements.discarded_cards.menu->SetIsActive(false);
		data.elements.discarded_cards.menu->SetIsVisible(false);
		//If user was about to exit game... prevent that
		data.elements.exit_menu.menu->SetIsActive(false);
		data.elements.exit_menu.menu->SetIsVisible(false);
		//WE WON :)
		we_are_the_champions_message(data);
		//Show local cards
		reveal_local_cards(data);
		//Inform remote player we won
		if (send_package(p, data.connection))
			data.feedback_event = FB_NO_EVENT;
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void remote_player_turn____remote_play_lost(game_data& data, Package_hanabi* package)
{
	//We are here because remote player played a card. And yes, we lost.
	//Let's process data
	Package_play *rec_p;
	//We know remote user turn finished
	remote_player_turn_ends(data);
	if ((rec_p = dynamic_cast<Package_play*>(package)) != nullptr)
	{
		unsigned int card_id;
		card c;
		string card_name, message;
		Package_we_lost p;

		//Card played?
		rec_p->get_card_id(&card_id);
		c = data.remote_player_card[card_id];

		//Compute card string
		card_name = c.get_name();

		//Prepare string..
		message = string("Friend played ") + card_name;

		//Play card and finish string
		if (play_card(c, data))
			message += " - SUCCESS!";
		else
			message += " - Card discarded :(";
		//Show message
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);
		//Leave black sapce where played card was
		data.remote_player_card[card_id] = card(NO_COLOR, NO_NUMBER);
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;

		//If open, colapse discarded cards menu
		data.elements.discarded_cards.menu->SetIsActive(false);
		data.elements.discarded_cards.menu->SetIsVisible(false);
		//If user was about to exit game... prevent that
		data.elements.exit_menu.menu->SetIsActive(false);
		data.elements.exit_menu.menu->SetIsVisible(false);
		//We lost :(
		we_lost_message(data);
		//Show local cards
		reveal_local_cards(data);
		//Inform remote player we lost
		if (send_package(p, data.connection))
			data.feedback_event = FB_NO_EVENT;
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void wait_draw____draw_next(game_data& data, Package_hanabi* package)
{
	//Got draw! disable timer
	abort_timeout_count(data);
	//We are here because remote draw a card, and he informed us his card.
	//Let's process data
	Package_draw *rec_p;
	if ((rec_p = dynamic_cast<Package_draw*>(package)) != nullptr)
	{
		card c;
		rec_p->get_card(&c);				//Get his card
		data.card_deck.remove_card(c);		//If he has this card, then remove it from card deck.
		cout << "[GAME_HANDLER][LOG] : Draw card: [" << c.get_short_name() << "]"  << endl;
		for (unsigned int i = 0; i < HANABI_HAND_SIZE; i++)
			if (data.remote_player_card[i] == card(NO_COLOR, NO_NUMBER))
			{
				//Give card to local representation of remote user
				data.remote_player_card[i] = c;
				data.elements.player.remote.player_card[i]->SetDefaultBitmap(data.skin.get_bitmap(c));
				data.elements.player.remote.player_card[i]->SetIsVisible(true);
				break;
			}
		local_player_turn_starts(data);
		data.redraw = true;
		data.feedback_event = FB_NO_EVENT;
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void wait_draw____draw_last(game_data& data, Package_hanabi* package)
{
	//Got draw! disable timer
	abort_timeout_count(data);
	cout << "[GAME_HANDLER][LOG] : LAST CARD!" << endl;
	//We are here because remote draw last card!
	data.elements.deck->SetUseSecondBitmap(false);	//Show empty deck
	data.redraw = true;
	//Same as draw next
	wait_draw____draw_next(data, package);
}
static void wait_draw____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: IN GAME, branch: REMOTE_PLAYER %
static const STATE remote_player_turn[] =
{
	{ REMOTE_GIVE_CLUE,remote_player_turn____remote_give_clue,local_player_turn },
	{ REMOTE_PLAY,remote_player_turn____remote_play,wait_draw },
	{ REMOTE_DISCARD,remote_player_turn____remote_discard,wait_draw },
	{ REMOTE_PLAY_WON,remote_player_turn____remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_PLAY_LOST,remote_player_turn____remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ GND,nullptr,nullptr }
};
static const STATE wait_draw[] =
{
	{ DRAW_NEXT,wait_draw____draw_next,local_player_turn },
	{ DRAW_LAST,wait_draw____draw_last,sc_2_local_player_turn },	//	--> GO TO GAME FINISHING SCENARIO 2
	{ TIMEOUT, wait_draw____timeout, end_state},					//break
	{ GND,nullptr,nullptr }
};

//$ Action group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FIRST), branch: REMOTE_PLAYER $
static void sc_1_remote_player_turn____remote_give_clue(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_give_clue(data, package);
}
static void sc_1_remote_player_turn____remote_play(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_play(data, package);
}
static void sc_1_remote_player_turn____remote_discard(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_discard(data, package);
}
static void sc_1_remote_player_turn____remote_play_won(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_play_won(data, package);
}
static void sc_1_remote_player_turn____remote_play_lost(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_play_lost(data, package);
}
static void sc_1_wait_draw____draw_fake(game_data& data, Package_hanabi* package)
{
	//Got draw! disable timeout
	abort_timeout_count(data);
	//We are here because remote draw a card, and he informed us his card. 
	//This is the 'NO CARD' card
	//Not much to do here...
	local_player_turn_starts(data);
	data.redraw = true;
	data.feedback_event = FB_NO_EVENT;
}
static void sc_1_wait_draw____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FIRST), branch: REMOTE_PLAYER %
static const STATE sc_1_remote_player_turn[] =
{
	{ REMOTE_GIVE_CLUE,sc_1_remote_player_turn____remote_give_clue,sc_1_local_player_turn },
	{ REMOTE_PLAY,sc_1_remote_player_turn____remote_play,sc_1_wait_draw },
	{ REMOTE_DISCARD,sc_1_remote_player_turn____remote_discard,sc_1_wait_draw },
	{ REMOTE_PLAY_WON,sc_1_remote_player_turn____remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_PLAY_LOST,sc_1_remote_player_turn____remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
};
static const STATE sc_1_wait_draw[] =
{
	{ DRAW_FAKE,sc_1_wait_draw____draw_fake,sc_1_local_player_turn },
	{ TIMEOUT,sc_1_wait_draw____timeout,end_state },		//break
};

//$ Action group: FINISHING SCENARIO 1, (LAST 2 TURNS, REMOTE FIRST) branch: LOCAL_PLAYER $
static void sc_1_local_player_turn____local_give_clue(game_data& data, Package_hanabi* package)
{
	//@@TO_DO: CHECK THIS AFTER AGUSTIN ANSWERS MAIL!
	//SHOULD THIS BE ALLOWED??
	//Same as normal
	local_player_turn____local_give_clue(data, package);
}
static void sc_1_local_player_turn____local_play(game_data& data, Package_hanabi* package)
{
	//Same as normal
	local_player_turn____local_play(data, package);
}
static void sc_1_local_player_turn____local_discard(game_data& data, Package_hanabi* package)
{
	//Same as normal
	local_player_turn____local_discard(data, package);
}
static void sc_1_wait_remote_player_response____remote_we_won(game_data& data, Package_hanabi* package)
{
	//Same as normal
	wait_remote_player_response____remote_we_won(data, package);
}
static void sc_1_wait_remote_player_response____remote_we_lost(game_data& data, Package_hanabi* package)
{
	//Same as normal
	wait_remote_player_response____remote_we_lost(data, package);
}
static void sc_1_wait_remote_player_response____remote_match_is_over(game_data& data, Package_hanabi* package)
{
	//@@TO_DO: CHECK THIS AFTER AGUSTIN ANSWERS MAIL!
	//Remote player says we match is over. But, didn't we win? lose?
	//Check lightning indicators
	//Got match is over! disable timeout
	abort_timeout_count(data);
	if (data.lightnings != HANABI_TOTAL_LIGHTNING_INDICATORS)
	{
		//Ok, we didn't lose
		//Did we win?
		unsigned int card_count = 0;
		//count cards in stacks
		for (unsigned int i = 0; i < HANABI_TOTAL_COLORS; i++)
			card_count += data.color_stack[i];
		if (card_count != HANABI_TOTAL_COLORS*HANABI_TOTAL_NUMBERS)
		{
			//If open, colapse discarded cards menu
			data.elements.discarded_cards.menu->SetIsActive(false);
			data.elements.discarded_cards.menu->SetIsVisible(false);
			//If user was about to exit game... prevent that
			data.elements.exit_menu.menu->SetIsActive(false);
			data.elements.exit_menu.menu->SetIsVisible(false);
			//Ok, match is over
			match_is_over_message(data);
			//Show local cards
			reveal_local_cards(data);
			data.feedback_event = FB_NO_EVENT;
		}
		else
			//WE WON! WE GOT AN ERROR HERE
			data.feedback_event = FB_ERROR;
	}
	else
		//WE LOST! WE GOT AN ERROR HERE
		data.feedback_event = FB_ERROR;
}
static void sc_1_wait_remote_palyer_response____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FISRT), branch: LOCAL_PLAYER %
static const STATE sc_1_local_player_turn[] =
{
	{ LOCAL_GIVE_CLUE,sc_1_local_player_turn____local_give_clue,sc_1_wait_remote_player_response },
	{ LOCAL_PLAY,sc_1_local_player_turn____local_play,sc_1_wait_remote_player_response },
	{ LOCAL_DISCARD,sc_1_local_player_turn____local_discard,sc_1_wait_remote_player_response },
};
static const STATE sc_1_wait_remote_player_response[] =
{
	{ REMOTE_WE_WON,sc_1_wait_remote_player_response____remote_we_won,b_wait_local_play_again_answer },				//  --> GO TO END OF GAME
	{ REMOTE_WE_LOST,sc_1_wait_remote_player_response____remote_we_lost,b_wait_local_play_again_answer },				//  --> GO TO END OF GAME
	{ REMOTE_MATCH_IS_OVER,sc_1_wait_remote_player_response____remote_match_is_over,b_wait_local_play_again_answer },	//  --> GO TO END OF GAME
	{ TIMEOUT,sc_1_wait_remote_palyer_response____timeout,end_state },	//break
};

//$ Action group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: LOCAL_PLAYER $
static void sc_2_local_player_turn____local_give_clue(game_data& data, Package_hanabi* package)
{
	//Same as normal
	local_player_turn____local_give_clue(data, package);
}
static void sc_2_local_player_turn____local_play(game_data& data, Package_hanabi* package)
{
	//Same as normal
	local_player_turn____local_play(data, package);
}
static void sc_2_local_player_turn____local_discard(game_data& data, Package_hanabi* package)
{
	//Same as normal
	local_player_turn____local_discard(data, package);
}
static void sc_2_wait_remote_player_response____ack(game_data& data, Package_hanabi* package)
{
	//Almost same as normal
	wait_remote_player_response____ack(data, package);
	//INTERCEPT FEEDBACK EVENT!
	if (data.feedback_event == FB_DRAW)
	{
		//Send draw card here, no need for an extra state!
		card c(NO_COLOR,NO_NUMBER);
		Package_draw p;
		p.set_card(c);
		if (send_package(p, data.connection))
		{
			data.feedback_event = FB_NO_EVENT;
			remote_player_turn_starts(data);
		}
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
		data.feedback_event = FB_NO_EVENT;
	}
}
static void sc_2_wait_remote_player_response____remote_we_won(game_data& data, Package_hanabi* package)
{
	//Same as normal
	wait_remote_player_response____remote_we_won(data, package);
}
static void sc_2_wait_remote_player_response____remote_we_lost(game_data& data, Package_hanabi* package)
{
	//Same as normal
	wait_remote_player_response____remote_we_lost(data, package);
}
static void sc_2_wait_remote_player_response____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: LOCAL_PLAYER %
static const STATE sc_2_local_player_turn[] =
{
	{ LOCAL_GIVE_CLUE,sc_2_local_player_turn____local_give_clue,sc_2_remote_player_turn },
	{ LOCAL_PLAY,sc_2_local_player_turn____local_play,sc_2_wait_remote_player_response },
	{ LOCAL_DISCARD,sc_2_local_player_turn____local_discard,sc_2_wait_remote_player_response },
};
static const STATE sc_2_wait_remote_player_response[] =
{
	{ ACK,sc_2_wait_remote_player_response____ack,sc_2_remote_player_turn },
	{ REMOTE_WE_WON,sc_2_wait_remote_player_response____remote_we_won,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_WE_LOST,sc_2_wait_remote_player_response____remote_we_lost,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{ TIMEOUT,sc_2_wait_remote_player_response____timeout,end_state },										//break
};

//$ Action group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: REMOTE_PLAYER $
static void sc_2_remote_player_turn____remote_give_clue(game_data& data, Package_hanabi* package)
{
	//IF WE ARE HERE IS BECAUSE MATCH IS OVER!!!
	//TO_DO: WHAT SHOULD I DO HERE???
	//@@TO_DO: CHECK THIS AFTER AGUSTIN ANSWERS MAIL!
}
static void sc_2_remote_player_turn____remote_discard(game_data& data, Package_hanabi* package)
{
	//@@TO_DO: CHECK THIS AFTER AGUSTIN ANSWERS MAIL!
	//IF WE ARE HERE IS BECAUSE MATCH IS OVER!!!
	//We are here because remote player discarded a card
	//Let's process data
	Package_discard *rec_p;
	//We know remote user turn finished
	remote_player_turn_ends(data);
	if ((rec_p = dynamic_cast<Package_discard*>(package)) != nullptr)
	{
		unsigned int card_id;
		card c;
		string card_name, message;
		Package_match_is_over p;

		//Card discarded?
		rec_p->get_card_id(&card_id);
		c = data.remote_player_card[card_id];
		discard_card(c, data);

		//Compute card string
		card_name = c.get_name();

		//Prepare message to show
		message = string("Friend discarded ") + card_name;

		//Show message.
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);

		//Leave a blank space where card was
		data.remote_player_card[card_id] = card(NO_COLOR, NO_NUMBER);
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;

		//If open, colapse discarded cards menu
		data.elements.discarded_cards.menu->SetIsActive(false);
		data.elements.discarded_cards.menu->SetIsVisible(false);
		//If user was about to exit game... prevent that
		data.elements.exit_menu.menu->SetIsActive(false);
		data.elements.exit_menu.menu->SetIsVisible(false);
		//Match is over
		match_is_over_message(data);
		//Show local cards
		reveal_local_cards(data);

		//Send match is over package
		if (send_package(p, data.connection))
			data.feedback_event = FB_NO_EVENT;
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
}
static void sc_2_remote_player_turn____remote_play(game_data& data, Package_hanabi* package)
{
	//@@TO_DO: CHECK THIS AFTER AGUSTIN ANSWERS MAIL!
	//IF WE ARE HERE IS BECAUSE MATCH IS OVER!!!
	//We are here because remote player played a card.
	//Let's process data
	Package_play *rec_p;
	//We know remote user turn finished
	remote_player_turn_ends(data);
	if ((rec_p = dynamic_cast<Package_play*>(package)) != nullptr)
	{
		unsigned int card_id;
		card c;
		string card_name, message;
		Package_match_is_over p;

		//Card played?
		rec_p->get_card_id(&card_id);
		c = data.remote_player_card[card_id];

		//Compute card string
		card_name = c.get_name();

		//Prepare string..
		message = string("Friend played ") + card_name;

		//Play card and finish string
		if (play_card(c, data))
			message += " - SUCCESS!";
		else
			message += " - Card discarded :(";
		//Show message
		data.elements.message2->SetText(message.c_str());
		data.elements.message2->SetIsVisible(true);
		//Leave black sapce where played card was
		data.remote_player_card[card_id] = card(NO_COLOR, NO_NUMBER);
		data.elements.player.remote.player_card[card_id]->SetIsVisible(false);
		data.redraw = true;

		//If open, colapse discarded cards menu
		data.elements.discarded_cards.menu->SetIsActive(false);
		data.elements.discarded_cards.menu->SetIsVisible(false);
		//If user was about to exit game... prevent that
		data.elements.exit_menu.menu->SetIsActive(false);
		data.elements.exit_menu.menu->SetIsVisible(false);
		//Match is over
		match_is_over_message(data);
		//Show local cards
		reveal_local_cards(data);
		//Inform remote player match is over
		if (send_package(p, data.connection))
			data.feedback_event = FB_NO_EVENT;
		else
			data.feedback_event = FB_ERROR;	//FATAL ERROR
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR

}
static void sc_2_remote_player_turn____remote_play_won(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_play_won(data, package);
}
static void sc_2_remote_player_turn____remote_play_lost(game_data& data, Package_hanabi* package)
{
	//Same as normal
	remote_player_turn____remote_play_lost(data, package);
}
//% State group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: REMOTE_PLAYER %
static const STATE sc_2_remote_player_turn[] =
{
	{ REMOTE_GIVE_CLUE,sc_2_remote_player_turn____remote_give_clue,a_wait_remote_play_again_answer },		//	--> GO TO END OF GAME
	{ REMOTE_DISCARD,sc_2_remote_player_turn____remote_discard,a_wait_remote_play_again_answer },			//	--> GO TO END OF GAME	
	{ REMOTE_PLAY,sc_2_remote_player_turn____remote_play,a_wait_remote_play_again_answer },				//	--> GO TO END OF GAME
	{ REMOTE_PLAY_WON,sc_2_remote_player_turn____remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{ REMOTE_PLAY_LOST,sc_2_remote_player_turn____remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
};
//$ Action group: LOCAL PLAYER QUIT $
static void local_player_quit____ack(game_data& data, Package_hanabi* package)
{
	//Got ack! dissable timer
	abort_timeout_count(data);
	//Exit!
	data.exit = true;
	data.connection->disconnect();
	data.feedback_event = FB_NO_EVENT;
	cout << "[GAME_HANDLER][INFO] : Exiting game properly..." << endl;
}
static void local_player_quit____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
//% State group: LOCAL PLAYER QUIT %
static const STATE local_player_quit[] =
{
	{ ACK,local_player_quit____ack,end_state },			//break
	{ TIMEOUT,local_player_quit____timeout,end_state },	//break
	{ GND, nullptr, nullptr}
};

//$ Action group: END $
static void end_state____end_game(game_data& data, Package_hanabi* package)
{
	//Dissable timer, just in case
	abort_timeout_count(data);

	data.connection->disconnect();	//Just in case
	data.exit = true;
	data.feedback_event = FB_NO_EVENT;
}
//% State group: END %
static const STATE end_state[] =
{
	{EXIT_GAME,end_state____end_game,nullptr},	//EXIT!
	{GND,nullptr,nullptr}
};

//# Actions common to all states! #
static void common____timeout(game_data& data, Package_hanabi* package)
{
	timeout(data, package);
}
static void common____error_ev(game_data& data, Package_hanabi* package)
{
	cerr << "[GAME_HANDLER][ERROR] : COMUNICATION ERROR" <<  endl;
	//Dissable timer, just in case
	abort_timeout_count(data);
	//Error event.
	//Remote user was already informed
	//Or cannot inform remote user about it
	//Inform local user
	data.elements.error_menu.menu->SetIsActive(true);
	data.elements.error_menu.menu->SetIsVisible(true);
	data.quit_button_enabled = false;
	//If user was about to exit game... prevent that
	data.elements.exit_menu.menu->SetIsActive(false);
	data.elements.exit_menu.menu->SetIsVisible(false);
	data.redraw = true;
	//Disconnect
	data.connection->disconnect();
	data.feedback_event = FB_NO_EVENT;
};
static void common____bad(game_data& data, Package_hanabi* package)
{
	//Something bad happened :(
	Package_error p;
	//Dissable timer, just in case
	abort_timeout_count(data);
	//If user was about to exit game... prevent that
	data.elements.exit_menu.menu->SetIsActive(false);
	data.elements.exit_menu.menu->SetIsVisible(false);

	//Inform local user
	data.elements.error_menu.menu->SetIsActive(true);
	data.elements.error_menu.menu->SetIsVisible(true);
	data.quit_button_enabled = false;

	data.redraw = true;

	//Send error;
	send_package(p, data.connection);
	data.feedback_event = FB_NO_EVENT; //Don't care about errors here. We are over.
	//After an error, disconect.
	data.connection->disconnect();
};
static void common____quit(game_data& data, Package_hanabi* package)
{
	//We are here because remote player left the game.
	Package_ack p;

	cout << "[GAME_HANDLER][INFO] : Remote player left..."  << endl;

	//Dissable timer
	abort_timeout_count(data);
	//If user was about to exit game... prevent that
	data.elements.exit_menu.menu->SetIsActive(false);
	data.elements.exit_menu.menu->SetIsVisible(false);

	//Inform local user about this
	data.elements.remote_player_left_menu.menu->SetIsVisible(true);
	data.elements.remote_player_left_menu.menu->SetIsActive(true);
	data.quit_button_enabled = false;
	data.redraw = true;

	//Just send ack;
	if (send_package(p, data.connection))
		data.feedback_event = FB_NO_EVENT;
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	data.connection->disconnect();	//Now, disconnect!
	cout << "[GAME_HANDLER][INFO] : Exiting game properly..." << endl;
};
static void common____local_quit(game_data& data, Package_hanabi* package)
{
	//We are here because local player left the game.
	//Send quit package
	Package_quit p;
	//Abort timeout
	abort_timeout_count(data);

	cout << "[GAME_HANDLER][INFO] : Local player left..." << endl;
	//Show quiting message
	data.elements.message->SetText("Quiting...");
	data.elements.message->SetIsVisible(true);
	data.elements.message2->SetIsVisible(false);
	data.quit_button_enabled = false;
	data.redraw = true;

	if (send_package(p, data.connection))
	{
		data.feedback_event = FB_NO_EVENT;
		//Will wait ack.. enable timer
		start_timeout_count(data);
	}
	else
		data.feedback_event = FB_ERROR;	//FATAL ERROR
	//Now must wait ack...
};

//This state is actually part of all states.

//nullptr means stay in same state
static const STATE common[] =
{
	{ TIMEOUT, common____timeout, end_state},
	{ ERROR_EV,common____error_ev,end_state },				//break
	{ BAD,common____bad,end_state },							//break
	{ QUIT,common____quit,end_state },						//break
	{ LOCAL_QUIT,common____local_quit,local_player_quit },	//wait ack and break
	{ GND,common____bad,end_state }							//Any other event will execute common____bad and change state to end_state
};
