#ifndef _EMILIA_HTTP_H_
#define _EMILIA_HTTP_H_

#include "emilia/log/logmarco.h"
#include "emilia/util/generalutil.h"

#include <string>
#include <memory>
#include <map>

namespace emilia{
namespace http{

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \



enum class HttpMethod   
//枚举类 访问成员时必须加上HttpMethod这个域名
//宏的扩展(当时想了一会儿)用的挺好的
{
#define XX(num, name, string)   \
    name = num,                 \
                
    HTTP_METHOD_MAP(XX)

#undef XX
    INVALID_METHOD      //Http方法无效(不存在)
};

enum class HttpStatus
{
#define XX(num, name, string)   \
    name = num,                 \

    HTTP_STATUS_MAP(XX)

#undef XX
    INVALID_STATUS
};

HttpMethod StringToHttpMethod(const std::string str);
HttpMethod CharToHttpMethod(const char* chars);

HttpStatus StringToHttpStatus(const std::string str);
HttpStatus CharToHttpStatus(const char* chars);

const char* HttpMethodToString(const HttpMethod method);
const char* HttpStatusToString(const HttpStatus status);


/*
HTTP请求信息由3部分组成：
（1）请求方法URI协议/版本
（2）　请求头(Request Header)
（3）　请求正文(Request Body)【可选部分，例如：GET没有，POST有】

PS: GET方法的参数放在url中,而POST方法的参数放在body中

GET参数举例：
    GET /test/demo_form.asp?name1=value1&name2=value2#far HTTP/1.1
    其中？后面的是query string #后面的是fragment


举例：
POST 　/index.php　HTTP/1.1(\r\n)     请求行
Host: localhost(\r\n)                                                                        请求头
User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:10.0.2) Gecko/20100101 Firefox/10.0.2(\r\n) 　   
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,/;q=0.8(\r\n) 
Accept-Language: zh-cn,zh;q=0.5(\r\n) 
Accept-Encoding: gzip, deflate(\r\n) 
Connection: keep-alive(\r\n) 
Referer: http://localhost/(\r\n) 
Content-Length：25(\r\n) 
Content-Type：application/x-www-form-urlencoded(\r\n)                                          请求头
　　空行(\r\n)
username=aa&password=1234　　请求数据


*/
class HttpRequest
{
public:
    using ptr = std::shared_ptr<HttpRequest>;
    using HttpMap = std::map<std::string, std::string, emilia::util::StrcaseIgnore>;

    HttpRequest();
    ~HttpRequest(){}

//get方法
    HttpMethod getMethod() const { return m_method; }
    const std::string& getUrl() const { return m_url; }
    uint8_t getVersion() const { return m_version; }

    const std::string& getPath() const { return m_path; }
    const std::string& getQuery() const { return m_query; }
    const std::string& getFragment() const { return m_fragment; }

    const HttpMap& getHeaders() const { return m_headers; }
    bool isclose() { return m_close; }

    const std::string& getBody() const { return m_body; }

    const HttpMap& getParameters() const { return m_parameters; }

//set方法
    void setMethod(HttpMethod method) { m_method = method; }
    void setUrl(const std::string& url) { m_url = url;  }
    void setVersion(uint8_t version) { m_version = version; }

    void setPath(const std::string& path) { m_path = path; }
    void setQuery(const std::string& query) { m_query = query; }
    void setFragment(const std::string& fragment) { m_fragment = fragment; }

    void setHeaders(HttpMap& headers) { m_headers = headers; }
    void setClose(bool close) { m_close = close; }

    void setBody(const std::string& body) { m_body = body; }

    void setParameters(HttpMap& parameters) { m_parameters = parameters; }

//其他
    const std::string getHeader(const std::string& key) const;
    //按key查找，返回与key对应的value

    void setHeader(const std::string& key, const std::string& value);
    //按key查找，将key对应的value变成此函数指定的value

    const std::string getParameter(const std::string& key) const;
    void setParameter(const std::string& key, const std::string& value);
    //同getHeader 和 setHeader

    std::string toString() const;
    //将httprequest转化为string输出

    void init();        //初始化工作
    void initUrl();     //将url拆分成path，query，fragment
    void initQuery();   //将query的参数解析出来，并存放到m_parameters中
    void initBody();    //将body的参数解析出来，并存放到m_parameters中

private:
    HttpMethod m_method;    //http方法
    std::string m_url;      //请求路径[不是url，而是url去掉query和fragment之后的字符串]
    uint8_t m_version;      //http版本号(0x10 == HTTP/1.0 0x11 == HTTP/1.1)

    HttpMap m_headers;      //请求头
    bool m_close;           //是否关闭(close 或者 keep-alive)

    std::string m_body;     //请求数据

    std::string m_path;     //从url中解析出来的path字符串
    std::string m_query;    //从url中解析出来的query字符串
    std::string m_fragment; //从url中解析出来的fragment字符串

    HttpMap m_parameters;   //从 query(GET) 或 body(POST) 中解析出来的参数
};

class HttpResponse
{
public:
    using ptr = std::shared_ptr<HttpResponse>;
    using HttpMap = std::map<std::string, std::string, emilia::util::StrcaseIgnore>;

    HttpResponse(bool isClose);

//get方法
    uint8_t getVersion() const { return m_version; }
    HttpStatus getStatus() const { return m_status; }
    const std::string& getDescribe() const { return m_describe; }

    const HttpMap& getHeaders() const { return m_headers; }
    const std::string& getBody() const { return m_body; }

//set方法
    void setVersion(uint8_t version) { m_version = version; }
    void setStatus(HttpStatus status) { m_status = status; }
    void setDescribe(const std::string& describe) { m_describe = describe; }

    void setHeaders(const HttpMap& headers) { m_headers = headers; }
    void setBody(const std::string& body) { m_body = body; }

//其他方法
    const std::string getHeader(const std::string& key);
    void setHeader(const std::string& key, const std::string& value);

    std::string toString() const;
    //转化成string输出

private:
    uint8_t m_version;      //版本(0x11 == HTTP/1.1 0x10 == HTTP/1.0)
    HttpStatus m_status;    //200
    std::string m_describe;   //OK

    HttpMap m_headers;      //响应头部字段

    bool m_close;           //是否支持长连接

    std::string m_body;     //响应体
};

}
}

#endif
