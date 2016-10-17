#include "Package_error.h"

Package_error::Package_error()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = ERROR_ID;
}

bool Package_error::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && ((unsigned char) data[0]) == ERROR_ID;
}
 