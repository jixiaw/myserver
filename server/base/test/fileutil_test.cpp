#include "base/fileutil.h"
#include <iostream>
using namespace std;
using namespace server;

void testListdir() 
{
    string path = "../";
    vector<string> files = FileUtil::listdir(path);
    for (const string& file : files) {
        cout<< file <<" isdir: "<<FileUtil::isdir(path + file)<<endl;
    }

    path = FileUtil::getcwd();
    files = FileUtil::listdir(path);
    for (const string& file : files) {
        cout<< file <<" isdir: "<<FileUtil::isdir(path + file)<<endl;
    }
}


int main()
{
    testListdir();
    return 0;
}