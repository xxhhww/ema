#include "stringutil.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

namespace emilia{
namespace util{

int8_t   StringUtil::ToInt8(const char* str)
{
    //int8的取值范围是 -128 ~ 127
    //如果str对应的值过大或者过小就取边界值
    int64_t tempVal = StringUtil::ToInt64(str);
    int8_t resVal = 0;
    if(tempVal > 127)
        resVal = 127;
    else if(tempVal < -128)
        resVal = -128;
    else
        resVal = (int8_t)tempVal;
    
    return resVal;
}

uint8_t  StringUtil::ToUint8(const char* str)
{
    int64_t tempVal = StringUtil::ToUint64(str);
    uint8_t resVal = 0;
    if(tempVal > 255 || tempVal < 0)
        resVal = 255;
    else
        resVal = (uint8_t)tempVal;
    
    return resVal;
}

int16_t  StringUtil::ToInt16(const char* str)
{
    int64_t tempVal = StringUtil::ToInt64(str);
    int16_t resVal = 0;
    if(tempVal > 32767)
        resVal = 32767;
    else if(tempVal < -32768)
        resVal = -32768;
    else
        resVal = (int16_t)tempVal;
    
    return resVal;
}

uint16_t StringUtil::ToUint16(const char* str)
{
    int64_t tempVal = StringUtil::ToUint64(str);
    uint16_t resVal = 0;
    if(tempVal > 65535 || tempVal < 0)
        resVal = 65535;
    else
        resVal = (uint16_t)tempVal;

    return resVal;
}

int32_t  StringUtil::ToInt32(const char* str)
{
    int64_t tempVal = StringUtil::ToInt64(str);
    int32_t resVal = 0;
    if(tempVal > 2147483647 )
        resVal = 2147483647 ;
    else if(tempVal < -2147483648 )
        resVal = -2147483648;
    else
        resVal = (int32_t)tempVal;
    
    return resVal;
}

uint32_t StringUtil::ToUint32(const char* str)
{
    int64_t tempVal = StringUtil::ToUint64(str);
    uint32_t resVal = 0;
    if(tempVal > 4294967295 || tempVal < 0)
        resVal = 4294967295;
    else
        resVal = (uint32_t)tempVal;

    return resVal;
}

int64_t  StringUtil::ToInt64(const char* str)
{
    if(strlen(str) == 0)
        return 0;
    else
        return strtoll(str, nullptr, 10);
}

uint64_t StringUtil::ToUint64(const char* str)
{
    if(strlen(str) == 0)
        return 0;
    else
        return strtoull(str, nullptr, 10);
}

float StringUtil::ToFloat(const char* str)
{
    if(strlen(str) == 0)
        return (float)0;
    else
        return strtof(str, nullptr);
}

double StringUtil::ToDouble(const char* str)
{
    if(strlen(str) == 0)
        return (double)0;
    else
        return strtod(str, nullptr);
}

std::vector<std::string> StringUtil::Split(const char* str, char separator)
{
    std::vector<std::string> rt;
    std::string source = str;
    size_t start = 0;
    size_t pos = source.find_first_of(separator);
    while(pos != std::string::npos){
        rt.push_back(source.substr(start, pos - start));
        start = pos + 1;
        pos = source.find_first_of(separator, start);
    }
    rt.push_back(source.substr(start));
    return rt;
}

std::string StringUtil::ReplaceChar(const std::string& str, char oldChar, char newChar)
{
    std::string rt = str;
    size_t pos = rt.find_first_of(oldChar);
    while(pos != std::string::npos){
        rt[pos] = newChar;
        pos = rt.find_first_of(oldChar, pos+1);
    }
    return rt;
}

std::string StringUtil::RemoveChar(const std::string& str, char oldChar)
{
    std::string rt = str;
    size_t pos = rt.find_first_of(oldChar);
    while(pos != std::string::npos){
        rt.erase(pos, 1);
        pos = rt.find_first_of(oldChar);
    }
    return rt;
}

const char* StringUtil::UpperLetter(const std::string& str, size_t index)
{
    std::string cppStr = str;
    size_t length = cppStr.size();

    if(index >= length)
        index = length - 1;
    if(index < 0)
        index = 0;
    
    //将index对应的字母升级为大写
    cppStr[index] = static_cast<char>(toupper(cppStr[index]));
    return cppStr.c_str();
}

std::string StringUtil::UpperAll(const std::string& str){
    std::string rt = str;
    size_t length = rt.size();
    for(size_t i = 0; i < length; i++){
        //如果rt[i]是字母
        if(isalpha(rt[i])){
            rt[i] = toupper(rt[i]);
        }
    }
    return rt;
}

std::string StringUtil::GetClassName(const std::string& str){
    if(str.empty())
        return "";
    std::string rt = str;
    rt[0] = toupper(rt[0]);

    size_t nextUp = rt.find_first_of("_");
    size_t idx = 1;
    size_t strSize = str.size();
    //T_t
    while(idx != strSize){
        if(idx != (nextUp+1)){
            if(isalpha(rt[idx]))
                rt[idx] = tolower(rt[idx]);
        }
        else{
            rt[idx] = toupper(rt[idx]);
            nextUp = rt.find_first_of("_", nextUp+1);
        }
        idx ++;
    }

    rt = RemoveChar(rt, '_');
    return rt;
}

static const char uri_chars[256] = {
    /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
    /* 64 */
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char xdigit_chars[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define CHAR_IS_UNRESERVED(c)           \
    (uri_chars[(unsigned char)(c)])

std::string StringUtil::UrlEncode(const std::string& str, bool space_as_plus) {
    static const char *hexdigits = "0123456789ABCDEF";
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for(const char* c = str.c_str() ; c < end; ++c) {
        if(!CHAR_IS_UNRESERVED(*c)) {
            if(!ss) {
                ss = new std::string;
                ss->reserve(str.size() * 1.2);
                ss->append(str.c_str(), c - str.c_str());
            }
            if(*c == ' ' && space_as_plus) {
                ss->append(1, '+');
            } else {
                ss->append(1, '%');
                ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                ss->append(1, hexdigits[*c & 0xf]);
            }
        } else if(ss) {
            ss->append(1, *c);
        }
    }
    if(!ss) {
        return str;
    } else {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus) {
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for(const char* c = str.c_str(); c < end; ++c) {
        if(*c == '+' && space_as_plus) {
            if(!ss) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, ' ');
        } else if(*c == '%' && (c + 2) < end
                    && isxdigit(*(c + 1)) && isxdigit(*(c + 2))){
            if(!ss) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
            c += 2;
        } else if(ss) {
            ss->append(1, *c);
        }
    }
    if(!ss) {
        return str;
    } else {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

}
}