#ifndef _GUI_ELEMENT_H_
#define _GUI_ELEMENT_H_

#include <allegro5\allegro.h>
class GuiElement
{
public:
	GuiElement();									//Initialization
	void SetPosition(float x, float y, float r);	//Set position of GuiElement
	void SetOffset(float offset_x, float offset_y);	//offset will be substracted from position
	void GetPosition(float *x, float* y, float *r); //Get position of GuiElement
	void SetIsVisible(bool is_visible);				//Is element visible?
	virtual void Draw() = 0;						//Draw element
protected:
	void TransformToPosition();
	ALLEGRO_TRANSFORM backup;
	float x, y, r;				//Default (0,0,0)
	float offset_x, offset_y;	//Default (0,0)
	bool is_visible;	//Default true
};

#endif