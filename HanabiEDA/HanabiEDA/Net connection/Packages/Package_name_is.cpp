 #include "Package_name_is.h"
#include <climits>
#include <algorithm>

#define NAME_COUNT_SIZE	1
#define MAX_NAME_LENGTH UCHAR_MAX	//Max name length, counter is unsigned char

#pragma pack(push,1)
//Package
typedef struct package_data
{
	unsigned char header;
	unsigned char name_length;
	char name[MAX_PACKAGE_SIZE - HEADER_SIZE - NAME_COUNT_SIZE];
}package_data;
#pragma pack(pop)

Package_name_is::Package_name_is()
{
	//Empty package
	package_data* p_data = (package_data*) this->raw_data;
	p_data->header = NAME_IS_ID;
	p_data->name_length = 0;
	raw_data_size = sizeof(p_data->header) + sizeof(p_data->name_length);
}

bool Package_name_is::get_name(string & name)
{
	//Just return name
	const package_data* p_data = (const package_data*) this->raw_data;
	if (p_data->name_length != 0)
	{
		name = string(p_data->name, p_data->name_length);
		return true;
	}
	return false;
}

bool Package_name_is::set_name(const string & name)
{
	package_data* p_data = (package_data*) this->raw_data;
	//Is name size ok?
	if (name.length() < MAX_NAME_LENGTH && name.length() != 0)
	{
		find_if_not(name.begin(), name.end(), isprint);
		if(find_if_not(name.begin(), name.end(), isprint) == name.end())	//All printable ASCII?
		{
			//Copy name to package
			memcpy(p_data->name, name.c_str(), name.length());
			//Update name length
			p_data->name_length = (unsigned char) name.length();
			//Update package raw data length;
			raw_data_size = p_data->name_length + HEADER_SIZE + NAME_COUNT_SIZE;
			return true;
		}
	}
	return false;
}

bool Package_name_is::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*) data;
	//Correct header id? is size coherent?
	if (data_length - HEADER_SIZE - NAME_COUNT_SIZE == p_data->name_length && p_data->header == NAME_IS_ID && p_data->name_length != 0)
	{
		string name(p_data->name, p_data->name_length);
		if (find_if_not(name.begin(), name.end(), isprint) == name.end())	//All printable ascii?
			return true;
	}
	return false;
}
