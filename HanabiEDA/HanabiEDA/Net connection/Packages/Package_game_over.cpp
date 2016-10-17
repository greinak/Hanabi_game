#include "Package_game_over.h"

Package_game_over::Package_game_over()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = GAME_OVER_ID;
}

bool Package_game_over::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == GAME_OVER_ID; 
}
