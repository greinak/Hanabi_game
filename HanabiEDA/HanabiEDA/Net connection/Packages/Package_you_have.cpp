#include "Package_you_have.h"
#include "card_hanabi.h"

//Package
#pragma pack(push,1)
typedef struct package_data
{
	unsigned char header;
	union
	{
		hanabi_color color;
		hanabi_number number;
	}card_data;
}package_data;
#pragma pack(pop)

Package_you_have::Package_you_have()
{
	package_data* p_data = (package_data*)this->raw_data;
	p_data->header = YOU_HAVE_ID;
	p_data->card_data.color = NO_CARD_COLOR;
	p_data->card_data.number = NO_CARD_NUMBER;	//Actually, it's an union, but ok
	this->raw_data_size = sizeof(package_data);
}

bool Package_you_have::set_clue(card clue)  
{
	package_data* p_data = (package_data*)raw_data; 
	if (clue.get_color() == NO_COLOR)
	{
		switch (clue.get_number())
		{
		case NUMBER_ONE:
			p_data->card_data.number = ONE;
			break;
		case NUMBER_TWO:
			p_data->card_data.number = TWO;
			break;
		case NUMBER_THREE:
			p_data->card_data.number = THREE;
			break;
		case NUMBER_FOUR:
			p_data->card_data.number = FOUR;
			break;
		case NUMBER_FIVE:
			p_data->card_data.number = FIVE;
			break;
		default:
			return false;
		}
	}
	else if (clue.get_number() == NO_NUMBER)
	{
		switch(clue.get_color())
		{
		case COLOR_BLUE:
			p_data->card_data.color = BLUE;
			break;
		case COLOR_GREEN:
			p_data->card_data.color = GREEN;
			break;
		case COLOR_RED:
			p_data->card_data.color = RED;
			break;
		case COLOR_WHITE:
			p_data->card_data.color = WHITE;
			break;
		case COLOR_YELLOW:
			p_data->card_data.color = YELLOW;
			break;
		default:
			return false;
		}
	}
	return true;
}

bool Package_you_have::get_clue(card *clue)
{
	package_data* p_data = (package_data*)raw_data;
	card_color_t color = NO_COLOR;
	card_number_t number = NO_NUMBER;
	if (p_data->card_data.color != RED && p_data->card_data.color != YELLOW &&
		p_data->card_data.color != GREEN && p_data->card_data.color != BLUE &&
		p_data->card_data.color != WHITE)
	{
		switch (p_data->card_data.number)
		{
		case ONE:
			number = NUMBER_ONE;
			break;
		case TWO:
			number = NUMBER_TWO;
			break;
		case THREE:
			number = NUMBER_THREE;
			break;
		case FOUR:
			number = NUMBER_FOUR;
			break;
		case FIVE:
			number = NUMBER_FIVE;
			break;
		default:
			return false;
		}
	}
	else if (p_data->card_data.number != ONE &&
			p_data->card_data.number != TWO && p_data->card_data.number != THREE &&
			p_data->card_data.number != FOUR && p_data->card_data.number != FIVE)
	{
		switch (p_data->card_data.color)
		{
		case BLUE:
			color = COLOR_BLUE;
			break;
		case GREEN:
			color = COLOR_GREEN;
			break;
		case RED:
			color = COLOR_RED;
			break;
		case WHITE:
			color = COLOR_WHITE;
			break;
		case YELLOW:
			color = COLOR_YELLOW;
			break;
		default:
			return false;
		}
	}
	(*clue) = card(color, number);
	return true;
}

bool Package_you_have::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == YOU_HAVE_ID)
		return (p_data->card_data.color == RED || p_data->card_data.color == YELLOW ||
			p_data->card_data.color == GREEN || p_data->card_data.color == BLUE ||
			p_data->card_data.color == WHITE || p_data->card_data.number == ONE ||
			p_data->card_data.number == TWO || p_data->card_data.number == THREE ||
			p_data->card_data.number == FOUR || p_data->card_data.number == FIVE);
	return false;
}