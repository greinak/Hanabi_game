#ifndef CARD_SKIN_H_
#define CARD_SKIN_H_

#include "card.h"
#include <allegro5\allegro5.h>

#define CARD_SKIN_DIRECTORY	"Hanabi_textures\\Cards"
#define CARD_SKIN_EXTENSION	"png"

//Filename:		%CARD_SKIN_DIRECTORY%\%COLOR%\card_%NUMBER%.%CARD_SKIN_EXTENSION%

class card_skin
{
public:
	//Load card skin
	card_skin();
	//Return true if skin loaded
	bool initialized_successfully();
	//Returns card bitmap. Nullptr if invalid card
	ALLEGRO_BITMAP* get_bitmap(const card& c);
	//Destroy card skin
	~card_skin();
private:
	ALLEGRO_BITMAP* skin[HANABI_TOTAL_COLORS][HANABI_TOTAL_NUMBERS];
	bool success;
};

#endif			//CARD_SKIN_H_