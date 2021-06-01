#include "emilia/http/http_session.h"

int main()
{
    emilia::Socket::ptr sock(new emilia::Socket(AF_INET, SOCK_STREAM, 0) );

    emilia::http::HttpSession::ptr session(new emilia::http::HttpSession(sock));

    

    return 0;
}