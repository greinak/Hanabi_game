#include "Package_match_is_over.h"

Package_match_is_over::Package_match_is_over()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = MATCH_IS_OVER_ID;
}

bool Package_match_is_over::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == MATCH_IS_OVER_ID;
}
 