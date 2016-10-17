#ifndef PACKAGE_NAME_H_
#define PACKAGE_NAME_H_
#include "Package_hanabi.h"

class Package_name : public Package_hanabi
{
public:
	Package_name();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_NAME_H_