#define _CRT_SECURE_NO_WARNINGS
#include "gui.h"
#include <expat.h>
#include <stack>
#include <map>
#include <list>
#include <istream>
#include <string>
#include <algorithm>
#include <allegro5\allegro.h>
#include <allegro5\allegro_ttf.h>

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
	//Initialize everything
	this->display = nullptr;
	this->gui_background = nullptr;
	this->gui_icon = nullptr;
	this->gui_sx = this->gui_sy = 1;
	this->gui_backg_color = al_map_rgb(0, 0, 0);
	this->gui_height = this->gui_width = 0;
	this->initialized = false;

	XML_Parser parser = XML_ParserCreate(nullptr);
	XML_parser_object data(&parser);
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	char buf[BUFSIZ];
	bool done = false;
	cout << "[GUI][INFO] : Reading GUI XML data..." << endl;
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
				done = (xml.peek() == EOF);
				if (XML_Parse(parser, buf, (size_t)len, done) == XML_STATUS_ERROR)
					break;
			} while (!done);
		}
	}
	XML_ParserFree(parser);	//Parsing finished
	my_XML_element parsed_data = data.get_parsed_data();	//Get data
	if (done && data.finished())	//is main element GuiMenu? 
																			//has parsed finished? or was is stopped?
	{
		cout << "[GUI][INFO] : Data loaded OK! parsing data and loading resources..." << endl;
		if (!parsed_data.name.compare("GuiMenu") && handle_gui_menu_data(parsed_data))	//This will convert data to GUI data
		{
			parsed_data.children.clear();	//No need to keep this info, we can free memory by clearing it...
			cout << "[GUI][INFO] : Data parsed successfully! Opening GUI..." << endl;
			//Now, create display in order to show menu
			if ((display = al_create_display(gui_width*gui_sx, gui_height*gui_sy)) != nullptr)
			{
				cout << "[GUI][INFO] : GUI \"" << gui_title  <<"\" opened!..." << endl;
				if (gui_title.size() != 0)
					al_set_window_title(display, gui_title.c_str());
				if (gui_icon != nullptr)
					al_set_display_icon(display, gui_icon);
				redraw();
				this->initialized = true;		//Success!!
				return;
			}
			else
				cerr << "[GUI][ERROR] : Could not create display for GUI..." << endl;
		}
		else
			cerr << "[GUI][ERROR] : Invalid XML GUI data..." << endl;
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
		cerr << "[GUI][ERROR] : Could not load XML data... check XML file." << endl;
}

bool Gui::initialized_successfully()
{
	return initialized;
}

Gui::~Gui()
{
	//Note that destructor of data structs will be called automatically
	if (initialized)
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
		cout << "[GUI][INFO] : GUI \"" << gui_title << "\" closed." << endl;
	}
}

//##########

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
		if (!strcmp(name, "elements") && it->attributes.size() == 0)	//Is element label? (No attributes for element labels)
			ret_val = handle_elements(*it, &menu_element_list);				//Then load elements
		else if (it->children.size() == 0)	//No other element has children
		{
			if (it->attributes.size() != 0)		//Has it got attributes?
			{
				if(!strcmp(name,"size"))
				{
					const char *(key[]) = { "width","height",nullptr };
					string* (value[sizeof(key) / sizeof(key[0])-1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**) value))
					{
						unsigned int width, height;
						char c;	//To check if string is ONLY float
						if (value[0] != nullptr && value[1] != nullptr && sscanf(value[0]->c_str(), "%u%c", &width, &c) == 1 && sscanf(value[1]->c_str(), "%u%c", &height, &c) == 1)
						{
							gui_width = width;
							gui_height = height;
							ret_val = true;
						}
					}
				}
				else if (!strcmp(name, "scale"))
				{
					const char *(key[]) = { "sx","sy",nullptr};
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**) value))
					{
						float sx, sy;
						char c;	//To check if string is ONLY float
						if (value[0] != nullptr && value[1] != nullptr && sscanf(value[0]->c_str(), "%f%c", &sx, &c) == 1 && sscanf(value[1]->c_str(), "%f%c", &sy, &c) == 1)
						{
							gui_sx = sx;
							gui_sy = sy;
							ret_val = true;	
						}
					}
				}
				else 
					ret_val = false;
			}
			else
			{
				if(!strcmp(name,"backgColor"))
				{
					ALLEGRO_COLOR color;
					if (get_color_from_string(it->text, &color))
						gui_backg_color = color;
					else
						ret_val = false;
				}
				else if(!strcmp(name,"backgBitmap"))
				{
					if ((gui_background = al_load_bitmap(it->text.c_str())) == nullptr)
						ret_val = false;
				}
				else if(!strcmp(name,"iconBitmap"))
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
		cerr << "[GUI][ERROR] : Could not load GUI XML Data. Error at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_submenu_data(const my_XML_element & element, GuiElement** created)
{
	//Submenu
	submenus.push_front(GuiSubmenu());
	GuiSubmenu &submenu = submenus.front();
	(*created) = &submenu;
	bool ret_val = true;
	list<GuiElement*> el_list;
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
					if (get_attributes_from_strings(it->attributes, key, (const string**)value))
					{
						float x, y, r;
						ret_val = handle_position(value[0], value[1], value[2], &x, &y, &r);
						if (ret_val)
							submenu.SetPosition(x, y, r);
					}
				}
				else if (!strcmp(name,"offset"))
				{
					const char *(key[]) = { "x","y",nullptr };
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**)value))
					{
						float x, y;
						ret_val = handle_position(value[0], value[1], nullptr, &x, &y, nullptr);
						if (ret_val)
							submenu.SetOffset(x, y);
					}
				}
				else if(!strcmp(name,"background"))
				{
					const char *(key[]) = { "width","height","radius","color",nullptr};
					string* (value[sizeof(key) / sizeof(key[0]) - 1]);
					submenu.SetUseBackground(true);
					ret_val = false; //true condition is easier to do
					if (get_attributes_from_strings(it->attributes, key, (const string**)value))
					{
						unsigned int width, height;
						float b_radius = 0;
						char c;
						ALLEGRO_COLOR color = al_map_rgb(0,0,0);
						if (value[0] != nullptr && value[1] != nullptr && (sscanf(value[0]->c_str(), "%u%c", &width,&c) == 1 && sscanf(value[1]->c_str(), "%u%c", &height,&c) == 1))
						{
							if (value[2] == nullptr || sscanf(value[2]->c_str(), "%f%c", &b_radius, &c) == 1)
							{
								if (value[3] != nullptr)
								{
									ret_val = get_color_from_string(*value[3], &color);
								}
								else
									ret_val = true;
								submenu.SetBackground(width, height, b_radius, color);
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
						submenu.SetIsVisible(flag);
					else
						ret_val = false;
				}
				else if (!strcmp(name,"active"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.SetIsActive(flag);
					else
						ret_val = false;
				}
				else if (!strcmp(name,"volatile"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.SetIsVolatile(flag);
					else
						ret_val = false;
				}
				else if (!strcmp(name, "blocker"))
				{
					bool flag;
					if (get_bool_from_string(it->text, &flag))
						submenu.SetIsBlocker(flag);
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
	if (ret_val)
	{
		for (list<GuiElement*>::iterator it = el_list.begin(); it != el_list.end(); it++)
			submenu.AddElement(*it);
	}
	else
		cerr << "[GUI][ERROR] : Could not load GUI XML Data. Error at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_image_data(const my_XML_element & element, GuiElement** created)
{
	//Image
	images.push_front(GuiImage());
	GuiImage &image = images.front();
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
			if (!strcmp(name, "position"))
			{
				const char *(key[]) = { "x","y","r",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					float x, y, r;
					ret_val = handle_position(value[0], value[1], value[2], &x, &y, &r);
					if (ret_val)
						image.SetPosition(x, y, r);
				}
			}
			else if (!strcmp(name, "offset"))
			{
				const char *(key[]) = { "x","y",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					float x, y;
					ret_val = handle_position(value[0], value[1], nullptr, &x, &y, nullptr);
					if (ret_val)
						image.SetOffset(x, y);
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
					image.SetIsVisible(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "imageBitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					image.SetDefaultBitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "secondBitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					image.SetSecondBitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "drawFlag"))
			{
				if (!strcmp(it->text.c_str(), "FLIP_HORIZONTAL"))
					image.SetDrawOption(GUI_IMAGE_FLIP_HORIZONTAL);
				else if (!strcmp(it->text.c_str(), "FLIP_VERTICAL"))
					image.SetDrawOption(GUI_IMAGE_FLIP_VERTICAL);
				else
					ret_val = false;
			}
			else
				ret_val = false;
		}
	}
	if (!ret_val)
		cerr << "[GUI][ERROR] : Could not load GUI XML Data. Error at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_button_data(const my_XML_element & element, GuiElement** created)
{
	//Button
	buttons.push_front(GuiButton());
	GuiButton &button = buttons.front();
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
					float x, y, r;
					ret_val = handle_position(value[0], value[1], value[2], &x, &y, &r);
					if (ret_val)
						button.SetPosition(x, y, r);
				}
			}
			else if (!strcmp(name, "offset"))
			{
				const char *(key[]) = { "x","y",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					float x, y;
					ret_val = handle_position(value[0], value[1], nullptr, &x, &y, nullptr);
					if (ret_val)
						button.SetOffset(x, y);
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
					button.SetIsVisible(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "active"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					button.SetIsActive(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "baseBitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.SetBitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "hoverBitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.SetHoverBitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "clickBitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.SetClickBitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "button_aux_bitmap"))
			{
				ALLEGRO_BITMAP *bitmap;
				if (get_bitmap_from_string(it->text.c_str(), &bitmap))
					button.SetTopBitmap(bitmap);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "checkAlpha"))
			{
				bool flag;
				if (get_bool_from_string(it->text, &flag))
					button.SetCheckBitmapAlpha(flag);
				else
					ret_val = false;
			}
			else
				ret_val = false;
		}
	}
	if (!ret_val)
		cerr << "[GUI][ERROR] : Could not load GUI XML Data. Error at label \"" << name << "\"" << endl;
	return ret_val;
}

bool Gui::handle_gui_text_data(const my_XML_element & element, GuiElement** created)
{
	//Text
	texts.push_front(GuiText());
	GuiText &text = texts.front();
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
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					float x, y, r;
					ret_val = handle_position(value[0], value[1], value[2], &x, &y, &r);
					if (ret_val)
						text.SetPosition(x, y, r);
				}
			}
			else if (!strcmp(name, "offset"))
			{
				const char *(key[]) = { "x","y",nullptr };
				string* (value[sizeof(key) / sizeof(key[0]) - 1]);
				ret_val = false; //true condition is easier to do
				if (get_attributes_from_strings(it->attributes, key, (const string**)value))
				{
					float x, y;
					ret_val = handle_position(value[0], value[1], nullptr, &x, &y, nullptr);
					if (ret_val)
						text.SetOffset(x, y);
				}
			}
			else if (!strcmp(name, "textbox"))
			{
				text.SetUseTextbox(true);
				const char *(key[]) = { "up","down","left","right","radius","color",nullptr };
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
					char c;
					if (
						(value[0] == nullptr || (sscanf(value[0]->c_str(), "%u%c", &up,&c)==1)) &&
						(value[1] == nullptr || (sscanf(value[1]->c_str(), "%u%c", &down,&c)==1)) &&
						(value[2] == nullptr || (sscanf(value[2]->c_str(), "%u%c", &left, &c)==1)) &&
						(value[3] == nullptr || (sscanf(value[3]->c_str(), "%u%c", &right, &c)==1)) &&
						(value[4] == nullptr || (sscanf(value[4]->c_str(), "%f%c", &radius, &c)==1)) &&
						(value[5] == nullptr || get_color_from_string(*value[5], &color))
						)
					{
						text.SetTextbox(left, right, up, down, radius, color);
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
					char c;
					if ((value[0] != nullptr) && (value[1] != nullptr) && (sscanf(value[0]->c_str(), "%u%c", &size,&c) == 1) && get_font_from_string(*value[1],size,&font))
					{
						text.SetFont(font);
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
					text.SetIsVisible(flag);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "text"))
			{
				if (it->text.size() != 0)
					text.SetText(it->text);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "textColor"))
			{
				ALLEGRO_COLOR color;
				if (get_color_from_string(it->text, &color))
					text.SetTextColor(color);
				else
					ret_val = false;
			}
			else if (!strcmp(name, "textAlign"))
			{
				if (!strcmp(it->text.c_str(), "CENTER"))
					text.SetAlign(GUI_TEXT_CENTER);
				else if (!strcmp(it->text.c_str(), "LEFT"))
					text.SetAlign(GUI_TEXT_LEFT);
				else if (!strcmp(it->text.c_str(), "RIGHT"))
					text.SetAlign(GUI_TEXT_RIGHT);
				else
					ret_val = false;
			}
			else
				ret_val = false;
		}
	}
	if (!ret_val)
		cerr << "[GUI][ERROR] : Could not load GUI XML Data. Error at label \"" << name << "\"" << endl;
	return ret_val;

}

bool Gui::handle_elements(const my_XML_element& element, list<GuiElement*>* list_p)
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
			GuiElement* el_2_add = nullptr;
			if (!strcmp(name,"GuiSubmenu"))
				ret_val &= handle_gui_submenu_data(*it2, &el_2_add);
			else if (!strcmp(name, "GuiImage"))
				ret_val &= handle_gui_image_data(*it2, &el_2_add);
			else if (!strcmp(name, "GuiButton"))
				ret_val &= handle_gui_button_data(*it2, &el_2_add);
			else if (!strcmp(name, "GuiText"))
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
		cerr << "[GUI][ERROR] : Could not load resource: [IMAGE]\"" << filename << "\"" << endl;
	return ret_val;
}

bool Gui::get_font_from_string(const string& filename, unsigned int size, ALLEGRO_FONT** font)
{
	bool ret_val = true;
	font_dic_t::iterator it;
	if ((it = font_dictionary.find(font_data_t(filename, size))) != font_dictionary.end())
		(*font) = it->second;
	else if ((*font = al_load_ttf_font(filename.c_str(), size, 0)) != nullptr)
		font_dictionary[font_data_t(filename, size)] = (*font);
	else
		ret_val = false;
	if (!ret_val)
		cerr << "[GUI][ERROR] : Could not load resource: [FONT]\"" << filename << "\", size: \"" << size << endl;
	return ret_val;
}

bool Gui::get_bool_from_string(const string & string, bool * result)
{
	bool ret_val = true;
	if(!strcmp(string.c_str(),"true"))
		*result = true;
	else if (!strcmp(string.c_str(), "false"))
		*result = false;
	else
		ret_val = false;
	if (!ret_val)
		cerr << "[GUI][ERROR] : Invalid bool value. Bool values must be either \"true\" or \"false\"." << endl;
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
	char c;
	if (is_hex_notation  && color_string.size() == 10 && sscanf(color_string.c_str(), "%x%c", &result,&c) == 1)
	{
		*color = al_map_rgba(byte_pointer[3], byte_pointer[2], byte_pointer[1], byte_pointer[0]);
		ret_val = true;
	}
	if (!ret_val)
		cerr << "[GUI][ERROR] : Invalid color value." << endl;
	return ret_val;
}

bool Gui::get_attributes_from_strings(const attr_t &attr, const char ** key, const string **values)
{
	bool ret_val = true;
	unsigned int i;
	for (i = 0; key[i] != nullptr; values[i++] = nullptr);
	for (attr_t::const_iterator it = attr.begin(); it != attr.end(); ++it)
	{
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
		if (!ret_val)
			cerr << "[GUI][ERROR] : Invalid attribute key given!" << endl;
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
	for (list<GuiElement*>::reverse_iterator it = menu_element_list.rbegin(); it != menu_element_list.rend(); ++it)
		(*it)->Draw();
	al_flip_display();
	al_use_transform(&backup);
}

bool Gui::handle_position(string* x, string *y, string *r, float* fx, float* fy, float* fr)
{
	char c;
	bool success;
	bool parse_r;
	if (success = (x != nullptr && y != nullptr))
	{
		if ((success &= sscanf(x->c_str(), "%f%c", fx, &c) == 1 && sscanf(y->c_str(), "%f%c", fy, &c) == 1))
		{
			parse_r = (r != nullptr && fr != nullptr);
			if (fr != nullptr)
				*fr = 0;
			if (success && parse_r)
			{
				success &= sscanf(r->c_str(), "%f%c", fr, &c) == 1;
				*fr *= ALLEGRO_PI / 180;
			}
		}
	}
	return success;
}

bool Gui::feed_mouse_event(ALLEGRO_MOUSE_STATE & state)
{
	//Transform used in order to deal with scale
	ALLEGRO_TRANSFORM transform;
	//Some variables
	bool interacted_with_gui = false;
	bool exclusive_interacted_with_gui = false;
	list<GuiElement*>::iterator it;
	GuiActiveElement* active_gui_element;
	GuiElement* interacted_element = nullptr;
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
		if ((active_gui_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
			if ((any_element_needs_exclusive_attention = active_gui_element->NeedsExclusiveAttention()))
			{
				//If an element needs exclusive attention, feed mouse state only to that element
				exclusive_interacted_with_gui |= active_gui_element->FeedMouseState(state, &should_gui_close, &redraw_flag);
				redraw |= redraw_flag;
				any_element_needs_exclusive_attention = active_gui_element->NeedsExclusiveAttention();	//Update exclusive attention flag
				break;
			}

	if (!any_element_needs_exclusive_attention)	//If no element need exclusive attention, or one needed but no longer does
	{
		for (it = menu_element_list.begin(); it != menu_element_list.end(); ++it)
			if ((active_gui_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
			{
				if ((interacted_with_gui |= (active_gui_element->FeedMouseState(state, &should_gui_close, &redraw_flag))))
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
				if ((active_gui_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
				{
					redraw_flag |= active_gui_element->ReleaseMouse();
					redraw |= redraw_flag;
				}
			}
		}
	}
	return redraw;
}

GuiElement * Gui::get_element_from_id(const char* element_id)
{
	//This function is extremely important!
	GuiElement* return_pointer = nullptr;
	id_dic_t::iterator it = id_dictionary.find(string(element_id));
	if (it != id_dictionary.end())
		return_pointer = it->second;
	return return_pointer;
}

void Gui::force_release_mouse()
{
	GuiActiveElement* active_gui_element;
	for (list<GuiElement*>::iterator it = menu_element_list.begin(); it != menu_element_list.end(); ++it)	//For all elements
		if ((active_gui_element = dynamic_cast<GuiActiveElement *>(*it)) != nullptr)
			active_gui_element->ReleaseMouse();	//Tell them they are not in use
}

ALLEGRO_DISPLAY * Gui::get_display()
{
	return display;
}
