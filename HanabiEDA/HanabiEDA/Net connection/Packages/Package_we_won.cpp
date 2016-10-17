#include "Package_we_won.h"

Package_we_won::Package_we_won()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = WE_WON_ID;
}

bool Package_we_won::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == WE_WON_ID;
}
 