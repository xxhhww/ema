#include "emilia/base/timer.h"
#include "emilia/base/scheduler.h"
#include <iostream>

void time1(){
    std::cout << emilia::base::Fiber::GetId() << " 定时触发" << std::endl;
}

void time2(int i){
    std::cout << emilia::base::Fiber::GetId() << " 触发: " << i << std::endl;
}

int main(){
    emilia::base::Scheduler* scheduler = new emilia::base::Scheduler();

    scheduler->addTimer(5000, time1, true);

    scheduler->addTimer(3000, std::bind(time2, 3), false);

    scheduler->run();

    delete scheduler;
}