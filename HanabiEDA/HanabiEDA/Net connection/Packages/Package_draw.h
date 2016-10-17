#ifndef PACKAGE_DRAW_H_
#define PACKAGE_DRAW_H_

#include "Package_hanabi.h"
#include "card_hanabi.h"

class Package_draw : public Package_hanabi
{
public:
	Package_draw();
	bool set_card(card_hanabi card);
	bool get_card(card_hanabi *card);
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
	bool is_card_valid(card_hanabi card);
};
#endif //PACKAGE_DRAW_H_