#define ALLEGRO_STATICLINK
#include <allegro5\allegro5.h>
#include <allegro5\allegro_primitives.h>
#include "gui_text.h"


//Author: Gonzalo Julian Reina Kiperman

GUI_text::GUI_text() : text()
{
	this->box_color = al_map_rgb(0, 0, 0);
	this->text_color = al_map_rgb(0, 0, 0);
	this->up_clearance = this->down_clearance = 0;
	this->left_clearance = this->right_clearance = 0;
	this->font = nullptr;
	this->box_radius = 0;
	this->use_textbox = false;
	this->text_flags = 0;
}

void GUI_text::set_text(string text)
{
	this->text = text;
}

void GUI_text::set_use_textbox(bool use_textbox)
{
	this->use_textbox = use_textbox;
}

void GUI_text::set_textbox(float left_clearance, float right_clearance, float up_clearance, float down_clearance, float box_radius, ALLEGRO_COLOR box_color)
{
	this->left_clearance = left_clearance;
	this->right_clearance = right_clearance;
	this->up_clearance = up_clearance;
	this->down_clearance = down_clearance;
	this->box_color = box_color;
	this->box_radius = box_radius;
}

void GUI_text::set_font(ALLEGRO_FONT * font)
{
	this->font = font;
}

void GUI_text::set_text_color(ALLEGRO_COLOR text_color)
{
	this->text_color = text_color;
}

void GUI_text::set_text_flags(int text_flags)
{
	this->text_flags = text_flags;
}

void GUI_text::draw()
{
	if (is_visible && font != nullptr && text.length() != 0)
	{
		//Use transformations to achiveve desired position and rotation
		ALLEGRO_TRANSFORM backup, rotated;
		al_copy_transform(&backup, al_get_current_transform());		//Backup current transform
		al_identity_transform(&rotated);							//Create new transform
		al_rotate_transform(&rotated, r);							//Rotate it
		al_translate_transform(&rotated, x, y);						//Translate it
		al_compose_transform(&rotated, &backup);					//Compose it with backup
		al_use_transform(&rotated);									//Use it
		//Now draw
		if (use_textbox)
		{
			int box_x, box_y, box_w, box_h;
			float startx, starty, endx, endy;
			al_get_text_dimensions(font, text.c_str(), &box_x, &box_y, &box_w, &box_h);
			if (text_flags == ALLEGRO_ALIGN_LEFT)
				box_x -= 0;	//Do nothing
			else if (text_flags == ALLEGRO_ALIGN_CENTER)
			{
				box_x -= box_w / 2;
			}
			else if (text_flags == ALLEGRO_ALIGN_RIGHT)
				box_x -= box_w;
			startx = box_x - left_clearance;
			starty = box_y - up_clearance;
			endx = box_x + box_w + left_clearance + right_clearance;
			endy = box_y + box_h + up_clearance + down_clearance;
			al_draw_filled_rounded_rectangle(startx, starty, endx, endy, box_radius, box_radius, box_color);
		}
		al_draw_text(font, text_color, 0, 0, text_flags, text.c_str());
		al_use_transform(&backup);	//Restore transformation!!
	}
}
