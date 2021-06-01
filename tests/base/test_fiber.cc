#include "emilia/base/thread.h"
#include "emilia/base/fiber.h"

#include <iostream>
#include <unistd.h>

void test1(){
    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " 启动" << std::endl;

    sleep(5);

    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " 切出" << std::endl;
    emilia::base::Fiber::YieldToPend();
    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " 主协程切入" << std::endl;
}

void test2(){
    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " 启动" << std::endl;
    sleep(5);
    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " 切出" << std::endl;
}

int main(){
    std::cout << "test" << std::endl;
    //创建主协程 id号为1
    emilia::base::Fiber::CreateMainFiber();
    emilia::base::Fiber::ptr test_1(new emilia::base::Fiber(test1));
    std::cout << "test" << std::endl;
    test_1->swapIn();
    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " test_1 切回主协程" << std::endl;
    test_1->swapIn();
    test_1->reset(test2);
    test_1->swapIn();
    std::cout << emilia::base::Thread::GetRunFiber()->getId() << " test_1 切回主协程" << std::endl;

    return 0;
}
