#ifndef PACKAGE_HANABI_H_
#define PACKAGE_HANABI_H_

#define MAX_PACKAGE_SIZE 512	//Bytes
#define HEADER_SIZE 1	//Byte

typedef enum ID : unsigned char
{
	ACK_ID = 0x01,NAME_ID = 0x10,NAME_IS_ID = 0x11,
	START_INFO_ID = 0x12,YOU_START_ID = 0x20,I_START_ID = 0x21,
	DISCARD_ID = 0x32,PLAY_ID = 0x33,YOU_HAVE_ID = 0x34,
	DRAW_ID = 0x35,WE_WON_ID = 0x40,WE_LOST_ID = 0x41,
	MATCH_IS_OVER_ID = 0x42,PLAY_AGAIN_ID = 0x50,GAME_OVER_ID = 0x51,
	QUIT_ID = 0xFE,ERROR_ID = 0xFF
}ID;

typedef enum package_type : unsigned char
{
	ACK_P,NAME_P,NAME_IS_P,START_INFO_P,YOU_START_P,I_START_P,
	DISCARD_P,PLAY_P,YOU_HAVE_P,DRAW_P,WE_WON_P,WE_LOST_P,MATCH_IS_OVER_P,
	PLAY_AGAIN_P,GAME_OVER_P,QUIT_P,ERROR_P,UNKNOWN_PACKAGE_P
}package_type;	//These enums are in order to separate protocol from package management

class Package_hanabi
{
public:
	Package_hanabi();
	//Load raw data to package. True if success. Loading data also verifies if data makes sense
	bool load_raw_data(const char* data, size_t data_length);
	//Get raw data to package. True if success
	bool get_raw_data(char* dest, size_t out_buffer_size, size_t* raw_data_size);
protected:
	//Is raw data valid for this package type?
	virtual bool is_raw_data_valid(const char* data, size_t data_length) = 0;
	//Package data
	char raw_data[MAX_PACKAGE_SIZE];
	unsigned int raw_data_size;
};

package_type get_package_type_from_raw_data(const char* data, size_t data_length);

#endif	//PACKAGE_HANABI_H_