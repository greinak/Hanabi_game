#ifndef _GUI_BUTTON_H_
#define _GUI_BUTTON_H_

#include "GuiActiveElement.h"
#include <allegro5\allegro5.h>

#define GUI_BUTTON_ALPHA_THRESHOLD	5

//Callback function:
class GuiButton;
typedef bool(GUI_button_callback)(GuiButton* source, bool forced, bool mouse_over_element, void* user_data, bool* redraw);
//Source: pointer to button
//Forced: caused by ReleaseMouse
//mouse_over_element: as it says. invalid if forced
//Redraw: preset in false, set in true if should redraw
//Return true if should close submenu container. Return value does nothing if button is not in a submenu

class GuiButton : public GuiActiveElement
{
public:
	GuiButton();
	void SetBitmap(ALLEGRO_BITMAP* bitmap);			//Button bitmap, if nullptr button is disabled
	void SetHoverBitmap(ALLEGRO_BITMAP* bitmap);	//Bitmap to show when hovering element
	void SetClickBitmap(ALLEGRO_BITMAP* bitmap);	//Bitmap to show when clicking element
	void SetTopBitmap(ALLEGRO_BITMAP* bitmap);		//Optional bitmap to be drawn above button
	void SetUseTopBitmap(bool use_top_bitmap);		//Use optional bitmap?
	void SetUserData(void* user_data);				//Set user data for callbacks
	void SetCheckBitmapAlpha(bool check_bitmap_alpha);	//if bitmap is transparent in mouse, ignore

	//nullptr in order to disable.

	//When mouse is clicked down while hovering bitmap
	void SetOnClickDownCallback(GUI_button_callback* on_click_down);
	//When mouse is clicked down, and then a mouse movement occurs. Even if mouse is no longer over button
	void SetOnClickMovementCallback(GUI_button_callback* on_click_movement);
	//When mouse button is released, only if mouse is still over button
	void SetOnClickUpCallback(GUI_button_callback* on_click_up);
	//When mouse enters hovering button (if mouse button is not down). Or after mouse is released over button
	void SetOnHoverEnterCallback(GUI_button_callback* on_hover_enter);
	//When mouse moves hovering over button, but mouse is not clicked
	void SetOnHoverMovementCallback(GUI_button_callback* on_hover_movement);
	//When mouse leaves button. Or, if button was clicked down, before click down callback
	void SetOnHoverExitCallback(GUI_button_callback* on_hover_exit);

	virtual bool ReleaseMouse();	//True if should redraw
	virtual void Draw();
	virtual bool FeedMouseState(const ALLEGRO_MOUSE_STATE& st, bool *close_contaner, bool* redraw);	//This is where the magic happens

private:
	ALLEGRO_BITMAP* base_bitmap, *hover_bitmap, *click_bitmap, *top_bitmap;
	bool use_top_bitmap;	//Default false
	void* user_data;
	bool check_bitmap_alpha;	//Default false
	GUI_button_callback* on_click_down;
	GUI_button_callback* on_click_movement;
	GUI_button_callback* on_click_up;
	GUI_button_callback* on_hover_enter;
	GUI_button_callback* on_hover_movement;
	GUI_button_callback* on_hover_exit;
	bool is_being_hovered, is_being_clicked;
};
#endif