#ifndef PACKAGE_NAME_IS_H_
#define PACKAGE_NAME_IS_H_
#include "Package_hanabi.h"
#include <string>

using namespace std;

class Package_name_is : public Package_hanabi
{
public:
	Package_name_is();
	//Get name. True if success.
	bool get_name(string &name);
	//Set name. True if success.
	bool set_name(const string& name);
private:
	//Is raw data valid for Name_is package?
	virtual bool is_raw_data_valid(const char* data, size_t data_length);
};
#endif //PACKAGE_NAME_IS_H_