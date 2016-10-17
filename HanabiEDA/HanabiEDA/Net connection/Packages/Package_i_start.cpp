#include "Package_i_start.h"

Package_i_start::Package_i_start()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = I_START_ID;
}

bool Package_i_start::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == I_START_ID;
}
 