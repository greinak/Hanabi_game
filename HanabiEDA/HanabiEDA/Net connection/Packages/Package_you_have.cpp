#include "Package_you_have.h"

//Package
#pragma pack(push,1)
typedef struct
{
	unsigned char header;
	union
	{
		hanabi_color color;
		hanabi_number number;
	};
}package_data;
#pragma pack(pop)

Package_you_have::Package_you_have()
{
	package_data* p_data = (package_data*)this->raw_data;
	p_data->header = YOU_HAVE_ID;
	p_data->color = NO_COLOR;
	this->raw_data_size = sizeof(package_data);
}

bool Package_you_have::set_clue(card_hanabi clue)  
{
	package_data* p_data = (package_data*)raw_data; 
	if (clue.color == ANY_COLOR || clue.number == ANY_NUMBER)
	{
		if (clue.color != ANY_COLOR)
		{
			if (clue.color == RED || clue.color == YELLOW ||
				clue.color == GREEN || clue.color == BLUE ||
				clue.color == WHITE)
			{
				p_data->color = clue.color;
				return true;
			}
		}
		else if (clue.number != ANY_NUMBER)
		{
			if (ONE <= clue.number && clue.number <= SIX)
			{
				p_data->number = clue.number;
				return true;
			}
		}
	}
	return false;
}

bool Package_you_have::get_clue(card_hanabi *clue)
{
	package_data* p_data = (package_data*)raw_data;
	if (p_data->color == RED || p_data->color == YELLOW ||
		p_data->color == GREEN || p_data->color == BLUE ||
		p_data->color == WHITE)
	{
		(*clue).color = p_data->color;
		(*clue).number = ANY_NUMBER;
	}
	else if (ONE <= p_data->number && p_data->number <= SIX)
	{
		(*clue).number = p_data->number;
		(*clue).color = ANY_COLOR;
	}
	return false;
}

bool Package_you_have::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == YOU_HAVE_ID)
		return (p_data->color == RED || p_data->color == YELLOW ||
			p_data->color == GREEN || p_data->color == BLUE ||
			p_data->color == WHITE) || (ONE <= p_data->number && p_data->number <= SIX);
	return false;
}