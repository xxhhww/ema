#include "http_parser.h"
#include <cstring>

namespace emilia{
namespace http
{
HttpRequestParser::HttpRequestParser()
:m_result(new HttpRequest())
,m_checkIndex(0)
,m_startIndex(0)
,m_requestStatus(RequestStatus::REQUEST_OK)
,m_parseStatus(ParseStatus::PARSE_LINE)
{}

RequestStatus HttpRequestParser::execute(char* data, size_t length)
{

    LineStatus LStatus_ = LineStatus::LINE_COMPLETE;
    while( (LStatus_ = checkLineStatus(data, length)) == LineStatus::LINE_COMPLETE ) //解析出完整的行
    {
        char* temp = data + m_startIndex;   //当前要解析的行

        m_startIndex = m_checkIndex;       //指向下一次要解析的行在数据中的位置

        switch (m_parseStatus)
        {
        case ParseStatus::PARSE_LINE:
            m_requestStatus = parseRequestLine(temp);
            if( m_requestStatus == RequestStatus::REQUEST_BAD ) //请求行有问题，说明是错误的请求，直接返回
                return m_requestStatus;
            else
                m_parseStatus = ParseStatus::PARSE_HEAD;        //行解析完成，接下来解析头部字段
            break;
        
        case ParseStatus::PARSE_HEAD:
            m_requestStatus = parseHeaders(temp);          
            //如果temp开头就是\r\n，会在函数内部直接返回，并设置ParseStatus为PARSE_BODY
            if( m_requestStatus == RequestStatus::REQUEST_BAD )
                return m_requestStatus;
            if( m_parseStatus == ParseStatus::PARSE_BODY )
                goto Label;
            break;
        default:
            return RequestStatus::REQUEST_BAD;
        }
    }

    if( LStatus_ == LineStatus::LINE_INCOMPLETE )   //如果行不完整，返回请求不完整
        return RequestStatus::REQUEST_INCOMPLETE;
    else if( LStatus_ == LineStatus::LINE_BAD )
        return RequestStatus::REQUEST_BAD;
    else
    {
Label:
        if(m_result->getHeader("Connection") == "keep-alive")   //长连接
            m_result->setClose(false);
        else
            m_result->setClose(true);   //短连接

        std::string content_length = m_result->getHeader("Content-Length");
        if( content_length != "")
        {
            char* temp = data + m_startIndex;
            parseBody(temp, atoi(content_length.c_str()));
        }
        return RequestStatus::REQUEST_OK;
    }
}

LineStatus HttpRequestParser::checkLineStatus(char* data, size_t length)
{
    //m_checkIndex从0开始
    //length从1开始
    char temp = data[m_checkIndex];
    

    while( m_checkIndex != (int)length )
    {
        if( temp == '\r' )  //可能是完整的行
        {
            if( (m_checkIndex + 1) == (int)length ) //说明\r是最后一个字符，此行不完整(缺一个'\n')
                return LineStatus::LINE_INCOMPLETE;
            else
            {
                temp = data[m_checkIndex+1];
                if( temp == '\n' )
                {
                    data[m_checkIndex++] = '\0';    //方便截取
                    data[m_checkIndex++] = '\0';    //m_checkIndex++后指向下一行的开头
                    //\r和\n都被置成了\0，方便截取行

                    return LineStatus::LINE_COMPLETE;
                }
                else
                    return LineStatus::LINE_BAD;
            }
        }
        m_checkIndex ++;
        temp = data[m_checkIndex];
    }

    return LineStatus::LINE_INCOMPLETE;
}

RequestStatus HttpRequestParser::parseRequestLine(char* data)
{
    int readIndex = 0;
    char temp = data[readIndex];
    while( temp != '\0' )
    {
        if( temp == ' ' )
        {
            m_result->setMethod(StringToHttpMethod(std::string(data, readIndex)));
            readIndex++;
            break;
        }
        readIndex++;
        temp = data[readIndex];
    }

    char* next = data + readIndex;
    readIndex = 0;
    temp = next[readIndex];
    while( temp != '\0' )
    {
        if( temp == ' ' )
        {
            m_result->setUrl(std::string(next, readIndex));
            readIndex++;
            break;
        }
        readIndex++;
        temp = next[readIndex];
    }

    char* version = next + readIndex;
    if( strcasecmp(version, "HTTP/1.1") == 0 )
    {
        m_result->setVersion(0x11);
        return RequestStatus::REQUEST_INCOMPLETE;
    }
    else if( strcasecmp(version, "HTTP/1.0") == 0 )
    {
        m_result->setVersion(0x10);
        return RequestStatus::REQUEST_INCOMPLETE;
    }
    else
        return RequestStatus::REQUEST_BAD;
}

RequestStatus HttpRequestParser::parseHeaders(char* data)
{
    if( data[0] == '\0' )
    {
        m_parseStatus = ParseStatus::PARSE_BODY;
        return RequestStatus::REQUEST_INCOMPLETE;
    }

    int readIndex = 0;
    char temp = data[readIndex];

    while( temp != ':')
    {
        readIndex++;
        temp = data[readIndex];
    }

    char* value = data + readIndex;
    value ++;   //跳过':'
    value ++;   //跳过' '

    m_result->setHeader(std::string(data, readIndex), value);
    return RequestStatus::REQUEST_INCOMPLETE;
}
 
void HttpRequestParser::parseBody(char* data, int length)
{
    EMILIA_LOG_INFO("system") << data << " " << length; 
    m_result->setBody(std::string(data, length));
}


}
}
