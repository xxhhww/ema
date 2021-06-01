#include "emilia/util/stringutil.h"
#include <iostream>

int main(){
    const char* test = "-111111111111";

    std::cout << (int32_t)emilia::util::StringUtil::ToUint8(test) << std::endl;
    std::cout << (int32_t)emilia::util::StringUtil::ToInt8(test) << std::endl;

    std::cout << emilia::util::StringUtil::ToUint16(test) << std::endl;
    std::cout << emilia::util::StringUtil::ToInt16(test) << std::endl;

    std::cout << emilia::util::StringUtil::ToUint32(test) << std::endl;
    std::cout << emilia::util::StringUtil::ToInt32(test) << std::endl;

    std::cout << emilia::util::StringUtil::ToUint64(test) << std::endl;
    std::cout << emilia::util::StringUtil::ToInt64(test) << std::endl;

    const char* test2 = "-13.33";
    std::cout << emilia::util::StringUtil::ToFloat(test2) << std::endl;
    std::cout << emilia::util::StringUtil::ToDouble(test2) << std::endl;

    const char* test3 = "wuhu,333,444,555,666,777,888";
    std::vector<std::string> rt = emilia::util::StringUtil::Split(test3, ',');
    for(auto& i : rt){
        std::cout << i << std::endl;
    }

    std::string test4 = "111,222,333,444,555,666,777,888,999";
    test4 = emilia::util::StringUtil::ReplaceChar(test4, ',', 'K');
    std::cout << test4 << std::endl;

    test4 = emilia::util::StringUtil::RemoveChar(test4, 'K');
    std::cout << test4 << std::endl;

    std::string test5 = "idzzzzzzzzzzz";
    std::cout << emilia::util::StringUtil::UpperLetter(test5) << std::endl;
    std::cout << emilia::util::StringUtil::UpperAll(test5) << std::endl;

    std::string className = "user_info_dao";
    std::string className1 = "User_InFo";

    std::cout << emilia::util::StringUtil::GetClassName(className) << std::endl;
    std::cout << emilia::util::StringUtil::GetClassName(className1) << std::endl;

    return 0;
}