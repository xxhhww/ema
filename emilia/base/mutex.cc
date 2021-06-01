#include "mutex.h"

namespace emilia{
namespace base{
    //============================================================
//class Semaphore

Semaphore::Semaphore(uint32_t count)
{
    if( sem_init(&m_semaphore, 0, count) != 0 )
    {
        throw std::logic_error("sem_init error");
    }
}
Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}
bool Semaphore::post()
//信号量增加
{
    return sem_post(&m_semaphore) == 0;
}
bool Semaphore::wait()
//等待信号量
{
    return sem_wait(&m_semaphore) == 0;
}

//============================================================
//互斥锁封装类、自旋锁封装类、读写锁封装类
Mutex::Mutex()
{
    if( pthread_mutex_init(&m_mutex, 0) != 0 )
    {
        throw std::logic_error("mutex_init error");
    }
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

bool Mutex::lock()
{
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool Mutex::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

RWMutex::RWMutex()
{
    pthread_rwlock_init(&m_lock, nullptr);
}

RWMutex::~RWMutex()
{
    pthread_rwlock_destroy(&m_lock);
}

bool RWMutex::rdlock()
{
    return pthread_rwlock_rdlock(&m_lock) == 0;
}
bool RWMutex::wrlock()
{
    return pthread_rwlock_wrlock(&m_lock) == 0;
}
bool RWMutex::unlock()
{
    return pthread_rwlock_unlock(&m_lock) == 0;
}

}
}