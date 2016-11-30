#ifndef GUI_TEXT_H_
#define GUI_TEXT_H_

#include "gui_element.h"
#include <allegro5\allegro5.h>
#include <allegro5\allegro_ttf.h>
#include <string>
using namespace std;

//Author: Gonzalo Julian Reina Kiperman

//Implementation of text
class GUI_text : public GUI_element
{
public:
	//Variables initialization
	GUI_text();
	//Set text to show
	void set_text(string text);
	//Use textbox?
	void set_use_textbox(bool use_textbox); //default is false
	//Set textbox. Default is all zero and black box
	void set_textbox(float left_clearance, float right_clearance, float up_clearance, float down_clearance, float box_radius, ALLEGRO_COLOR box_color);
	//Set font to use
	void set_font(ALLEGRO_FONT* font);	//Initialized in nullptr
	//Set text color
	void set_text_color(ALLEGRO_COLOR text_color);	//Default is black
	//Set text flags, use allegro flags
	void set_text_flags(int text_flags);	//Allignment also sets center of rotation. Default is 0
	//Draw text
	virtual void draw();
	//When you set the poistion of this element
	//You set the position of the LEFT, MIDDLE, OR RIGHT top part of text (NOT TEXT BOX)
	//LEFT, MIDDLE or RIGHT according to alignment
private:
	string text;
	ALLEGRO_FONT* font;
	bool use_textbox;
	float left_clearance, right_clearance, up_clearance, down_clearance;
	float box_radius;
	ALLEGRO_COLOR box_color, text_color;
	int text_flags;
};

#endif