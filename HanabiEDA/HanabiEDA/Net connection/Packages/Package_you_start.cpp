#include "Package_you_start.h"

Package_you_start::Package_you_start()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = YOU_START_ID;
}

bool Package_you_start::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == YOU_START_ID;
}
