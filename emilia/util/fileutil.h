#ifndef _EMILIA_FILEUTIL_H_
#define _EMILIA_FILEUTIL_H_

#include <string>
#include <vector>

namespace emilia{
namespace util{

class FileUtil{
public:
    //生成对应目录
    static bool MKDir(const std::string& path);
    //生成对应文件
    static bool MKFile(const std::string& path);

    //删除对应文件或者目录
    static bool RMAll(const std::string& path);
    //移动文件或者目录
    //from:源目录
    //to:目标目录
    static bool MVAll(const std::string& from, const std::string& to);

    //列出目标目录下的所有目标文件名加载至files中
    //files:存放所有的目标文件
    //path:检索目录
    //suffix:文件后缀
    static void ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& suffix = nullptr);
};

}
}
#endif