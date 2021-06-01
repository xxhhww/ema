#include "emilia/base/thread.h"
#include "unistd.h"

#include <iostream>
void test(){
    std::cout << emilia::base::Thread::GetThis()->getId() << "启动成功" << std::endl;
    sleep(5);
    std::cout << emilia::base::Thread::GetId() << "测试线程结束" << std::endl;
}

int main(){
    emilia::base::Thread::ptr thread(new emilia::base::Thread(test, "测试线程"));
    thread->start();
    std::cout << "等待回收" << std::endl;
    thread->join();
    std::cout << "回收成功" << std::endl;
    return 0;
}