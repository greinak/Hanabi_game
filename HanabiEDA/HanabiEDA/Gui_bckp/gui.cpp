#define _CRT_SECURE_NO_WARNINGS
#include "gui.h"
#include <expat.h>
#include <stack>
#include <map>
#include <list>
#include <istream>
#include <string>
#include <algorithm>

//Author: Gonzalo Julian Reina Kiperman

using namespace std;

//Fisrt, stuff needed for XML parsing
typedef struct my_XML_element
{
	string name;				//Label
	attr_t attributes;	//Attributes
	string text;				//Text (within labels)
	list<my_XML_element> children;	//Labels inside label
}my_XML_element;

//This class is defined here since this class is defined just por parsing
//And extra files for this are not needed
class XML_parser_object
{
public:
	XML_parser_object(XML_Parser* parser);	//Initialization
	my_XML_element get_parsed_data();		//Return parsed data, check if finished() before using data
	bool finished();
private:
	bool has_finished;
	friend static void XMLCALL startElement(void *userData, const char *name, const char **atts);	//Callbacks
	friend static void XMLCALL charData(void *userData, const XML_Char *s, int len);
	friend static void XMLCALL endElement(void *userData, const char *name);
	XML_Parser* parser;
	stack<my_XML_element*> stack_XML;
	my_XML_element parsed_data;
};

static void XMLCALL
startElement(void *userData, const char *name, const char **atts)	//Start label
{
	XML_parser_object &data = *(XML_parser_object*)userData;
	if (!data.finished())	//If parsing has not finished
	{
		my_XML_element new_element;
		new_element.name = string(name);	//New element

		unsigned int i;
		for (i = 0; atts[i] != nullptr; i += 2)	//Get attributes
			new_element.attributes[atts[i]] = atts[i + 1];
		if (new_element.attributes.size() != i / 2)	//No repeated attributes!
			XML_StopParser(*data.parser, false);

		if (data.stack_XML.size() != 0)	//Is first element?
		{

			data.stack_XML.top()->children.push_front(new_element);		//Add as children
			data.stack_XML.push(&data.stack_XML.top()->children.front());//Push it on the stack
		}
		else
		{
			data.parsed_data = new_element;
			data.stack_XML.push(&data.parsed_data);	//Push it on the stack
		}
	}
	else
		XML_StopParser(*data.parser, false);
}
static void XMLCALL
charData(void *userData, const XML_Char *s, int len)	//Text within labels
{
	XML_parser_object &data = *(XML_parser_object*)userData;
	if (s != nullptr && !data.finished())
		data.stack_XML.top()->text += string((char*)s, (size_t)len);	//Add text to element
}

static void XMLCALL
endElement(void *userData, const char *name)	//End label
{
	XML_parser_object &data = *((XML_parser_object*)userData);
	if (!string(name).compare(data.stack_XML.top()->name) && !data.finished())	//If information is ok
	{
		string &text = data.stack_XML.top()->text;
		bool whiteSpacesOnly = std::all_of(text.begin(), text.end(), isspace);	//Is text only whitespaces?
		if (whiteSpacesOnly)
			text = "";	//Clear it
		data.stack_XML.pop();	//Remove element from TOS
		if (data.stack_XML.size() == 0)	//Is stack empty?
			data.has_finished = true;		//If it is, parsing should have finished
	}
	else
		XML_StopParser(*data.parser, false);
}

XML_parser_object::XML_parser_object(XML_Parser * parser)
{
	this->has_finished = false;	//Parser objec not finished
	this->parser = parser;		//Set parser pointer
}

my_XML_element XML_parser_object::get_parsed_data()
{
	return parsed_data;
}

bool XML_parser_object::finished()
{
	return has_finished;
}

//Now, GUI
Gui::Gui(istream &xml)
{
	XML_Parser parser = XML_ParserCreate(nullptr);
	XML_parser_object data(&parser);
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);
	this->display = nullptr;
	this->gui_background = nullptr;
	this->gui_icon = nullptr;
	this->gui_sx = this->gui_sy = 1;
	this->gui_backg_color = al_map_rgb(0, 0, 0);
	this->gui_height = this->gui_width = 0;
	char buf[BUFSIZ];
	bool done;
	xml.read(buf, 1);	//This is a workaround for readsome not reading any data
	if (!xml.fail())
	{
		xml.putback(buf[0]);	//Workaround
		if (!xml.fail())
		{
			do
			{
				streamsize len = xml.readsome(buf, BUFSIZ);
				if (xml.fail())
					break;
				done = xml.peek() == EOF;
				if (XML_Parse(parser, buf, (size_t)len, done) == XML_STATUS_ERROR)
					break;
			} while (!done);
		}
	}
	XML_ParserFree(parser);	//Parsing finished
	my_XML_element parsed_data = data.get_parsed_data();	//Get data
	if (data.finished() && !parsed_data.name.compare("GUI_menu"))	//is main elemeng GUI_menu? //has parsed finished? or was is stopped?
	{
		if (handle_gui_menu_data(parsed_data))	//This will convert data to GUI data
		{
			parsed_data.children.clear();	//No need to keep this info, we can free memory by clearing it...
			//Now, create display in order to show menu
			if ((display = al_create_display(gui_width*gui_sx, gui_height*gui_sy)) != nullptr)
			{
				if (gui_title.size() != 0)
					al_set_window_title(display, gui_title.c_str());
				if (gui_icon != nullptr)
					al_set_display_icon(display, gui_icon);
				redraw();
				this->initialized = true;		//Success!!
				return;
			}
			else
				cerr << "Error creating allegro display for GUI\n" << endl;
		}
		else
			cerr << "Error creating GUI from XML file. Please check your XML file.\n" << endl;
		menu_element_list.clear();
		submenus.clear();
		images.clear();
		texts.clear();
		buttons.clear();
		id_dictionary.clear();
		al_destroy_bitmap(gui_background);
		al_destroy_bitmap(gui_icon);
		for (bitmap_dic_t::iterator it = bitmap_dictionary.begin(); it != bitmap_dictionary.end(); ++it)
			al_destroy_bitmap(it->second);
		bitmap_dictionary.clear();
		for (font_dic_t::iterator it = font_dictionary.begin(); it != font_dictionary.end(); ++it)
			al_destroy_font(it->second);
		font_dictionary.clear();
	}
	else
		cerr << "Error parsing XML data\n" << endl;
}

bool Gui::initialized_successfully()
{
	return initialized;
}

Gui::~Gui()
{
	//Note that destructor of data structs will be called automatically
	if (initialized_successfully())
	{
		al_destroy_display(display);
		for (bitmap_dic_t::iterator it = bitmap_dictionary.begin(); it != bitmap_dictionary.end(); ++it)
			al_destroy_bitmap(it->second);
		bitmap_dictionary.clear();
		for (font_dic_t::iterator it = font_dictionary.begin(); it != font_dictionary.end(); ++it)
			al_destroy_font(it->second);
		font_dictionary.clear();
		if (gui_background != nullptr)
			al_destroy_bitmap(gui_background);
		if (gui_icon != nullptr)
			al_destroy_bitmap(gui_icon);
	}
}

bool Gui::handle_gui_menu_data(const my_XML_element & element)
{
	//Menu containing all elements
	bool ret_val = true;
	const char* name = nullptr;
	for (list<my_XML_element>::const_iterator it = element.children.begin(); it != element.children.end() && ret_val; ++it)
	{
		//Look for labels
		const attr_t &attr = (*it).attributes;
		name = it->name.c_str();
		if (!strcmp(name, "elements") && it->attributes.size() == 0)
			ret_val = handle_elements(*it, &menu_element_list);
		else if (it->children.size() == 0)
		{
			if (it->attributes.size() != 0)
			{
				if(!strcmp(name,"size"))
				{
					const char *(key[]) = { "width","height",nullptr };
					string* (value[sizeof(key) / sizeof(key[0])-1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**) value))
					{
						unsigned int width, height;
						if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%u", &width) == 1 && sscanf(value[1]->c_str(), "%u", &height) == 1))
						{
							gui_width = width; gui_height = height;
							ret_val = true;
						}
					}
				}
				else if (!strcmp(name, "scale"))
				{
					const char *(key[]) = { "sx","sy",nullptr };
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**) value))
					{
						float sx, sy;
						if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%f", &sx) == 1 && sscanf(value[1]->c_str(), "%f", &sy) == 1))
						{
							gui_sx = sx; gui_sy = sy;
							ret_val = true;
						}
					}
				}
				else 
					ret_val = false;
			}
			else
			{
				if(!strcmp(name,"backg_color"))
				{
					ALLEGRO_COLOR color;
					if (get_color_from_string(it->text, &color))
						gui_backg_color = color;
					else
						ret_val = false;
				}
				else if(!strcmp(name,"backg_bitmap"))
				{
					if ((gui_background = al_load_bitmap(it->text.c_str())) == nullptr)
						ret_val = false;
				}
				else if(!strcmp(name,"icon_bitmap"))
				{
					if ((gui_icon = al_load_bitmap(it->text.c_str())) == nullptr)
						ret_val = false;
				}
				else if(!strcmp(name,"title"))
				{
					gui_title = it->text;
					if (gui_title.size() == 0)
						ret_val = false;
				}
				else
					ret_val = false;
			}
		}
		else
			ret_val = false;
	}
	if (!ret_val)
		cerr << "Error in GUI_menu XML data, at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_submenu_data(const my_XML_element & element, GUI_element** created)
{
	//Submenu
	submenus.push_front(GUI_menu());
	GUI_menu &submenu = submenus.front();
	(*created) = &submenu;
	bool ret_val = true;
	list<GUI_element*> el_list;
	const char* name = nullptr;
	for (list<my_XML_element>::const_iterator it = element.children.begin(); it != element.children.end() && ret_val; ++it)
	{
		name = it->name.c_str();
		if (!strcmp(name, "elements") && it->attributes.size() == 0)
		{
			ret_val = handle_elements(*it, &el_list);
		}
		else if (it->children.size() == 0)
		{
			if (it->attributes.size() != 0)
			{
				if(!strcmp(name,"position"))
				{
					const char *(key[]) = { "x","y","r",nullptr };
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**) value))
					{
						int x, y;
						float r = 0;
						bool radian;
						if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &x) == 1 && sscanf(value[1]->c_str(), "%d", &y) == 1))
						{
							if ((value[2] == nullptr) || (radian = (sscanf(value[2]->c_str(), "%f", &r) == 1)) || sscanf(value[2]->c_str(), "d%f", &r) == 1)
							{
								if (!radian)
									r *= ALLEGRO_PI / 180;
								submenu.set_position(x, y, r);
								ret_val = true;
							}
						}
					}
				}
				else if (!strcmp(name,"ref_point"))
				{
					const char *(key[]) = { "ref_x","ref_y",nullptr };
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**) value))
					{
						int ref_x, ref_y;
						if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &ref_x) == 1 && sscanf(value[1]->c_str(), "%d", &ref_y) == 1))
						{
							submenu.set_reference_point(ref_x, ref_y);
							ret_val = true;
						}
					}
				}
				else if(!strcmp(name,"size"))
				{
					const char *(key[]) = { "width","height","b_radius",nullptr };
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**)value))
					{
						unsigned int width, height;
						float b_radius;
						float r = 0;
						if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%u", &width) == 1 && sscanf(value[1]->c_str(), "%u", &height) == 1))
						{
							if (value[2] == nullptr || sscanf(value[2]->c_str(), "%f", &b_radius) == 1)
							{
								submenu.set_size(width, height);
								submenu.set_menu_rectangle_rounded_radius(b_radius);
								ret_val = true;
							}
						}
					}
				}
				else
					ret_val = false;
			}
			else
			{
				if(!strcmp(name,"visible"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.set_is_visible(flag);
					else
						ret_val = false;
				}
				else if (!strcmp(name,"active"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.set_is_active(flag);
					else
						ret_val = false;
				}
				else if(!strcmp(name,"backg_color"))
				{
					ALLEGRO_COLOR color;
					if (get_color_from_string(it->text, &color))
						submenu.set_background_color(color);
					else
						ret_val = false;
				}
				else if (!strcmp(name,"volatile"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.set_is_volatile(flag);
					else
						ret_val = false;
				}
				else if (!strcmp(name, "blocker"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.set_is_blocker(flag);
					else
						ret_val = false;
				}
				else
					ret_val = false;
			}
		}
		else
			ret_val = false;
	}
	if(ret_val)
		submenu.set_menu_GUI_element_pointer_list(el_list);
	else
		cerr << "Error in GUI_submenu XML data, at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_image_data(const my_XML_element & element, GUI_element** created)
{
	//Image
	images.push_front(GUI_image());
	GUI_image &image = images.front();
	(*created) = &image;
	bool ret_val = true;
	const char* name = nullptr;
	for (list<my_XML_element>::const_iterator it = element.children.begin(); it != element.children.end() && ret_val; ++it)
	{
		name = it->name.c_str();
		if (it->children.size() != 0)
		{
			ret_val = false;
			break;
		}
		if (it->attributes.size() != 0)
		{
			if(!strcmp(name,"position"))
			{
				const char *(key[]) = { "x","y","r",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					int x, y;
					float r = 0;
					bool radian;
					if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &x) == 1 && sscanf(value[1]->c_str(), "%d", &y) == 1))
					{
						if ((value[2] == nullptr) || (radian = (sscanf(value[2]->c_str(), "%f", &r) == 1)) || sscanf(value[2]->c_str(), "d%f", &r) == 1)
						{
							if (!radian)
								r *= ALLEGRO_PI / 180;
							image.set_position(x, y, r);
							ret_val = true;
						}
					}
				}
			}
			else if (!strcmp(name, "ref_point"))
			{
				const char *(key[]) = { "ref_x","ref_y",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**) value))
				{
					int ref_x, ref_y;
					if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &ref_x) == 1 && sscanf(value[1]->c_str(), "%d", &ref_y) == 1))
					{
						image.set_reference_point(ref_x, ref_y);
						ret_val = true;
					}
				}
			}
			else 
				ret_val = false;
		}
		else
		{
			if (!strcmp(name, "visible"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					image.set_is_visible(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "image_bitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					image.set_bitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "draw_flag"))
			{
				if (!strcmp(it->text.c_str(), "FLIP_HORIZONTAL"))
					image.set_draw_flags(ALLEGRO_FLIP_HORIZONTAL);
				else if (!strcmp(it->text.c_str(), "FLIP_VERTICAL"))
					image.set_draw_flags(ALLEGRO_FLIP_VERTICAL);
				else
					ret_val = false;
			}
			else
				ret_val = false;
		}
	}
	if (!ret_val)
		cerr << "Error in GUI_image XML data, at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_text_data(const my_XML_element & element, GUI_element** created)
{
	//Text
	texts.push_front(GUI_text());
	GUI_text &text = texts.front();
	(*created) = &text;
	bool ret_val = true;
	const char* name = nullptr;
	for (list<my_XML_element>::const_iterator it = element.children.begin(); ret_val && it != element.children.end(); ++it)
	{
		name = it->name.c_str();
		if (it->children.size() != 0)
		{
			ret_val = false;
			break;
		}
		if (it->attributes.size() != 0)
		{
			if (!strcmp(name, "position"))
			{
				const char *(key[]) = { "x","y","r",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**) value))
				{
					int x, y;
					float r = 0;
					bool radian;
					if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &x) == 1 && sscanf(value[1]->c_str(), "%d", &y) == 1))
					{
						if ((value[2] == nullptr) || (radian = (sscanf(value[2]->c_str(), "%f", &r) == 1)) || sscanf(value[2]->c_str(), "d%f", &r) == 1)
						{
							if (!radian)
								r *= ALLEGRO_PI / 180;
							text.set_position(x, y, r);
							ret_val = true;
						}
					}
				}
			}
			else if (!strcmp(name, "textbox"))
			{
				const char *(key[]) = { "enabled","up","down","left","right","radius","color",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				ALLEGRO_COLOR color = al_map_rgb(0, 0, 0);
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					unsigned int up = 0, down = 0, left = 0, right = 0;
					float radius = 0;
					bool enabled = false;
					float r = 0;
					bool ok = true;
					//NOTE: TAB MISSING! read carefully
					//Note that if is "if key was not given or it was given and it's value could be loaded"
					if (
						(value[0] == nullptr || get_bool_from_string(*value[0], &enabled)) &&
						(value[1] == nullptr || (sscanf(value[1]->c_str(), "%u", &up))) &&
						(value[2] == nullptr || (sscanf(value[2]->c_str(), "%u", &down))) &&
						(value[3] == nullptr || (sscanf(value[3]->c_str(), "%u", &left))) &&
						(value[4] == nullptr || (sscanf(value[4]->c_str(), "%u", &right))) &&
						(value[5] == nullptr || (sscanf(value[5]->c_str(), "%f", &radius))) &&
						(value[6] == nullptr || get_color_from_string(*value[6], &color))
						)
					{
						text.set_textbox(left, right, up, down, radius, color);
						text.set_use_textbox(enabled);
						ret_val = true;
					}
				}
			}
			else if (!strcmp(name, "font"))
			{
				const char *(key[]) = { "size","type",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					unsigned int size;
					ALLEGRO_FONT* font;
					if ((value[0] != nullptr) && (value[1] != nullptr) && (sscanf(value[0]->c_str(), "%u", &size) == 1) && get_font_from_string(*value[1],size,&font))
					{
						text.set_font(font);
						ret_val = true;
					}
				}
			}
			else
				ret_val = false;
		}
		else
		{
			if (!strcmp(name, "visible"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					text.set_is_visible(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "text"))
			{
				if (it->text.size() != 0)
					text.set_text(it->text);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "text_color"))
			{
				ALLEGRO_COLOR color;
				if (get_color_from_string(it->text, &color))
					text.set_text_color(color);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "text_align"))
			{
				if (!strcmp(it->text.c_str(), "CENTER"))
					text.set_text_flags(ALLEGRO_ALIGN_CENTRE);
				else if (!strcmp(it->text.c_str(), "LEFT"))
					text.set_text_flags(ALLEGRO_ALIGN_LEFT);
				else if (!strcmp(it->text.c_str(), "RIGHT"))
					text.set_text_flags(ALLEGRO_ALIGN_RIGHT);
				else
					ret_val = false;
			}
			else
				ret_val = false;
		}
	}
	if (!ret_val)
		cerr << "Error in GUI_text XML data, at label \"" << name << "\"" << endl;
	return ret_val;

}

bool Gui::handle_gui_button_data(const my_XML_element & element, GUI_element** created)
{
	//Button
	buttons.push_front(GUI_button());
	GUI_button &button = buttons.front();
	(*created) = &button;
	bool ret_val = true;
	const char* name = nullptr;
	for (list<my_XML_element>::const_iterator it = element.children.begin(); it != element.children.end() && ret_val; ++it)
	{
		name = it->name.c_str();
		if (it->children.size() != 0)
		{
			ret_val = false;
			break;
		}
		if (it->attributes.size() != 0)
		{
			if (!strcmp(name, "position"))
			{
				const char *(key[]) = { "x","y","r",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					int x, y;
					float r = 0;
					bool radian;
					if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &x) == 1 && sscanf(value[1]->c_str(), "%d", &y) == 1))
					{
						if ((value[2] == nullptr) || (radian = (sscanf(value[2]->c_str(), "%f", &r) == 1)) || sscanf(value[2]->c_str(), "d%f", &r) == 1)
						{
							if (!radian)
								r *= ALLEGRO_PI / 180;
							button.set_position(x, y, r);
							ret_val = true;
						}
					}
				}
			}
			else if (!strcmp(name, "ref_point"))
			{
				const char *(key[]) = { "ref_x","ref_y",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**) value))
				{
					int ref_x, ref_y;
					if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%d", &ref_x) == 1 && sscanf(value[1]->c_str(), "%d", &ref_y) == 1))
					{
						button.set_reference_point(ref_x, ref_y);
						ret_val = true;
					}
				}
			}
			else
				ret_val = false;
		}
		else
		{
			if (!strcmp(name, "visible"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					button.set_is_visible(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "active"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					button.set_is_active(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "button_base_bitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.set_base_bitmap(bitmap);
				else
					ret_val = false;
			}
			else if(!strcmp(name, "button_hover_bitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.set_hover_bitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "button_click_bitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.set_click_bitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "button_aux_bitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.set_aux_bitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "check_alpha"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					button.set_check_alpha_for_mouse(flag);
				else
					ret_val = false;
			}
			else
				ret_val = false;
		}
	}
	if (!ret_val)
		cerr << "Error in GUI_button XML data, at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::get_bitmap_from_string(const string& filename, ALLEGRO_BITMAP** bitmap)
{
	bool ret_val = true;
	bitmap_dic_t::iterator it;
	if ((it = bitmap_dictionary.find(filename)) != bitmap_dictionary.end())
		(*bitmap) = it->second;
	else if ((*bitmap = al_load_bitmap(filename.c_str())) != nullptr)
		bitmap_dictionary[filename] = (*bitmap);
	else
		ret_val = false;
	if (!ret_val)
		cerr << "Error loading bitmap \"" << filename << "\"" << endl;
	return ret_val;
}

bool Gui::get_font_from_string(const string& filename, unsigned int size, ALLEGRO_FONT** font)
{
	bool ret_val = true;
	font_dic_t::iterator it;
	if ((it = font_dictionary.find(font_data_t(filename,size))) != font_dictionary.end())
		(*font) = it->second;
	else if ((*font = al_load_ttf_font(filename.c_str(),size,0)) != nullptr)
		font_dictionary[font_data_t(filename,size)] = (*font);
	else
		ret_val = false;
	if (!ret_val)
		cerr << "Error loading font \"" << filename << "\" with size " << size << "!" << endl;
	return ret_val;
}

bool Gui::handle_elements(const my_XML_element& element, list<GUI_element*>* list_p)
{
	bool ret_val = true;
	list<my_XML_element>::const_iterator it;
	for (list<my_XML_element>::const_iterator it2 = element.children.begin(); it2 != element.children.end() && ret_val; ++it2)
	{
		attr_t::const_iterator id;
		//If no id or id given and it is not repeated
		if (it2->attributes.size() != 1 || ((id = it2->attributes.find("id")) != it2->attributes.end() && id_dictionary.find(id->second) == id_dictionary.end()))
		{
			const char* name = it2->name.c_str();
			GUI_element* el_2_add = nullptr;
			if (!strcmp(name,"GUI_submenu"))
				ret_val &= handle_gui_submenu_data(*it2, &el_2_add);
			else if (!strcmp(name, "GUI_image"))
				ret_val &= handle_gui_image_data(*it2, &el_2_add);
			else if (!strcmp(name, "GUI_button"))
				ret_val &= handle_gui_button_data(*it2, &el_2_add);
			else if (!strcmp(name, "GUI_text"))
				ret_val &= handle_gui_text_data(*it2, &el_2_add);
			else
				ret_val = false;
			if (ret_val)
			{
				list_p->push_front(el_2_add);
				if (it2->attributes.size() == 1)	//Did it have an id?
				{
					id_dictionary[id->second] = el_2_add;
				}
			}
		}
		else
			ret_val = false;
	}
	return ret_val;
}

bool Gui::get_bool_from_string(const string & file, bool * result)
{
	bool ret_val = true;
	if(!strcmp(file.c_str(),"true"))
		*result = true;
	else if (!strcmp(file.c_str(), "false"))
		*result = false;
	else
		ret_val = false;
	if (!ret_val)
		cerr << "Error on bool value. Values must be either \"true\" or \"false\"." << endl;
	return ret_val;
}

bool Gui::get_color_from_string(const string & color_string, ALLEGRO_COLOR* color)
{
	bool ret_val = false;
	unsigned int result;
	char* byte_pointer = (char*)&result;
	//Little endian code
	//The following bool value was taken from
	//http://stackoverflow.com/questions/8899069/how-to-find-if-a-given-string-conforms-to-hex-notation-eg-0x34ff-without-regex
	bool is_hex_notation = color_string.compare(0, 2, "0x") == 0
		&& color_string.size() > 2
		&& color_string.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
	//Must be 10 characters(4 bytes: Red,Green,Blue,Alpha + "0x")
	if (is_hex_notation  && color_string.size() == 10 && sscanf(color_string.c_str(), "%x", &result) == 1)
	{
		*color = al_map_rgba(byte_pointer[3], byte_pointer[2], byte_pointer[1], byte_pointer[0]);
		ret_val = true;
	}
	if (!ret_val)
		cerr << "Error parsing color!" << endl;
	return ret_val;
}

bool Gui::get_attributes_from_strings(const attr_t &attr, const char ** key, const string **values)
{
	bool ret_val = true;
	for (unsigned int i = 0; key[i] != nullptr; values[i++] = nullptr);
	for (attr_t::const_iterator it = attr.begin(); it != attr.end(); ++it)
	{
		unsigned int i;
		for (i = 0; key[i] != nullptr; ++i)
		{
			if (!strcmp(key[i], it->first.c_str()))
			{
				values[i] = &(it->second);
				break;
			}
		}
		if (key[i] == nullptr)
		{
			ret_val = false;
			break;
		}
	}
	if (!ret_val)
		cerr << "Invalid attribute key given!" << endl;
	return ret_val;
}

void Gui::redraw()
{
	ALLEGRO_TRANSFORM backup, transform;
	al_copy_transform(&backup, al_get_current_transform());
	al_identity_transform(&transform);
	al_scale_transform(&transform, gui_sx, gui_sy);
	al_use_transform(&transform);
	al_clear_to_color(gui_backg_color);
	if (gui_background != nullptr)
		al_draw_scaled_bitmap(gui_background, 0, 0, al_get_bitmap_width(gui_background), al_get_bitmap_height(gui_background), 0, 0, gui_width, gui_height, 0);
	//Draw from back to front!
	for (list<GUI_element*>::reverse_iterator it = menu_element_list.rbegin(); it != menu_element_list.rend(); ++it)
		(*it)->draw();
	al_flip_display();
	al_use_transform(&backup);
}

bool Gui::feed_mouse_event(ALLEGRO_MOUSE_STATE & state)
{
	//Transform used in order to deal with scale
	ALLEGRO_TRANSFORM transform;
	//Some variables
	bool interacted_with_gui = false;
	bool exclusive_interacted_with_gui = false;
	list<GUI_element*>::iterator it;
	GUI_active_element* active_gui_element;
	GUI_element* interacted_element = nullptr;
	bool should_gui_close;
	bool redraw = false;
	bool redraw_flag = false;
	float x = state.x, y = state.y;

	//Use transforms
	al_identity_transform(&transform);
	al_scale_transform(&transform, gui_sx, gui_sy);
	//Invert transform
	al_invert_transform(&transform);
	//Need to convert from received coordinates, scaled, to logical coordinates
	al_transform_coordinates(&transform, &x, &y);
	state.x = x; state.y = y;

	bool any_element_needs_exclusive_attention = false;
	//Check if any element needs exclusive attention
	for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
		if ((active_gui_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
			if ((any_element_needs_exclusive_attention = active_gui_element->need_exclusive_attention()))
			{
				//If an element needs exclusive attention, feed mouse state only to that element
				exclusive_interacted_with_gui |= active_gui_element->feed_mouse_state(state, &should_gui_close, &redraw_flag);
				redraw |= redraw_flag;
				any_element_needs_exclusive_attention = active_gui_element->need_exclusive_attention();	//Update exclusive attention flag
				break;
			}

	if (!any_element_needs_exclusive_attention)	//If no element need exclusive attention, or one needed but no longer does
	{
		for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
			if ((active_gui_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
			{
				if ((interacted_with_gui |= (active_gui_element->feed_mouse_state(state, &should_gui_close, &redraw_flag))))
				{
					interacted_element = active_gui_element;
					redraw |= redraw_flag;
					break;	//Pass mouse state to elements until an interaction occurs
				}
				redraw |= redraw_flag;	//If menu is closed because it was volatile, it did not interact with mouse, but need redraw
			}
		if (interacted_with_gui)	//If an interaction ocurred
		{//Start from scratch, list may have changed due to callbacks. Inform other elements they are not in use
			for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
			{
				if (interacted_element == (*it))
					continue;	//Ommit tell not in use to used element
				if ((active_gui_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
				{
					active_gui_element->tell_not_in_use(&redraw_flag);
					redraw |= redraw_flag;
				}
			}
		}
	}
	return redraw;
}

GUI_element * Gui::get_element_from_id(const char* element_id)
{
	GUI_element* return_pointer = nullptr;
	id_dic_t::iterator it = id_dictionary.find(string(element_id));
	if (it != id_dictionary.end())
		return_pointer = it->second;
	return return_pointer;
}

void Gui::force_release_mouse()
{
	bool dummy;
	GUI_active_element* active_gui_element;
	for (list<GUI_element*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)	//For all elements
		if ((active_gui_element = dynamic_cast<GUI_active_element *>(*it)) != nullptr)
			active_gui_element->tell_not_in_use(&dummy);	//Tell them they are not in use
}

bool Gui::get_element_absolute_position(GUI_element* element, float * x, float * y, float * r)
{
	GUI_menu* menu;
	bool el_found = true;
	for (list<GUI_element*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
		//If element is inside gui
		if ((*it) == element)
		{
			//Just return it's position
			(*it)->get_position(x, y, r);
			el_found = true;
			break;
		}
	//Also look in menus within gui
		else if ((menu = dynamic_cast<GUI_menu*>(*it)) != nullptr)
			//If element is found in menu within menu
			if (menu->get_element_position(element, x, y, r))
			{
				float dummy, menu_r;
				menu->get_position(&dummy, &dummy, &menu_r);
				el_found = true;
				break;
			}
	//If no element was found
	if (!el_found)
		//Set values to zero
		(*x) = (*y) = (*r) = 0;
	//Return element found?
	return el_found;
}

bool Gui::set_element_absolute_position(GUI_element * element, float x, float y, float r)
{
	GUI_menu* menu;
	bool el_found = true;
	for (list<GUI_element*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
		//If element is inside gui
		if ((*it) == element)
		{
			//Just set it's position
			(*it)->set_position(x, y, r);
			el_found = true;
			break;
		}
	//Also look in menus within gui
		else if ((menu = dynamic_cast<GUI_menu*>(*it)) != nullptr)
			//If element is found in menu within menu
			if (menu->set_element_position(element, x, y, r))
			{
				el_found = true;
				break;
			}
	//Return element found?
	return el_found;
}

ALLEGRO_DISPLAY * Gui::get_display()
{
	return display;
}
