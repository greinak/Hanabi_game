#include "Package_name.h"

Package_name::Package_name()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = NAME_ID;
}

bool Package_name::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == NAME_ID; 
}
 