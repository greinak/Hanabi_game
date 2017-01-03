#include "Package_play.h"

#define MIN_POSITION 0
#define MAX_POSITION 6
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

bool Package_play::get_card_id(unsigned int * id)
{
	const package_data* p_data = (const package_data*)this->raw_data;
	return MIN_POSITION <= p_data->card_id && p_data->card_id <= MAX_POSITION;
}

bool Package_play::set_card_id(unsigned int id)
{
	package_data* p_data = (package_data*)this->raw_data;
	if (MIN_POSITION <= id && id <= MAX_POSITION)
	{
		p_data->card_id = id;
		return true;
	}
	return false;
}

bool Package_play::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == PLAY_ID && MIN_POSITION <= p_data->card_id && p_data->card_id <= MAX_POSITION)
		return true;
	return false;
}
