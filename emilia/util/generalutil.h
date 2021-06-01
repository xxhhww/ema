#ifndef _EMILIA_GENERALUTIL_H_
#define _EMILIA_GENERALUTIL_H_

#include <vector>
#include <string>

namespace emilia{
namespace util{

//获得函数调用栈，并将其以string类型返回，以供输出
void Backtrace(std::vector<std::string>& bt, int size, int skip);
std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");

struct StrcaseIgnore
{
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

}
}

#endif