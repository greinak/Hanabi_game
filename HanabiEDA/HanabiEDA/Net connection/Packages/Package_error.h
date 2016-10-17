#ifndef PACKAGE_ERROR_H_
#define PACKAGE_ERROR_H_
#include "Package_hanabi.h"

class Package_error : public Package_hanabi
{
public:
	Package_error();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_ERROR_H_