#include "emilia/log/logformat.h"
#include "emilia/log/logstream.h"

#include <vector>
#include <tuple>
#include <iostream>

namespace emilia{
namespace log{

//解析pattern
LogFormat::LogFormat(const std::string pattern)
:m_pattern(pattern)
,m_error(false){
    /*
    %m 消息
    %p 日志级别
    %r 累计毫秒数
    %c 日志名称
    %t 线程id
    %n 换行
    %d 时间
    %f 文件名
    %l 行号
    %T 制表符
    %F 协程id
    %N 线程名称

    默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    */
   //!str、fmt、type
   //!str：字符或者字符串
   //!fmt：格式 例如日期格式{%Y-%m-%d %H:%M:%S}
   //!type：类型 两种类型(0是普通字符串或者普通字符，1是特殊字符m、p、r、c)
   //!Ps：如果m、p、r、c等特殊字符前面没有加%，那么会认为是普通字符
   //!Ps：如果要输出%字符，那么要使用%%，就是两个百分号

   std::vector<std::tuple<std::string, std::string, int> > vec;
   //初始态处于解析普通字符
   ParseStatus status = ParseStatus::NORMAL;
   //用于记录普通字符串
   std::string normalStr;
   //用于记录特殊字符
   std::string specialStr;
   //用于记录日志输出格式
   std::string formatStr;
   for(size_t i = 0; i < m_pattern.size(); i++){
       //当前处于解析普通字符状态
       if(status == ParseStatus::NORMAL){
           //如果当前字符是%，那么需要判断它是否是转义字符
           if(m_pattern[i] == '%'){
               //是转义字符，那么将%当作普通字符追加到normalStr中
               if(m_pattern[i+1] == '%'){
                   normalStr.append(1, '%');
                   //当前指标是指向%%中第一个%，而现在要将两个%%都跳过
                   i++;
                   continue;
               }
               //不是转义字符，则记录下当前的普通字符串，然后状态切入解析特殊字符的状态
               else{
                   //记录当前普通字符串
                   vec.push_back(std::tuple<std::string, std::string, int>(normalStr, std::string(), 0));
                   normalStr.clear();
                   status = ParseStatus::SPECIAL;
                   continue;
               }
           }
           //不是%，那就是其他普通字符
           else{
               normalStr.append(1, m_pattern[i]);
               continue;
           }
       }
       //当前处于解析特殊字符状态
       else if(status == ParseStatus::SPECIAL){
           //记录当前的特殊字符
           specialStr.append(1, m_pattern[i]);
           //如果下一个字符是{，那么就开始解析此特殊字符的输出格式
           if(m_pattern[i+1] == '{'){
               status = ParseStatus::FORMAT;
               //跳过{字符
               i++;
               continue;
           }
           //如果下一个字符不是{，那么就记录下当前的特殊字符，然后返回解析普通字符的状态
           else{
               vec.push_back(std::tuple<std::string, std::string, int>(specialStr, std::string(), 1));
               specialStr.clear();
               status = ParseStatus::NORMAL;
               continue;
           }
       }
       //当前处于解析日志输出格式状态
       else{
           //一直解析，直到碰到}符号，则重新转入解析普通字符状态
           if(m_pattern[i] != '}'){
               //如果当前字符已经是最后一个字符了，但是还是没有遇到}符号则报错
               if(i == m_pattern.size()-1){
                   std::cout << "Parse Error: } Not Exist" << std::endl;
                   m_error = true;
                   break;
               }
               else{
                   formatStr.append(1, m_pattern[i]);
                   continue;
               }
           }
           else{
               vec.push_back(std::tuple<std::string, std::string, int>(specialStr, formatStr, 1));
               specialStr.clear();
               formatStr.clear();
               status = ParseStatus::NORMAL;
               continue;
           }
       }
   }
   //如果普通字符串是处于日志格式的最后
   //!例如：%m[Test]
   //!此时[Test]是普通字符串并且处于日志输出格式的末尾
   //!需要对normalStr进行判断
   if(!normalStr.empty()){
       vec.push_back(std::tuple<std::string, std::string, int>(normalStr, std::string(), 0));
       normalStr.clear();
   }

   static FillFormatItemMap FillFormatItemMap_;

   //!接下来就是根据刚才解析日志格式获得的vec，从FormatItemMap中获得对应的生成FormatItem的函数，然后将此函数返回的FormatItem存入m_items中
   for(auto& i : vec){
       //!如果是普通字符串，那么生成对应的StringFormatItem存入m_items
       if(std::get<2>(i) == 0){
           m_items.push_back(StringFormatItem::ptr(new StringFormatItem(std::get<0>(i))));
       }
       //剩下都是特殊字符
       else{
           auto it = FormatItemMap::GetInstance()->find(std::get<0>(i));
           if(it == FormatItemMap::GetInstance()->end()){
               std::cout << "Special Char: "<< std::get<0>(i) << "Not Exist" << std::endl;
               m_error = true;
               break;
           }
           else{
               m_items.push_back(it->second(std::get<1>(i)));
           }
       }
   }
}

void LogFormat::outPut(std::ostream& os, LogStream::ptr stream){
    //调用输出格式实体的outPut()方法
    for(auto& i : m_items){
        i->outPut(os, stream);
    }
}

}
}