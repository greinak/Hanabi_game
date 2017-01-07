#include "card.h"
#include <algorithm>

card::card(card_color_t color, card_number_t number)
{
	this->color = color;
	this->number = number;
}

card_color_t card::get_color() const
{
	return color;
}

card_number_t card::get_number() const
{
	return number;
}

string card::get_name() const
{
	if ((*this) != card(NO_COLOR, NO_NUMBER))
	{
		string card_name(color_string[color]);
		card_name += "-";
		card_name += '0' + number + 1;
		transform(card_name.begin(), card_name.end(), card_name.begin(), toupper);
		return card_name;
	}
	return "NO CARD";
}

string card::get_short_name() const
{
	if ((*this) != card(NO_COLOR, NO_NUMBER))
	{
		char c = *color_string[color];
		string card_name;
		card_name += c;	//Just initial letter
		card_name += '0' + number + 1;
		transform(card_name.begin(), card_name.end(), card_name.begin(), toupper);
		return card_name;
	}
	return "--";
}

bool card::operator==(const card & other) const
{
	return (this->color == other.color && this->number == other.number);
}

bool card::operator!=(const card & other) const
{
	return !(this->operator==(other));
}
