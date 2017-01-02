#include "card.h"

card::card(card_color_t color, card_number_t number)
{
	this->color = color;
	this->number = number;
}

card_color_t card::get_color()
{
	return color;
}

card_number_t card::get_number()
{
	return number;
}

bool card::operator==(const card & other)
{
	return (this->color == other.color && this->number == other.number);
}
