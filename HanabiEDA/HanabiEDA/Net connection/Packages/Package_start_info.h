#ifndef PACKAGE_START_INFO_H_
#define PACKAGE_START_INFO_H_
#include "Package_hanabi.h"
#include "card_hanabi.h"

class Package_start_info : public Package_hanabi
{
public:
	Package_start_info();
	bool set_info(const card_hanabi client_hand[HAND_CARDS], const card_hanabi server_hand[HAND_CARDS]);
	bool get_info(card_hanabi client_hand[HAND_CARDS], card_hanabi server_hand[HAND_CARDS]);
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
	bool check_cards(const card_hanabi client_hand[HAND_CARDS], const card_hanabi server_hand[HAND_CARDS]);
};

#endif	//PACKAGE_START_INFO_H_