#include "Package_we_lost.h"

Package_we_lost::Package_we_lost()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = WE_LOST_ID;
}

bool Package_we_lost::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == WE_LOST_ID;
}
 