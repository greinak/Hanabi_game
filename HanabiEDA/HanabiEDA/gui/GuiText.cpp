#include "GuiText.h"
#include <allegro5\allegro_primitives.h>

GuiText::GuiText()
{
	this->align = GUI_TEXT_LEFT;
	this->font = nullptr;
	this->tb_color = this->t_color = al_map_rgb(0, 0, 0);
	this->tb_l_clearance = 0;
	this->tb_r_clearance = 0;
	this->tb_u_clearance = 0;
	this->tb_d_clearance = 0;
	this->use_textbox = false;
}

void GuiText::SetText(const string & text)
{
	this->text = text;
}

string GuiText::getText()
{
	return this->text;
}

void GuiText::SetUseTextbox(bool use_textbox)
{
	this->use_textbox = use_textbox;
}

void GuiText::SetTextbox(float tb_l_clearance, float tb_r_clearance, float tb_u_clearance, float tb_d_clearance, float tb_radius, ALLEGRO_COLOR tb_color)
{
	this->tb_l_clearance = tb_l_clearance;
	this->tb_r_clearance = tb_r_clearance;
	this->tb_u_clearance = tb_u_clearance;
	this->tb_d_clearance = tb_d_clearance;
	this->tb_radius = tb_radius;
	this->tb_color = tb_color;
}

void GuiText::SetFont(ALLEGRO_FONT * font)
{
	this->font = font;
}

void GuiText::SetTextColor(ALLEGRO_COLOR t_color)
{
	this->t_color = t_color;
}

void GuiText::SetAlign(align_t align)
{
	this->align = align;
}

void GuiText::Draw()
{
	int flag;
	switch (align)
	{
		case GUI_TEXT_LEFT:
			flag = ALLEGRO_ALIGN_LEFT;
			break;
		case GUI_TEXT_CENTER:
			flag = ALLEGRO_ALIGN_CENTER;
			break;
		case GUI_TEXT_RIGHT:
			flag = ALLEGRO_ALIGN_RIGHT;
			break;
		default:
			flag = ALLEGRO_ALIGN_LEFT;
			break;
	}
	if (is_visible && font != nullptr && text.length() != 0)
	{
		TransformToPosition();
		if (use_textbox)
		{
			int box_x, box_y, box_w, box_h;
			float startx, starty, endx, endy;
			al_get_text_dimensions(font, text.c_str(), &box_x, &box_y, &box_w, &box_h);
			if (align == GUI_TEXT_LEFT)
				box_x -= 0;	//Do nothing
			else if (align == GUI_TEXT_CENTER)
			{
				box_x -= box_w / 2;
			}
			else if (align == GUI_TEXT_RIGHT)
				box_x -= box_w;
			startx = box_x - tb_l_clearance;
			starty = box_y - tb_u_clearance;
			endx = box_x + box_w + (tb_l_clearance+ tb_r_clearance);
			endy = box_y + box_h + (tb_u_clearance + tb_d_clearance);
			al_draw_filled_rounded_rectangle(startx, starty, endx, endy, tb_radius, tb_radius, tb_color);
		}
		al_draw_text(font, t_color, 0, 0, flag, text.c_str());
		al_use_transform(&backup);	//Restore transformation!!
	}
}
