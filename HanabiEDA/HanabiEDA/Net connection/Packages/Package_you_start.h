#ifndef PACKAGE_YOU_START_H_
#define PACKAGE_YOU_START_H_
#include "Package_hanabi.h"

class Package_you_start : public Package_hanabi
{
public:
	Package_you_start();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_YOU_START_H_