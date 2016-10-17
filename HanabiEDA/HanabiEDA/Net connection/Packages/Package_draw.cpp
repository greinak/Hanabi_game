#include "Package_draw.h"

#pragma pack(push,1)
//Package
typedef struct
{
	unsigned char header;
	card_hanabi card;
}package_data; 
#pragma pack(pop)

Package_draw::Package_draw()
{
	package_data* p_data = (package_data*)raw_data; 
	card_hanabi card;
	card.color = NO_COLOR;
	card.number = NO_NUMBER;
	this->raw_data_size = sizeof(package_data);
	p_data->header = DRAW_ID;
	p_data->card = card;
}

bool Package_draw::set_card(card_hanabi card)
{
	package_data* p_data = (package_data*)raw_data;
	if (is_card_valid(card))
	{
		p_data->card = card;
		return true;
	}
	return false;
}

bool Package_draw::get_card(card_hanabi * card)
{
	const package_data* p_data = (const package_data*)raw_data;
	if (is_card_valid(p_data->card))
	{
		*card = p_data->card;
		return true;
	}
	return false;
}

bool Package_draw::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	return data_length == HEADER_SIZE && data[0] == DRAW_ID && is_card_valid(p_data->card);
}

bool Package_draw::is_card_valid(card_hanabi card)
{
	bool is_color_valid = false;
	bool is_number_valid = false;
	bool is_special_card = false;
	is_color_valid |= card.color == RED;
	is_color_valid |= card.color == YELLOW;
	is_color_valid |= card.color == GREEN;
	is_color_valid |= card.color == BLUE;
	is_color_valid |= card.color == WHITE;
	is_number_valid = ONE <= card.number && card.number <= SIX;
	is_special_card = card.color == NO_COLOR && card.number == NO_NUMBER;
	return (is_color_valid && is_number_valid) || is_special_card;
}
