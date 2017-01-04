#ifndef PACKAGE_START_INFO_H_
#define PACKAGE_START_INFO_H_
#include "Package_hanabi.h"
#include "../../card.h"

#define TOTAL_HAND_CARDS 6
class Package_start_info : public Package_hanabi
{
public:
	Package_start_info();
	bool set_info(const card receptor_hand[TOTAL_HAND_CARDS], const card transmisor_hand[TOTAL_HAND_CARDS]);
	bool get_info(card receptor_hand[TOTAL_HAND_CARDS], card transmisor_hand[TOTAL_HAND_CARDS]);
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif	//PACKAGE_START_INFO_H_