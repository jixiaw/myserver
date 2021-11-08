#ifndef SERVER_BASE_FILEUTIL_H
#define SERVER_BASE_FILEUTIL_H
#include <string>
#include <vector>
namespace server 
{

class FileUtil
{
public:
    enum FileType {
        T_FILE,
        T_DIR,
        UNKNOWN,
    };
    static std::string readFile(const std::string& filename);
    static std::vector<std::string> listdir(const std::string& path);
    static bool isdir(const std::string& path);
    static FileType getType(const std::string& path);
    static std::string getcwd();
    static std::string getExt(const std::string& path);
};
}

#endif