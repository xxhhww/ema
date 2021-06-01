#ifndef _EMILIA_CODEUTIL_H_
#define _EMILIA_CODEUTIL_H_

#include <string>

namespace emilia{
namespace util{

std::string SHA1(const std::string& data);

std::string base64encode(const std::string& data);
std::string base64encode(const void* data, size_t length);

}
}

#endif