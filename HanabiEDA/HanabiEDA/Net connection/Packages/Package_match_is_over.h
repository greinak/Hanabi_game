#ifndef PACKAGE_GAME_IS_OVER_H_
#define PACKAGE_GAME_IS_OVER_H_
#include "Package_hanabi.h"

class Package_match_is_over : public Package_hanabi
{
public:
	Package_match_is_over();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_GAME_IS_OVER_H_