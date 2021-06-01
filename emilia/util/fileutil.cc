#include "emilia/util/fileutil.h"
#include <unistd.h>

namespace emilia{
namespace util{

//生成对应目录
bool FileUtil::MKDir(const std::string& path){
    //循环判断父目录是否存在，如不存在，则创建父目录
    //例：/home/test/example
    //先检查/home，其次/home/test/，最后/home/test/example

    //从根目录后开始检查
    size_t pos = 1;

    while(pos != std::string::npos){
        size_t pos = path.find('/', pos);
        //当前检查的目录
        std::string curPath = path.substr(0, pos);
    }
}
//生成对应文件
bool FileUtil::MKFile(const std::string& path){

}
//删除对应文件或者目录
bool FileUtil::RMAll(const std::string& path){

}
//移动文件或者目录
//from:源目录
//to:目标目录
bool FileUtil::MVAll(const std::string& from, const std::string& to){

}
//列出目标目录下的所有目标文件名加载至files中
//files:存放所有的目标文件
//path:检索目录
//suffix:文件后缀
void FileUtil::ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& suffix = nullptr){

}

}
}