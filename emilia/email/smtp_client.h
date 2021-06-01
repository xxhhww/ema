#ifndef _EMILIA_SMTP_CLIENT_H_
#define _EMILIA_SMTP_CLIENT_H_

#include "emilia/net/socketstream.h"
#include "emilia/email/email.h"

using namespace emilia::net;

namespace emilia{
namespace email{

struct SmtpResult
//用于存储如 
//250 OK
//550 No such user here
//结果码 描述信息
{
    using ptr = std::shared_ptr<SmtpResult>;
    
    SmtpResult(uint16_t code, const std::string& desc)
    :m_resultCode(code)
    ,m_describe(desc){}

    uint16_t m_resultCode;
    std::string m_describe; 
};

class SmtpClient : public SockStream
{
public:
    using ptr = std::shared_ptr<SmtpClient>;

    static SmtpClient::ptr Create(const std::string& host, bool isSSL = true);

    //发送邮件
    //内部调用executeCmd()方法
    //对外返回是否发送成功
    bool send(Email::ptr email);
private:
    //host:要连接的地址
    SmtpClient(Socket::ptr sock, const std::string& host, bool isSSL);

    //连接邮箱服务器
    bool connect();

    //执行命令
    //内部获得邮件服务器回送的结果码
    //解析结果码
    //如果结果码代表错误，则返回false
    //不然返回true
    bool executeCmd(const std::string& cmd);
private:
    std::string m_host;
    bool m_isSSL;
};

}
}

#endif
