#include "Package_ack.h"

Package_ack::Package_ack()
{
	this->raw_data_size = HEADER_SIZE;
	this->raw_data[0] = ACK_ID;
}

bool Package_ack::is_raw_data_valid(const char * data, size_t data_length)
{
	return data_length == HEADER_SIZE && data[0] == ACK_ID;  
}
