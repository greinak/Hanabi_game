#include "Package_draw.h"
#include "card_hanabi.h"
#include <algorithm>

using namespace std;

#define TOTAL_HAND_CARDS 6

#pragma pack(push,1)
//Package
typedef struct package_data
{
	unsigned char header;
	card_hanabi card;
}package_data; 
#pragma pack(pop)

Package_draw::Package_draw()
{
	package_data* p_data = (package_data*)raw_data; 
	card_hanabi card;
	card.color = NO_CARD_COLOR;
	card.number = NO_CARD_NUMBER;
	this->raw_data_size = sizeof(package_data);
	p_data->header = DRAW_ID;
	p_data->card = card;
}

bool Package_draw::set_card(card c)
{
	card_hanabi h_card;
	bool card_ok = true;
	package_data* p_data = (package_data*)raw_data;
	switch (c.get_color())
	{
	case COLOR_RED:
		h_card.color = RED;
		break;
	case COLOR_GREEN:
		h_card.color = GREEN;
		break;
	case COLOR_BLUE:
		h_card.color = BLUE;
		break;
	case COLOR_YELLOW:
		h_card.color = YELLOW;
		break;
	case COLOR_WHITE:
		h_card.color = WHITE;
		break;
	default:
		card_ok = false;
	}
	switch (c.get_number())
	{
	case NUMBER_ONE:
		h_card.number = ONE;
		break;
	case NUMBER_TWO:
		h_card.number = TWO;
		break;
	case NUMBER_THREE:
		h_card.number = THREE;
		break;
	case NUMBER_FOUR:
		h_card.number = FOUR;
		break;
	case NUMBER_FIVE:
		h_card.number = FIVE;
		break;
	default:
		card_ok = false;
	}
	if (card_ok)
		p_data->card = h_card;
	return card_ok;
}

bool Package_draw::get_card(card * c)
{
	bool card_ok = true;
	card_color_t color;
	card_number_t number;
	package_data* p_data = (package_data*)raw_data;
	switch (p_data->card.color)
	{
	case RED:
		color = COLOR_RED;
		break;
	case GREEN:
		color = COLOR_GREEN;
		break;
	case BLUE:
		color = COLOR_BLUE;
		break;
	case YELLOW:
		color = COLOR_YELLOW;
		break;
	case WHITE:
		color = COLOR_WHITE;
		break;
	default:
		card_ok = false;
	}
	switch (p_data->card.number)
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
		card_ok = false;
	}
	if (card_ok)
		(*c) = card(color, number);
	return card_ok;
}

bool Package_draw::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	bool data_valid = data_length == sizeof(package_data) && data[0] == DRAW_ID;
	if (data_valid)
	{
		unsigned char colors[] = { BLUE,GREEN,RED,WHITE,YELLOW };
		unsigned char numbers[] = { ONE,TWO,THREE,FOUR,FIVE };
		unsigned char color_length = sizeof(colors) / sizeof(colors[0]);
		unsigned char number_length = sizeof(numbers) / sizeof(numbers[0]);
		data_valid &= find(colors, colors + color_length, p_data->card.color) != colors + color_length;
		data_valid &= find(numbers, numbers + number_length, p_data->card.number) != numbers + number_length;
		data_valid |= p_data->card.color == NO_CARD_COLOR && p_data->card.number == NO_CARD_NUMBER;
	}
	return data_valid;
}
