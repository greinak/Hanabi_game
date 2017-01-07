#ifndef _GUI_TEXT_H_
#define _GUI_TEXT_H_

#include <allegro5\allegro5.h>
#include <allegro5\allegro_font.h>
#include <string>
#include "GuiElement.h"

using namespace std;

typedef enum TEXT_ALIGN {GUI_TEXT_LEFT, GUI_TEXT_CENTER, GUI_TEXT_RIGHT} align_t;
class GuiText : public GuiElement
{
public:
	GuiText();
	void SetText(const string& text);	//Set text
	string GetText();
	void SetUseTextbox(bool use_textbox);	//Use textbox?
	void SetTextbox(float tb_l_clearance, float tb_r_clearance, float tb_u_clearance, float tb_d_clearance, float tb_radius, ALLEGRO_COLOR tb_color);	//Customize textbox
	void SetFont(ALLEGRO_FONT* font);	//Font to use
	void SetTextColor(ALLEGRO_COLOR t_color);	//Text color
	void SetAlign(align_t align);	//Text align?
	virtual void Draw();
private:
	string text;
	bool use_textbox;	//Default false
	float tb_u_clearance, tb_d_clearance, tb_l_clearance, tb_r_clearance, tb_radius;	//Default (0,0,0)
	ALLEGRO_FONT* font;
	ALLEGRO_COLOR t_color, tb_color;	//Black default
	align_t align;	//Defualt GUI_TEXT_LEFT
};

#endif