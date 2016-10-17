#include "Package_play_again.h"

Package_play_again::Package_play_again()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = PLAY_AGAIN_ID;
}

bool Package_play_again::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == PLAY_AGAIN_ID;
}
 