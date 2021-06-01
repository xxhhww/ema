#include "emilia/emilia.h"

int main()
{
    char str[] = {
    "GET / HTTP/1.1\r\n"
    "Host: www.baidu.com\r\n"
    "Accept: sadarwefasf\r\n"
    "Content-Length: 11\r\n"
    "\r\n"
    "hello world"
    };


    emilia::http::HttpRequestParser::ptr Parser(new emilia::http::HttpRequestParser());

    Parser->execute(str, strlen(str));

    std::cout << Parser->getResult()->toString() << std::endl;

    emilia::http::HttpResponse::ptr rps (new emilia::http::HttpResponse(Parser->getResult()->isclose()));
    rps->setVersion(Parser->getResult()->getVersion());
    std::cout << rps->toString();

    return 0;
}