#include "handle_menu.h"
#include <sys/utime.h>

#define MAX_TEXT_SIZE 21
#define WAIT_AS_CLIENT_TIME_MIN	200
#define WAIT_AS_CLIENT_TIME_MAX	10000
#define CONNECTION_TIMEOUT		15000
#define CONNECTION_PORT			13796 
#define CONNECTED_HOLD_TIME		3

typedef struct
{
	//Menu elements
	GuiButton *name_button, *remote_host_button, *connect_button;
	GuiText *name_text, *remote_host_text, *message;
	Gui* gui;
	//Event queue
	ALLEGRO_EVENT_QUEUE* ev_q;
	//Flags
	bool exit;				//true if should exit
	bool connected;			//true if connected
	bool on_name;			//true if writing name
	bool on_remote_host;	//true if writing remote host
	bool release_mouse;		//true if should release mouse
	bool is_server;
	Net_connection* net;	//connection
}menu_data;

static bool name_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw);
static bool remote_host_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw);
static bool connect_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw);

bool handle_menu(Gui* menu, string* name, Net_connection** net, bool* is_server)
{
	bool ret_val = false;
	menu_data data;
	if (menu == nullptr || name == nullptr || net == nullptr || is_server == nullptr)
		return false;
	if ((data.ev_q = al_create_event_queue()) != nullptr)
	{
		//Register event sources in event queue
		al_register_event_source(data.ev_q, al_get_mouse_event_source());
		al_register_event_source(data.ev_q, al_get_keyboard_event_source());
		al_register_event_source(data.ev_q, al_get_display_event_source(menu->get_display()));
		//Fetch gui elements
		data.name_button = dynamic_cast<GuiButton *>(menu->get_element_from_id("name_button"));
		data.remote_host_button = dynamic_cast<GuiButton *>(menu->get_element_from_id("ip_button"));
		data.connect_button = dynamic_cast<GuiButton *>(menu->get_element_from_id("connect_button"));
		data.name_text = dynamic_cast<GuiText *>(menu->get_element_from_id("name_textbox"));
		data.remote_host_text = dynamic_cast<GuiText *>(menu->get_element_from_id("ip_textbox"));
		data.message = dynamic_cast<GuiText *>(menu->get_element_from_id("message_text"));
		//Set data variables
		data.gui = menu;
		data.exit = false;
		data.on_name = false;
		data.on_remote_host = false;
		data.net = nullptr;
		data.connected = false;
		data.is_server = false;
		//Set callbacks
		data.connect_button->SetOnClickUpCallback(connect_button_callback);
		data.name_button->SetOnClickDownCallback(name_button_callback);
		data.remote_host_button->SetOnClickDownCallback(remote_host_button_callback);
		data.release_mouse = false;
		if (data.name_button != nullptr && data.remote_host_button != nullptr && data.connect_button != nullptr && data.name_text != nullptr && data.remote_host_text != nullptr && data.message != nullptr)
		{
			//Set callback data
			data.name_button->SetUserData(&data);
			data.connect_button->SetUserData(&data);
			data.remote_host_button->SetUserData(&data);
			bool redraw = false;
			while (!data.exit)	//RUN MENU
			{
				ALLEGRO_EVENT ev;
				al_wait_for_event(data.ev_q, &ev);

				if (ev.any.source == al_get_display_event_source(menu->get_display()))
				{
					if (ev.display.type == ALLEGRO_EVENT_DISPLAY_CLOSE)	//Display close? close it.
						data.exit = true;
				}
				else if (ev.any.source == al_get_mouse_event_source())		//Mouse event? feed it to gui
				{
					ALLEGRO_MOUSE_STATE st;
					al_get_mouse_state(&st);
					if (menu->feed_mouse_event(st))
						redraw = true;	//Redraw if gui says so
				}
				else if (ev.any.source == al_get_keyboard_event_source())	//Keyboard?
				{
					if (ev.keyboard.type == ALLEGRO_EVENT_KEY_CHAR)	//Is it a key?
					{
						char c = ev.keyboard.unichar;		//Get it.
						if (c != '\r')
						{
							GuiText* target = data.on_name ? data.name_text : data.on_remote_host ? data.remote_host_text : nullptr;	//Prepare target
							if (target != nullptr)	//If any textbox is selected, write on it.
							{
								string text = target->getText();
								if (c == '\b' && text.size() != 0)
									text = text.substr(0, text.size() - 1);
								else if (isprint(c) && text.length() < MAX_TEXT_SIZE)
									text += c;
								target->SetText(text);
								redraw = true;	//Redraw!
							}
						}
						else
							connect_button_callback(data.connect_button, false, true, &data, &redraw);	//Enter equals button click!!
					}
				}
				if (data.release_mouse)	//Callbacks says gui should release mouse?
				{
					menu->force_release_mouse();
					redraw = true;
					data.release_mouse = false;
				}
				if (redraw && al_is_event_queue_empty(data.ev_q))	//Redraw.
				{
					menu->redraw();
					redraw = false;
				}
				if (data.connected)
				{
					(*is_server) = data.is_server;
					(*net) = data.net;
					(*name) = data.name_text->getText();
					al_rest(CONNECTED_HOLD_TIME);
					data.exit = true;
					ret_val = true;
				}
			}
		}
		else
			cerr << "Could not connect to Menu UI elements" << endl;
		al_destroy_event_queue(data.ev_q);
	}
	else
		cerr << "Error creating event queue for handling menu." << endl;
	return ret_val;
}


static bool name_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw)
{
	//Name textbox clicked!!
	((menu_data*)user_data)->on_remote_host = false;
	((menu_data*)user_data)->on_name = true;
	return false;
}
static bool remote_host_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw)
{
	//Remote host textbox clicked!!
	((menu_data*)user_data)->on_remote_host = true;
	((menu_data*)user_data)->on_name = false;
	return false;
}
static bool connect_button_callback(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw)
{
	//Connect button clicked (or enter was pressed)
	menu_data* data = (menu_data*)user_data;
	if (mouse_over_element && !forced)
	{
		Client *cl;
		Server *sv;
		data->message->SetText("Connecting...");
		data->message->SetIsVisible(true);
		al_pause_event_queue(data->ev_q,true);
		data->gui->redraw();
		//Attempt connection.
		unsigned int wait_as_client = ((float)rand() / (float)RAND_MAX)*(WAIT_AS_CLIENT_TIME_MAX - WAIT_AS_CLIENT_TIME_MIN) + WAIT_AS_CLIENT_TIME_MIN;
		unsigned int wait_as_server = CONNECTION_TIMEOUT - wait_as_client;
		//As client...
		if ((cl = new Client) != nullptr)
		{
				if (cl->connect_to_server(data->remote_host_text->getText(), CONNECTION_PORT, wait_as_client))
				{
					data->connected = true;
					data->net = cl;
					data->is_server = false;
				}
				if (!data->connected)
					delete cl;
		}
		else
		{
			data->exit = true;
			cerr << "Error creating client connection" << endl;
		}
		//As server
		if (data->exit == false && data->connected == false)
		{
			if ((sv = new Server(CONNECTION_PORT)) != nullptr)
			{
				if (sv->listen_for_connection(wait_as_server))
				{
					data->connected = true;
					data->net = sv;
					data->is_server = true;
				}
				else
					delete sv;
			}
			else
			{
				data->exit = true;
				cerr << "Error creating server connection" << endl;
			}
		}
		if (!data->exit)
		{
			if (data->connected)
			{
				cout << "Connected! Opening game..." << endl;
				data->message->SetText("Connected! Opening game...");
			}
			else
			{
				cout << "Could not connect." << endl;
				data->message->SetText("Could not connect.");
			}
		}
		al_flush_event_queue(data->ev_q);
		al_pause_event_queue(data->ev_q, false);
		data->release_mouse = true;
	}
	return false;
}

