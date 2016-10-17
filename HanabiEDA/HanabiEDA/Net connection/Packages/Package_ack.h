#ifndef PACKAGE_ACK_H_
#define PACKAGE_ACK_H_
#include "Package_hanabi.h"

class Package_ack : public Package_hanabi
{
public:
	Package_ack();
private:
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};

#endif //PACKAGE_ACK_H_