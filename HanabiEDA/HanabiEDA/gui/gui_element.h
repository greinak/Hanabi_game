#ifndef GUI_ELEMENT_H_
#define GUI_ELEMENT_H_

//This is the base class for any GUI screen element

//Author: Gonzalo Julian Reina Kiperman

class GUI_element
{
public:
	//Variables initialization
	GUI_element();
	//Sets position of GUI_element
	//R Rotates GUI_element r radians.
	//Rotation is intended to be used only during initialization
	//Or while element is not being active
	//Be careful if modifying rotation while using it in an active element
	//This note only applies to GUI_active_element. This was not tested.
	void set_position(float x, float y, float r);	//Default (0,0,0)
	//Get position
	void get_position(float *x, float *y, float *r);
	//Sets if GUI_element is visible
	void set_is_visible(bool is_visible);	//Default true
	//Draw GUI_element, if it is visible
	virtual void draw() = 0;
protected:
	float x, y, r;
	bool is_visible;
};

#endif //GUI_ELEMENT_H_