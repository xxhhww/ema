#include "emilia/http/http_serve.h"

#include "jsoncpp/json/json.h"

int main()
{

    auto addr = emilia::IPAddress::CreateByString("172.16.231.120", 30002);
    emilia::IO_Manger* iomanger = new emilia::IO_Manger(8);

    emilia::http::HttpServe httpServe(iomanger, addr);
    httpServe.run();

    return 0;
}
