#ifndef PACKAGE_DRAW_H_
#define PACKAGE_DRAW_H_

#include "Package_hanabi.h"
#include "../../card.h"

class Package_draw : public Package_hanabi
{
public:
	Package_draw();
	bool set_card(card c);
	bool get_card(card *c);
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};
#endif //PACKAGE_DRAW_H_