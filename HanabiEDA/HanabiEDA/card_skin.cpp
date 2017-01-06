#include "card_skin.h"
#include <string>

using namespace std;

card_skin::card_skin()
{
	this->success = true;
	this->blackside = nullptr;
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS; c++)
		for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS; n++)
			this->skin[c][n] = nullptr;	//First, initialize all to nullptr
	string directory(CARD_SKIN_DIRECTORY);
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS && this->success; c++)
		for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS && this->success; n++)
		{
			char number = '0' + n + 1;
			string filename((((((directory + '\\') + color_string[c] + '\\') + "card_") + number) + '.') + CARD_SKIN_EXTENSION);
			this->success &= ((this->skin[c][n] = al_load_bitmap(filename.c_str())) != nullptr);
		}
	if (this->success)
	{
		string filename((((directory + '\\') + BLACKSIDE_FILENAME) + '.') + CARD_SKIN_EXTENSION);
		this->success &= ((this->blackside = al_load_bitmap(filename.c_str())) != nullptr);
	}
}

bool card_skin::initialized_successfully()
{
	return this->success;
}

ALLEGRO_BITMAP * card_skin::get_bitmap(const card & c)
{
	if ((COLOR_BLUE <= c.get_color() && c.get_color() <= COLOR_YELLOW) && (NUMBER_ONE <= c.get_number() && c.get_number() <= NUMBER_FIVE))
		return skin[c.get_color()][c.get_number()];
	else if (c == card(NO_COLOR, NO_NUMBER))
		return blackside;
	return nullptr;
}

card_skin::~card_skin()
{
	for (unsigned int c = 0; c < HANABI_TOTAL_COLORS && this->success; c++)
		for (unsigned int n = 0; n < HANABI_TOTAL_NUMBERS && this->success; n++)
			al_destroy_bitmap(this->skin[c][n]);
}
