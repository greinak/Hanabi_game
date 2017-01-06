#include "Package_discard.h"

#define MIN_POSITION 1
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

Package_discard::Package_discard()
{
	package_data* p_data = (package_data*)this->raw_data;
	p_data->header = DISCARD_ID;
	p_data->card_id = 0;
	this->raw_data_size = sizeof(package_data);
}

bool Package_discard::get_card_id(unsigned int * id)
{
	const package_data* p_data = (const package_data*)this->raw_data;
	if (MIN_POSITION <= p_data->card_id && p_data->card_id <= MAX_POSITION)
	{
		(*id) = p_data->card_id - 1;	//Protocol represents card 0 to 5 as card 1 to 6
		return true;
	}
	return false;
}

bool Package_discard::set_card_id(unsigned int id)
{
	package_data* p_data = (package_data*)this->raw_data;
	if (MIN_POSITION <= (id+1) && (id+1) <= MAX_POSITION)
	{
		p_data->card_id = id + 1;		//Protocol represents card 0 to 5 as card 1 to 6
		return true;
	}
	return false;
}

bool Package_discard::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == DISCARD_ID && MIN_POSITION <= p_data->card_id && p_data->card_id <= MAX_POSITION)
		return true;
	return false;
}
