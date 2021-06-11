#include "fileutil.h"
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "logging.h"
#include <sys/stat.h>

using namespace server;

std::string FileUtil::readFile(const std::string& filename)
{
    FILE* fp = ::fopen(filename.c_str(), "rb");
    std::string res;
    if (fp) {
        char buf[1024*1024];
        size_t nread = 0;
        while((nread = ::fread(buf, 1, sizeof buf, fp)) > 0) {
            res.append(buf, nread);
        }
        ::fclose(fp);
    }
    return res;
}

std::vector<std::string> FileUtil::listdir(const std::string& path)
{
    std::vector<std::string> fileList;
    DIR *dirptr = ::opendir(path.c_str());
    if (!dirptr) {
        LOG_ERROR << "FileUtil::listdir() cannot open: " << path;
        return fileList;
    }
    struct dirent* fileptr;
    while(NULL != (fileptr = ::readdir(dirptr))) {
        if (::strcmp(".", fileptr->d_name) == 0 || ::strcmp("..", fileptr->d_name) == 0) {
            continue;
        }
        if (isdir(path + "/" + fileptr->d_name)) {
            fileList.push_back(fileptr->d_name);
            fileList.back() += "/";
        } else {
            fileList.push_back(fileptr->d_name);
        }
        
    }
    ::closedir(dirptr);
    return fileList;
}

bool FileUtil::isdir(const std::string& path)
{
    struct stat statbuf;
    ::stat(path.c_str(), &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
        return true;
    }
    return false;
}

std::string FileUtil::getcwd()
{
    char path[256];
    if (::getcwd(path, sizeof path)) {
        return path;
    }
    return "";
}

FileUtil::FileType FileUtil::getType(const std::string& path)
{
    struct stat statbuf;
    ::stat(path.c_str(), &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
        return T_DIR;
    } else if (S_ISREG(statbuf.st_mode)) {
        return T_FILE;
    }
    return UNKNOWN;
}
