#include "http.h"
#include "emilia/util/stringutil.h"

#include <sstream>
#include <string.h>

namespace emilia
{
namespace http
{

//执行效率不高(遍历查找)
HttpMethod StringToHttpMethod(const std::string str)
{
#define XX(num, name, string)               \
    if(strcmp(#string, str.c_str()) == 0)   \
        return HttpMethod::name;            \

    HTTP_METHOD_MAP(XX);

#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpMethod CharToHttpMethod(const char* chars)
{
#define XX(num, name, string)               \
    if(strcmp(#string, chars) == 0)         \
        return HttpMethod::name;            \
    
    HTTP_METHOD_MAP(XX);

#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpStatus StringToHttpStatus(const std::string str)
{
#define XX(num, name, string)               \
    if(strcmp(#string, str.c_str()) == 0)   \
        return HttpStatus::name;            \
    
    HTTP_STATUS_MAP(XX);

#undef XX
    return HttpStatus::INVALID_STATUS;
}

HttpStatus CharToHttpStatus(const char* chars)
{
#define XX(num, name, string)               \
    if(strcmp(#string, chars) == 0)         \
        return HttpStatus::name;            \
    
    HTTP_STATUS_MAP(XX);

#undef XX
    return HttpStatus::INVALID_STATUS;
}

const char* HttpMethodToString(const HttpMethod method)
{
    switch (method)
    {

#define XX(num, name, string)               \
    case HttpMethod::name:                  \
        return #string;                     \

    HTTP_METHOD_MAP(XX);

#undef XX
    default:
        return "INVALID_METHOD";
    }
}

const char* HttpStatusToString(const HttpStatus status)
{
    switch (status)
    {
#define XX(num, name, string)               \
    case HttpStatus::name:                  \
        return #string;                     \

    HTTP_STATUS_MAP(XX);

#undef XX    
    default:
        return "INVALID_STATUS";
    }
}

HttpRequest::HttpRequest()
:m_method(HttpMethod::INVALID_METHOD)
,m_url("/")
,m_version(0x11)
,m_close(true)
{}

const std::string HttpRequest::getHeader(const std::string& key) const
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? "" : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

const std::string HttpRequest::getParameter(const std::string& key) const
{
    auto it = m_headers.find(key);
    return it == m_parameters.end() ? "" : it->second;
}

void HttpRequest::setParameter(const std::string& key, const std::string& value)
{
    m_parameters[key] = value;
}

std::string HttpRequest::toString() const
{
    std::stringstream ss;
    ss  << HttpMethodToString(m_method) << " "
        << m_url
        << (m_query.empty() ? "" : "?")
        << m_query
        << (m_fragment.empty() ? "" : "#")
        << m_fragment
        << " HTTP/"
        << ((uint32_t)(m_version >> 4))
        << "."
        << ((uint32_t)(m_version & 0x0F))
        << "\r\n";

    for(auto& i : m_headers)
    {
        ss << i.first << ": " << i.second << "\r\n";
    }

    if(!m_body.empty())
    {
        ss << "\r\n"
           << m_body;
    }
    else
    {
        ss << "\r\n";
    }

    return ss.str();
}

void HttpRequest::init()
{
    initUrl();
    initQuery();
    initBody();
}

void HttpRequest::initUrl()
{
    size_t pos1 = m_url.find_first_of('?');
    size_t pos2 = m_url.find_first_of('#');

    if(pos1 == std::string::npos && pos2 == std::string::npos){
        m_path = m_url;
        m_query = "";
        m_fragment = "";
    }
    else if( pos1 != std::string::npos && pos2 == std::string::npos ){
        m_path = m_url.substr(0, pos1);
        m_query = m_url.substr(pos1+1);
        m_fragment = "";
    }
    else if( pos1 == std::string::npos && pos2 != std::string::npos ){
        m_path = m_url.substr(0, pos2);
        m_query = "";
        m_fragment = m_url.substr(pos2+1);
    }
    else{
        m_path = m_url.substr(0, pos1);
        m_query = m_url.substr(pos1+1, pos2 - pos1-1);
        m_fragment = m_url.substr(pos2+1);
    }
}

#define PARSE_PARAM(str, m, flag, trim) \
    size_t pos = 0; \
    do { \
        size_t last = pos; \
        pos = str.find('=', pos); \
        if(pos == std::string::npos) { \
            break; \
        } \
        size_t key = pos; \
        pos = str.find(flag, pos); \
        m.insert(std::make_pair(trim(str.substr(last, key - last)), \
                    emilia::util::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1)))); \
        if(pos == std::string::npos) { \
            break; \
        } \
        ++pos; \
    } while(true);  \

void HttpRequest::initQuery()
{
    //如果query为空
    if(m_query.empty())
        return;
    PARSE_PARAM(m_query, m_parameters, '&',);
}
void HttpRequest::initBody()
{
    //如果Body为空
    if(m_body.empty())
        return;
    std::string content_type = getHeader("content-type");
    if(strcasecmp(content_type.c_str(), "application/x-www-form-urlencoded") != 0 )
    {
        EMILIA_LOG_ERROR("system") << "Content-type Unsupport";
        return;    
    }
    PARSE_PARAM(m_body, m_parameters, '&',);
}
//==================================================================
HttpResponse::HttpResponse(bool isClose)
:m_version(0x11)
,m_status(HttpStatus::OK)
,m_describe("OK")
,m_close(isClose)
{}

const std::string HttpResponse::getHeader(const std::string& key)
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? "" : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

std::string HttpResponse::toString() const
{
    std::stringstream ss;
    ss  << "HTTP/"
        << ((uint32_t)(m_version >> 4))
        << "."
        << ((uint32_t)(m_version & 0x0F))
        << " "
        << (uint32_t)m_status
        << " "
        << (m_describe.empty() ? HttpStatusToString(m_status) : m_describe)
        << "\r\n";

    for(auto& i : m_headers)
    {
        ss << i.first << ": " << i.second << "\r\n";
    }

    if(m_close) //不支持长连接
    {
        ss << "Connection: close" << "\r\n";
    }
    else
        ss << "Connection: keep-alive" << "\r\n";

    

    if(!m_body.empty())
    {
        ss << "Content-length: " << m_body.size() << "\r\n";
        ss << "\r\n"
           << m_body;
    }
    else
    {
        ss << "\r\n";
    }

    return ss.str();
}
    


}
}
