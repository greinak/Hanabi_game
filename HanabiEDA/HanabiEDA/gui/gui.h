#ifndef GUI_H_
#define GUI_H_
#include <iostream>
#include <allegro5\allegro5.h>
#include <map>
#include "gui_element.h"
#include "gui_menu.h"
#include "gui_text.h"
#include "gui_image.h"
#include "gui_button.h"

//Author: Gonzalo Julian Reina Kiperman

using namespace std;

typedef map<string, ALLEGRO_BITMAP*> bitmap_dic_t;
typedef pair<string, unsigned int> font_data_t;
typedef map<font_data_t, ALLEGRO_FONT*> font_dic_t;
typedef map<string, GUI_element*> id_dic_t;
typedef map<string, string> attr_t;

typedef struct my_XML_element my_XML_element;

class Gui
{
public:
	//Construct GUI with istream to XML gui data
	Gui(istream &xml);
	//Always check this
	bool initialized_successfully();
	//Redraws gui
	void redraw();
	//Feed mouse event
	bool feed_mouse_event(ALLEGRO_MOUSE_STATE & state);
	GUI_element* get_element_from_id(const char* element_id);
	ALLEGRO_DISPLAY* get_display();
	~Gui();
private:
	bool initialized;
	//Functions used in parsing:
	//Returns true if success. Adds elements to GUI list
	bool handle_gui_menu_data(const my_XML_element& element);
	//Returns true if success. Creates submenu from element and returns it on created
	bool handle_gui_submenu_data(const my_XML_element& element, GUI_element** created);
	//Returns true if success. Creates image from element and returns it on created
	bool handle_gui_image_data(const my_XML_element& element, GUI_element** created);
	//Returns true if success. Creates text from element and returns it on created
	bool handle_gui_text_data(const my_XML_element& element, GUI_element** created);
	//Returns true if success. Creates button from element and returns it on created
	bool handle_gui_button_data(const my_XML_element& element, GUI_element** created);
	//Returns true if success. Creates allegro bitmap from string, aviods duplicates.
	bool get_bitmap_from_string(const string& filename, ALLEGRO_BITMAP ** bitmap);
	//Returns true if success. Creates allegro font from strings, avoids duplicates.
	bool get_font_from_string(const string & filename, unsigned int size, ALLEGRO_FONT ** font);
	//Returns true if success. Creates elements from given element and adds them on *list_p
	bool handle_elements(const my_XML_element& element, list<GUI_element*>* list_p);
	//Get bool true or false from string. true on success
	bool get_bool_from_string(const string &file, bool* result);	//Returns true if success in parsing string
	//Get color from string. True on success. Format 0xRRGGBBAA
	bool get_color_from_string(const string & color_string, ALLEGRO_COLOR * color);
	//From a nullptr terminated array of pointers to char (keys to look for)
	//looks map for keys
	//and returns pointers to string values, in given array of string pointers (same order as keys)
	//string pointer will be null if key was not given in attr
	//Returns true on success. Returns false if map contains elements that are not in key array
	//NOTE: (sizeof(values)/sizeof(values[0])-1) >= (sizeof(key)/sizeof(key[0]))  !!!!!
	bool get_attributes_from_strings(const attr_t& attr,const char** key, const string **values);	//Last one is pointer to pointer to const string ;)
	//Element list
	list<GUI_element*> menu_element_list;
	//Variables used to hold data
	list<GUI_menu> submenus;
	list<GUI_image> images;
	list<GUI_text> texts;
	list<GUI_button> buttons;
	bitmap_dic_t bitmap_dictionary; //In order to load images just one time
	font_dic_t font_dictionary;	//In order to load fonts just one time
	id_dic_t id_dictionary;	//In order to access elements declared in XML from C++
	//GUI_menu data
	unsigned int gui_width, gui_height;
	float gui_sx, gui_sy;
	ALLEGRO_BITMAP* gui_background, *gui_icon;
	string gui_title;
	ALLEGRO_COLOR gui_backg_color;
	ALLEGRO_DISPLAY* display;
};

#endif