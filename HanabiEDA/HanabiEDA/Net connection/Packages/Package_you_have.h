#ifndef PACKAGE_YOU_HAVE_H_
#define PACKAGE_YOU_HAVE_H_
#include "Package_hanabi.h"
#include "../../card.h"
class Package_you_have : public Package_hanabi
{
public:
	Package_you_have();
	bool set_clue(card clue);
	bool get_clue(card* clue);
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_YOU_HAVE_H_