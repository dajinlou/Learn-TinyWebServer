#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

using namespace std;
class Sem
{
private:
    sem_t m_sem;
    /* data */
public:
    Sem(/* args */)
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw exception();
        }
    }
    Sem(int num){
        if(sem_init(&m_sem,0,num)!=0){
            throw exception();
        }
    }
    ~Sem()
    {
        sem_destroy(&m_sem);
    }

    bool wait();

    bool post();
};

inline bool Sem::wait()
{
    return sem_wait(&m_sem) == 0;
}

inline bool Sem::post()
{
    return sem_post(&m_sem) == 0;
}

class Locker
{
private:
    /* data */
    pthread_mutex_t m_mutex;

public:
    Locker(/* args */)
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw exception();
        }
    }
    ~Locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    // 加锁
    bool lock();
    // 解锁
    bool unlock();

    // 获取互斥锁的指针
    pthread_mutex_t *get();
};

inline bool Locker::lock()
{
    return pthread_mutex_lock(&m_mutex) == 0;
}

inline bool Locker::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

inline pthread_mutex_t *Locker::get()
{
    return &m_mutex;
}

class Cond
{
private:
    /* data */
    pthread_cond_t m_cond;

public:
    Cond(/* args */)
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            throw exception();
        }
    }
    ~Cond()
    {
        pthread_cond_destroy(&m_cond);
    }

    // 等待
    bool wait(pthread_mutex_t *mutex)
    {
        return pthread_cond_wait(&m_cond, mutex) == 0;
    }

    // 超时等待
    bool timewait(pthread_mutex_t *mutex, struct timespec *m_abstime)
    {
        return pthread_cond_timedwait(&m_cond, mutex, m_abstime) == 0;
    }

    // 唤醒一个
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

    // 广播  但也只是唤醒全部，但只有一个获得使用权
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
};

#endif // LOCKER_H