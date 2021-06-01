#ifndef _EMILIA_LOGFORMAT_H_
#define _EMILIA_LOGFORMAT_H_

#include "emilia/singleton.h"
#include "emilia/log/logstream.h"
#include <iostream>

#include <memory>
#include <map>
#include <vector>
#include <string>

namespace emilia{
namespace log{

//解析状态
enum class ParseStatus{
    //当前正在解析普通字符
    NORMAL = 0x00,
    //当前正在解析特殊字符
    SPECIAL = 0x01,
    //当前正在解析特殊字符对应的输出格式(日期格式)
    FORMAT = 0x02
};

class FormatItem{
public:
    using ptr = std::shared_ptr<FormatItem>;
    //虚析构
    virtual ~FormatItem() {}
    virtual void outPut(std::ostream& os, LogStream::ptr stream) = 0;
};

//日志格式解析器
class LogFormat{
public:
    using ptr = std::shared_ptr<LogFormat>;
    LogFormat(const std::string pattern);

    void outPut(std::ostream& os, LogStream::ptr stream);
private:
    //日志格式
    std::string m_pattern;
    //解析是否出错
    bool m_error;
    //日志格式输出实体
    std::vector<FormatItem::ptr> m_items;
};

//输出日志的内容
class MessageFormatItem : public FormatItem{
public:
    MessageFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getContent();
    }
};

//输出日志的等级
class LevelFormatItem : public FormatItem{
public:
    LevelFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << LevelToString(stream->getLevel());
    }
};

//输出程序启动到现在的时间
class ElapseFormatItem : public FormatItem
{
public:
    ElapseFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getElapse();
    }
};


//输出日志器名称
class NameFormatItem : public FormatItem
{
public:
    NameFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getLoggerName();
    }
};

//输出线程id
class ThreadIdFormatItem : public FormatItem
{
public:
    ThreadIdFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getThreadId();
    }
};

//输出线程的名称
class ThreadNameFormatItem : public FormatItem
{
public:
    ThreadNameFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getThreadName();
    }
};

//输出日志产生的时间
class DateTimeFormatItem : public FormatItem
{
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S"):m_format(format)
    {
        if(m_format.empty()){
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        //更改时间格式
        struct tm tm;
        time_t time = stream->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;   //时间的格式
};

//输出文件名
class FilenameFormatItem : public FormatItem
{
public:
    FilenameFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getFile();
    }
};

//输出行数
class LineFormatItem : public FormatItem
{
public:
    LineFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << stream->getLine();
    }
};

//输出普通字符
class StringFormatItem : public FormatItem
{
public:
    StringFormatItem(const std::string& str):m_string(str)
    {}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << m_string;
    }
private:
    std::string m_string;
};

//输出换行符号 
class NewLineFormatItem : public FormatItem                                                 
{
public:
    NewLineFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << std::endl;
    }
};

//输出制表符
class TabFormatItem : public FormatItem
{
public:
    TabFormatItem(const std::string& str = ""){}
    void outPut(std::ostream& os, LogStream::ptr stream) override{
        os << '\t';
    }
};

//接收一个str参数用于初始化FormatItem，返回一个FormatItem
using GetFormatItem = std::function<FormatItem::ptr(const std::string& str)>;

using FormatItemMap = singleton<std::map<std::string, GetFormatItem> >;


struct FillFormatItemMap{
    FillFormatItemMap(){
#define XX(str, type) \
    FormatItemMap::GetInstance()->insert(std::pair<std::string, GetFormatItem>(#str \
                                        ,[](const std::string& fmt){ return FormatItem::ptr(new type(fmt)); })); \

    XX(m, MessageFormatItem)
    XX(p, LevelFormatItem)
    XX(r, ElapseFormatItem)
    XX(c, NameFormatItem)
    XX(t, ThreadIdFormatItem)
    XX(n, NewLineFormatItem)
    XX(d, DateTimeFormatItem)
    XX(f, FilenameFormatItem)
    XX(l, LineFormatItem)
    XX(T, TabFormatItem)
    XX(N, ThreadNameFormatItem)

#undef XX
    }
};

}
}

#endif
