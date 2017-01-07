#ifndef _GUI_IMAGE_H_
#define _GUI_IMAGE_H_

#include "GuiElement.h"
#include <allegro5\allegro5.h>

typedef enum DRAW_OPTIONS {GUI_IMAGE_NORMAL,GUI_IMAGE_FLIP_HORIZONTAL,GUI_IMAGE_FLIP_VERTICAL,GUI_IMAGE_ROTATE_180} draw_option_t;

class GuiImage : public GuiElement
{
public:
	GuiImage();	//Initialization
	void SetDefaultBitmap(ALLEGRO_BITMAP* bitmap);		//Set default image bitmap
	void SetSecondBitmap(ALLEGRO_BITMAP* bitmap);		//Set second bitmap
	void SetUseSecondBitmap(bool use_second_bitmap);		//Use second bitmap?
	void SetDrawOption(draw_option_t draw_option);			//Set image draw flag
	virtual void Draw();								//Draw image
private:
	ALLEGRO_BITMAP* default_bitmap, *second_bitmap;
	draw_option_t draw_option;	//Default GUI_IMAGE_NORMAL
	bool use_second_bitmap;	//Default false
};

#endif