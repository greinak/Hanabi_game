#include "Package_play.h"

#define CARD_ID_SIZE 1
#pragma pack(push,1)
//Package
typedef struct package_data
{
	unsigned char header;
	unsigned char card_id;
}package_data;
#pragma pack(pop)

Package_play::Package_play()
{
	package_data* p_data = (package_data*)this->raw_data;
	p_data->header = PLAY_ID;
	p_data->card_id = 0; 
	this->raw_data_size = sizeof(package_data);
}

bool Package_play::get_card(hanabi_card_id * id)
{
	const package_data* p_data = (const package_data*)this->raw_data;
	*id = (hanabi_card_id) p_data->card_id;
	return true;
}

bool Package_play::set_card(hanabi_card_id id)
{
	package_data* p_data = (package_data*)this->raw_data;
	if (FIRST <= id && id <= SIXTH)
	{
		p_data->card_id = id;
		return true;
	}
	return false;
}

bool Package_play::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == PLAY_ID && FIRST <= p_data->card_id && p_data->card_id <= SIXTH)
		return true;
	return false;
}
