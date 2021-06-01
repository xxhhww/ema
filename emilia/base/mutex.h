#ifndef _EMILIA_MUTEX_H_
#define _EMILIA_MUTEX_H_

#include "pthread.h"
#include "semaphore.h"
#include <string>
#include <memory>
#include <stdint.h>

namespace emilia{
namespace base{
//============================================================
//信号量封装类
class Semaphore
{
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    bool post();    //信号量增加
    bool wait();    //等待信号量
private:
    //禁止Semaphore的拷贝方法
    Semaphore(Semaphore& ) = delete;
    Semaphore(Semaphore&& ) = delete;
    Semaphore& operator=(const Semaphore& ) = delete;
private:
    sem_t m_semaphore;
};

//============================================================
//通用锁封装类
//进一步对封装，使得
//在对象构造时加锁，对象析构时解锁，同时方便使用
template <class T>
class ScopedLock
{
public:
    ScopedLock(T& lock):m_lock(lock)
    {
        m_lock.lock();
        m_islock = true;
    }

    bool unlcok()
    {
        m_islock = false;
        return m_lock.unlock();
    }

    ~ScopedLock()
    {
        if(m_islock)
            m_lock.unlock();
    }
private:
    T& m_lock;
    bool m_islock;
};

template <class T>
class ReadScopedLock
{
public:
    ReadScopedLock(T& lock):m_lock(lock)
    {
        m_lock.rdlock();
        m_islock = true;
    }

    bool unlock()
    {
        m_islock = false;
        return m_lock.unlock();
    }

    ~ReadScopedLock()
    {
        if(m_islock)
            m_lock.unlock();
    }
private:
    T& m_lock;
    bool m_islock;
};

template <class T>
class WriteScopedLock
{
public:
    WriteScopedLock(T& lock):m_lock(lock)
    {
        m_lock.wrlock();
        m_islock = true;
    }

    bool unlock()
    {
        m_islock = false;
        return m_lock.unlock();
    }

    ~WriteScopedLock()
    {
        if(m_islock)
            m_lock.unlock();
    }
private:
    T& m_lock;
    bool m_islock;
};

//============================================================
//互斥锁封装类、自旋锁封装类、读写锁封装类
class Mutex
{
public:
    using Lock = ScopedLock<Mutex>;
    Mutex();
    ~Mutex();
    bool lock();
    bool unlock();
private:
    pthread_mutex_t m_mutex;
};

class SpinLock
{
public:
    using spLock = ScopedLock<SpinLock>;
    SpinLock();
    ~SpinLock();
    bool lock();
    bool unlock();
private:
    pthread_spinlock_t m_lock;
};

class RWMutex
{
public:
    using rdLock = ReadScopedLock<RWMutex>;
    using wrLock = WriteScopedLock<RWMutex>;
    RWMutex();
    ~RWMutex();
    bool rdlock();
    bool wrlock();
    bool unlock();
private:
    pthread_rwlock_t m_lock;
};
}
}
#endif