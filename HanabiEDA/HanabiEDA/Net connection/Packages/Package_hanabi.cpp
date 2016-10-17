#include "Package_hanabi.h"
#include <string>

Package_hanabi::Package_hanabi()
{
	//Initialize package size to zero
	this->raw_data_size = 0;
}

bool Package_hanabi::load_raw_data(const char * data, size_t data_length)
{
	//Is data bigger than MAX_PACKAGE_SIZE? Is it valid?
	if (data_length <= MAX_PACKAGE_SIZE && is_raw_data_valid(data,data_length))
	{
		//Yes, copy data
		memcpy(this->raw_data, data, this->raw_data_size = data_length); 
		return true;
	}
	//No.
	return false;
}

bool Package_hanabi::get_raw_data(char * dest, size_t out_buffer_size, size_t * raw_data_size)
{
	//Is out buffer big enought?
	if (out_buffer_size >= this->raw_data_size)
	{
		//Yes, copy data
		memcpy(dest, this->raw_data, *raw_data_size = this->raw_data_size);
		return true;
	}
	//No.
	return false;
}

package_type get_package_type_from_raw_data(const char * data, size_t data_length)
{
	if(data_length != 0)
		switch ((unsigned char)data[0])
		{
		case ACK_ID: return ACK_P;
		case NAME_ID: return NAME_P;
		case NAME_IS_ID: return NAME_IS_P;
		case START_INFO_ID: return START_INFO_P;
		case YOU_START_ID: return YOU_START_P;
		case I_START_ID: return I_START_P;
		case DISCARD_ID: return DISCARD_P;
		case PLAY_ID: return PLAY_P;
		case YOU_HAVE_ID: return YOU_HAVE_P;
		case DRAW_ID: return DRAW_P;
		case WE_WON_ID: return WE_WON_P;
		case WE_LOST_ID: return WE_LOST_P;
		case MATCH_IS_OVER_ID: return MATCH_IS_OVER_P;
		case PLAY_AGAIN_ID: return PLAY_AGAIN_P;
		case GAME_OVER_ID: return GAME_OVER_P;
		case QUIT_ID: return QUIT_P;
		case ERROR_ID: return ERROR_P;
		}
	return UNKNOWN_PACKAGE_P;
}
