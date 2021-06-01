#include "http_session.h"
#include "http_parser.h"

namespace emilia{
namespace http{



HttpSession::HttpSession(Socket::ptr sock)
:SockStream(sock)
{}

//待改进：不用4096而用配置模块直接限定
HttpRequest::ptr HttpSession::recvRequest(){
    //http请求解析器
    HttpRequestParser::ptr Parser(new HttpRequestParser());

    char* Buffer = new char[8192];

    int offset = 0;
    //代表Buffer的总长度
    int len = 0;

    //请求状态
    RequestStatus ReqStatus = RequestStatus::REQUEST_OK;

    do{
        if((len = read(Buffer, 8192)) <= 0)
            return nullptr;
        len = len + offset;
        //len = read(Buffer, 8192) + offset;      //从sock中收取数据
        offset = len;
        //EMILIA_LOG_INFO("test") << "Buffer";
        //将刚刚读取的数据解析
        ReqStatus = Parser->execute(Buffer, len);
        //请求正确
        if(ReqStatus == RequestStatus::REQUEST_OK)
            break;
        //请求出错
        if(ReqStatus == RequestStatus::REQUEST_BAD)
            return nullptr;
    }while(true);

    delete[]  Buffer;

    auto Result = Parser->getResult();
    Result->init();    
    
    return Result;
}

int HttpSession::sendResponse(HttpResponse::ptr rsp){
    std::string str = rsp->toString();
    return write(str.c_str(), str.size());
}

}
}
