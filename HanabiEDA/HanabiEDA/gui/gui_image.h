#ifndef GUI_IMAGE_H_
#define GUI_IMAGE_H_

#include "gui_element.h"
#include <allegro5\allegro.h>

//Author: Gonzalo Julian Reina Kiperman

//Implementation of an image
class GUI_image : public GUI_element
{
public:
	//Variables initialization
	GUI_image();
	//Set image bitmap
	void set_bitmap(ALLEGRO_BITMAP* bitmap);
	//Set image flags (use allegro flags for drawing)
	void set_draw_flags(int draw_flags);	//Default is 0
	//Set reference point. You set the position of this point within bitmap. This is also the center of rotation
	void set_reference_point(float ref_x, float ref_y);	//Default is (0,0)
	//Draw image
	virtual void draw();
private:
	ALLEGRO_BITMAP* bitmap;
	int draw_flags;
	float ref_x, ref_y;
};

#endif //GUI_IMAGE_H_