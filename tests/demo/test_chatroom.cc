#include "emilia/demo/chatroom/chatserve.h"

using namespace emilia::base;
using namespace emilia::net;

int main(){
    signal(SIGPIPE, SIG_IGN);
    chatServe::ptr chatserve(new chatServe(IOManger::Create(4), IPAddress::CreateByString("172.16.231.120", 20000)));
    chatserve->run();
}