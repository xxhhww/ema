#ifndef _EMILIA_HTTP_PARSER_H_
#define _EMILIA_HTTP_PARSER_H_

#include "http.h"

namespace emilia{
namespace http{

enum class LineStatus
//行状态
{
    LINE_INCOMPLETE = 0,    //行不完整
    LINE_COMPLETE = 1,      //行完整
    LINE_BAD = 2,           //行出错
};

enum class RequestStatus
//请求状态
{
    REQUEST_INCOMPLETE = 0, //请求不完整
    REQUEST_OK = 1,         //请求正确
    REQUEST_BAD = 2,        //请求出错
};

enum class ParseStatus
//解析状态(接下来要解析的对象)
{
    PARSE_LINE = 0, //接下来要解析请求行
    PARSE_HEAD = 1, //接下来解析头部字段
    PARSE_BODY = 2, //接下来解析body段
};

class HttpRequestParser
//Http请求报文的解析器
//解析的结果将存放在 m_result中
{
public:
    using ptr = std::shared_ptr<HttpRequestParser>;

    HttpRequestParser();

    RequestStatus execute(char* data, size_t length);
    //执行函数，进行解析(它直接返回请求的状态而不是行状态，因为它的处理对象是整个请求)
    //length是data的大小

    HttpRequest::ptr getResult() const { return m_result; }

private:
    LineStatus checkLineStatus(char* data, size_t length);
    //检查行状态(它直接返回行状态，因为它的处理对象是整个行)

    RequestStatus parseRequestLine(char* data);
    //解析请求行

    RequestStatus parseHeaders(char* data);
    //解析头部字段

    void parseBody(char* data, int length);
    //获得body字段(不返回，body不做检查直接f复制)

private:
    HttpRequest::ptr m_result;  //解析结果

    int m_checkIndex;
    int m_startIndex;
    //checkIndex用于检查行是否完整
    //startIndex用于标识当前要解析的行(数据)在原始数组(char*)中的开端

    RequestStatus m_requestStatus;  //请求状态
    ParseStatus m_parseStatus;      //解析状态
};





}
}

#endif
