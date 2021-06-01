#include "emilia/emilia.h"

int main()
{
    emilia::http::HttpRequest::ptr httpRequest( new emilia::http::HttpRequest() );

    httpRequest->setMethod(emilia::http::HttpMethod::GET);

    httpRequest->setUrl("/");

    httpRequest->setQuery("name=1&password=83442383");

    httpRequest->setFragment("id=3");


    httpRequest->setVersion(0x11);

    httpRequest->setHeader("Host", "www.baidu/com");

    
    httpRequest->setHeader("connection", "close");

    httpRequest->setBody("hello");

    std::cout << httpRequest->toString();

    return 0;
}