#include "gui_image.h"

//Author: Gonzalo Julian Reina Kiperman

GUI_image::GUI_image()
{
	this->bitmap = nullptr;
	this->draw_flags = 0;
	this->ref_x = this->ref_y = 0;
}

void GUI_image::set_bitmap(ALLEGRO_BITMAP * bitmap)
{
	this->bitmap = bitmap;
}

void GUI_image::set_draw_flags(int draw_flags)
{
	this->draw_flags = draw_flags;
}

void GUI_image::set_reference_point(float ref_x, float ref_y)
{
	this->ref_x = ref_x;
	this->ref_y = ref_y;
}

void GUI_image::draw()
{
	if (is_visible)
	{
		if (bitmap != nullptr)
		{
			//Use transformations to achiveve desired position and rotation
			ALLEGRO_TRANSFORM backup, rotated;
			al_copy_transform(&backup, al_get_current_transform());	//Backup current transform
			al_identity_transform(&rotated);						//Create new transform
			al_rotate_transform(&rotated, r);						//Rotate it
			al_translate_transform(&rotated, x, y);					//Translate it
			al_compose_transform(&rotated, &backup);				//Compose it with backup
			al_use_transform(&rotated);								//Use it
			al_draw_bitmap(bitmap, -ref_x, -ref_y, draw_flags);		//Draw bitmap, with offset
			al_use_transform(&backup);								//Restore transformation!!
		}
	}
}

