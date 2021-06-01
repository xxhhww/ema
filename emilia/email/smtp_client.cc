#include "smtp_client.h"
#include "emilia/net/address.h"
#include "emilia/net/socket.h"
#include "emilia/net/sslsocket.h"
#include "emilia/log/logmarco.h"
#include "emilia/util/codeutil.h"

#include <set>

namespace emilia{
namespace email{

SmtpClient::ptr SmtpClient::Create(const std::string& host, bool isSSL){
    //使用465端口
    if(isSSL){
        Socket::ptr sslSock = SSLSocket::CreateTcpV4();
        SmtpClient::ptr client(new SmtpClient(sslSock, host, isSSL));
        if(!client->connect()){
            return nullptr;
        }
        return client;
    }
    
    //使用25端口
    Socket::ptr sock = Socket::CreateTcpV4();
    SmtpClient::ptr client(new SmtpClient(sock, host, isSSL));
    if(!client->connect()){
        return nullptr;
    }
    return client;
}



//有待改进（只能访问465 SSL，不能访问25 smtp）
SmtpClient::SmtpClient(Socket::ptr sock, const std::string& host, bool isSSL)
:SockStream(sock)
,m_host(host)
,m_isSSL(isSSL)
{}

bool SmtpClient::connect(){
#define Error() \
    EMILIA_LOG_ERROR("system") <<"SmtpClient Connect "<< m_host <<" Error"; \
    return false;   \

    //获得邮件服务器的地址
    IPAddress::ptr address = IPAddress::CreateByHost(m_host);

    EMILIA_LOG_INFO("test") << "test";
    if (!m_sock->connect(address)){
        Error();
    }
    char buf[1024];
    int len = 0;

    if((len = read(buf, 1024)) <= 0){
        Error();
    }

    EMILIA_LOG_INFO("test") << buf;
    
    if(std::atoi((const char*)buf) != 220){
        Error();
    }
#undef Error

    return true;
}

bool SmtpClient::executeCmd(const std::string& cmd)
{
#define Error(str) \
    EMILIA_LOG_ERROR("system") << str; \
    return false;   \

    EMILIA_LOG_INFO("test") << cmd;

    if(writeFixByte(cmd.c_str(), cmd.size()) <= 0){
        Error("SmtpClient Write Error");
    }
   
    std::string buf;
    buf.resize(1024);
    int len = 0;
    
    if((len = read(&buf[0], buf.size())) <= 0 ){
        Error("SmtpClient Read Error");
    }

    EMILIA_LOG_INFO("test") << buf;

    size_t pos = buf.find(' ');
    std::string code = buf.substr(0, pos);
    if(std::atoi(code.c_str()) >= 400){
        Error("SmtpServer Code Error");
    }
    return true;
#undef Error
}

bool SmtpClient::send(Email::ptr email)
{
#define DoCmd() \
    if(!executeCmd(cmd))    \
        return false;       \

    std::string cmd = "HELO " + m_host + "\r\n";
    DoCmd();

    cmd = "auth login\r\n";
    DoCmd();

    std::string fromAddress = email->getMailFrom();
    size_t pos = fromAddress.find('@');

    cmd = emilia::util::base64encode(fromAddress.substr(0, pos)) + "\r\n";
    DoCmd();

    cmd = emilia::util::base64encode(email->getPassWord()) + "\r\n";
    DoCmd();

    cmd = "MAIL FROM: <" + fromAddress + ">\r\n";
    DoCmd();

    std::set<std::string> rcptTo;

//获得所有收件人的地址(包括To Cc Bcc)
#define GetAddress(function)    \
    for(auto& i : email->function())   \
        rcptTo.insert(i);   \

    GetAddress(getMailTo);
    GetAddress(getMailCc);
    GetAddress(getMailBcc);
#undef GetAddress
    for(auto& i : rcptTo) {
        cmd = "RCPT TO: <" + i + ">\r\n";
        DoCmd();
    }

    //开始输入内容
    cmd = "DATA\r\n";
    DoCmd();

    std::stringstream body;
    body << "From: " << email->getFrom() << "\r\n";
    body << "To: " <<  email->getTo() << "\r\n";

    body << "Subject: "<< email->getSubject() << "\r\n";
    body << "MIME-Version: 1.0\r\n";

    std::string boundary = "simpleboundary";
    //判断m_multipart是否为空
    //如果为空，则说明只有一个Mime对象
    //不然为多个
    if(email->getMultipart().empty()){
        for(auto& i : email->getMimeEntity())
        {
            body << i->toString();
        }
    }
    else{
        body << "Content-Type: " << email->getMultipart() << " ;boundary=" << boundary << "\r\n";
        for(auto& i : email->getMimeEntity())
        {
            body << "\r\n--" << boundary <<"\r\n";
            body << i->toString();
        }
        body << "\r\n--" << boundary << "--\r\n";
    }
    body << "\r\n.\r\n";
    cmd = body.str();
    DoCmd();
#undef DoCmd
    return true;
    
}

}
}
