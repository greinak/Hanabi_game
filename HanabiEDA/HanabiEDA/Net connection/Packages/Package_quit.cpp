#include "Package_quit.h"

Package_quit::Package_quit()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = QUIT_ID;
}

bool Package_quit::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && ((unsigned char)data[0]) == QUIT_ID;
}
 