#include "base/string_util.h"
#include "base/time_util.h"
#include <iostream>

using namespace std;

int main()
{
    int a = 1234;
    cout<<TimeUtil::GetCurrentMs()<<endl;
    cout << StringUtil::ToString(a)<<endl;
    cout<<StringUtil::ToString(1234.12523)<<endl;
    cout<<StringUtil::StringToDouble("123.21534")<<endl;
    cout<<StringUtil::StringToInt("123455324")<<endl;

    cout<<TimeUtil::GetCurrentSecond()<<endl;
    cout<<TimeUtil::GetFormatTime()<<endl;
    cout<<TimeUtil::GetCurrentMs()<<endl;
    cout<<TimeUtil::GetCurrentStringSecond()<<endl;
}