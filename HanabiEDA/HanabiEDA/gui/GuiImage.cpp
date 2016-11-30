#include "GuiImage.h"

GuiImage::GuiImage()
{
	this->default_bitmap = this->second_bitmap = nullptr;
	this->use_second_bitmap = false;
	this->draw_option = GUI_IMAGE_NORMAL;
}

void GuiImage::SetDefaultBitmap(ALLEGRO_BITMAP * bitmap)
{
	this->default_bitmap = bitmap;
}

void GuiImage::SetSecondBitmap(ALLEGRO_BITMAP * bitmap)
{
	this->second_bitmap = bitmap;
}

void GuiImage::useSecondBitmap(bool use_second_bitmap)
{
	this->use_second_bitmap = use_second_bitmap;
}

void GuiImage::SetDrawOption(draw_option_t draw_option)
{
	this->draw_option = draw_option;
}

void GuiImage::Draw()
{
	ALLEGRO_BITMAP* bitmap = use_second_bitmap ? second_bitmap : default_bitmap;
	int flag;
	switch (draw_option)
	{
	case GUI_IMAGE_NORMAL:
		flag = 0;
		break;
	case GUI_IMAGE_FLIP_HORIZONTAL:
		flag = ALLEGRO_FLIP_HORIZONTAL;
		break;
	case GUI_IMAGE_FLIP_VERTICAL:
		flag = ALLEGRO_FLIP_VERTICAL;
		break;
	case GUI_IMAGE_ROTATE_180:
		flag = ALLEGRO_FLIP_HORIZONTAL | ALLEGRO_FLIP_VERTICAL;
		break;
	default:
		flag = 0;
	}
	if (is_visible && bitmap != nullptr)
	{
		TransformToPosition();
		al_draw_bitmap(bitmap, -offset_x, -offset_y, flag);		//Draw bitmap, with offset
		al_use_transform(&backup);								//Restore transformation!!
	}
}
