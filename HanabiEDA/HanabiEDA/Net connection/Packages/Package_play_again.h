#ifndef PACKAGE_PLAY_AGAIN_H_
#define PACKAGE_PLAY_AGAIN_H_
#include "Package_hanabi.h"

class Package_play_again : public Package_hanabi
{
public:
	Package_play_again();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_PLAY_AGAIN_H_