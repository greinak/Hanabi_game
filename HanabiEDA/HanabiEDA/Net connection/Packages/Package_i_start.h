#ifndef PACKAGE_I_START_H_
#define PACKAGE_I_START_H_
#include "Package_hanabi.h"

class Package_i_start : public Package_hanabi
{
public:
	Package_i_start();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_I_START_H_