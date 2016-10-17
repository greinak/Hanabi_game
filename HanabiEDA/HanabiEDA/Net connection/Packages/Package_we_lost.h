#ifndef PACKAGE_WE_LOST_H_
#define PACKAGE_WE_LOST_H_
#include "Package_hanabi.h"

class Package_we_lost : public Package_hanabi
{
public:
	Package_we_lost();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_WE_LOST_H_