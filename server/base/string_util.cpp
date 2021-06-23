#include "base/string_util.h"

StringUtil::StringUtil() {}

StringUtil::~StringUtil() {}

int StringUtil::StringToInt(const std::string& str) 
{
    return std::atoi(str.c_str());
}

double StringUtil::StringToDouble(const std::string& str) 
{
    return std::atof(str.c_str());
}

std::string StringUtil::ToString(int value)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return std::string(buffer);
}

std::string StringUtil::ToString(double value)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.15f", value);
    return std::string(buffer);
}
