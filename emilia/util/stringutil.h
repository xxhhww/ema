#ifndef _EMILIA_StringUTIL_H_
#define _EMILIA_StringUTIL_H_

#include <stdint.h>
#include <vector>
#include <string>

namespace emilia{
namespace util{

//类型转换
class StringUtil{
public:
    static int8_t   ToInt8(const char* str);
    static uint8_t  ToUint8(const char* str);
    static int16_t  ToInt16(const char* str);
    static uint16_t ToUint16(const char* str);
    static int32_t  ToInt32(const char* str);
    static uint32_t ToUint32(const char* str);
    static int64_t  ToInt64(const char* str);
    static uint64_t ToUint64(const char* str);

    static float ToFloat(const char* str);
    static double ToDouble(const char* str);

    //按分隔符将str分成对应的vector<string>
    static std::vector<std::string> Split(const char* str, char separator);
    //将字符old全部替换为new
    static std::string ReplaceChar(const std::string& str, char oldChar, char newChar);
    //将字符old消除
    static std::string RemoveChar(const std::string& str, char oldChar);
    //将下标为index的字符大写
    static const char* UpperLetter(const std::string& str, size_t index = 0);
    //将字符串str整个大写
    static std::string UpperAll(const std::string& str); 

    //输入 输出
    //user_info UserInfo
    //user_info_dao UserInfoDao
    static std::string GetClassName(const std::string& str);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);
};

}
}

#endif