#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <cstring>

class StringUtil
{
public:
    StringUtil();
    ~StringUtil();

public:
    static int StringToInt(const std::string& str);
    static double StringToDouble(const std::string& str);

public:
    static std::string ToString(int value);
    static std::string ToString(double value);
};

#endif