#ifndef PACKAGE_PLAY_H_
#define PACKAGE_PLAY_H_
#include "Package_hanabi.h"
#include "card_hanabi.h"

using namespace std;

class Package_play : public Package_hanabi
{
public:
	Package_play();
	//Get card. True if success.
	bool get_card(hanabi_card_id *id);
	//Set card. True if success.
	bool set_card(hanabi_card_id id);
private:
	//Is raw data valid for Discard package?
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};
#endif //PACKAGE_DISCARD_H_