#ifndef PACKAGE_QUIT_H_
#define PACKAGE_QUIT_H_
#include "Package_hanabi.h"

class Package_quit : public Package_hanabi
{
public:
	Package_quit();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_QUIT_H_