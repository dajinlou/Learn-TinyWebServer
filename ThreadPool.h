#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "SqlConnectionPool.h"

template <typename T>
class ThreadPool
{
private:
    /* data */
public:
    // max_requests是请求队列中最多允许的、等待处理的请求的数量
    ThreadPool(int actor_model, SqlConnectionPool *connPool, int thread_number = 8, int max_request = 10000);
    ~ThreadPool();

    bool append(T *request, int state);
    bool append_p(T *request);

private:
    // 工作线程允许的函数
    static void *worker(void *arg);
    void run();

private:
    int m_thread_number;           // 线程池中的线程数
    int m_max_requests;            // 请求队列中允许的最大请求数
    pthread_t *m_threads;          // 描述线程池的数组，其大小为m_thread_number
    std::list<T *> m_workQueue;    // 请求队列
    Locker m_queueLocker;          // 保护请求队列的互斥锁
    Sem m_queueStat;               // 是否有任务需要处理
    SqlConnectionPool *m_connPool; // 数据库
    int m_actor_model;             // 模型切换
};

template <typename T>
ThreadPool<T>::ThreadPool(int actor_model, SqlConnectionPool *connPool, int thread_number, int max_request)
    : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_request), m_threads(NULL), m_connPool(connPool)

{
    if (thread_number <= 0 || max_request <= 0)
    {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if (nullptr == m_threads)
    {
        throw std::exception();
    }
    for (int i = 0; i < thread_number; ++i)
    {
        // pthread_create(m_threads + i, NULL, worker, this) != 0
        if (pthread_create(&m_threads[i], NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    if (m_threads != NULL)
    {
        delete[] m_threads;
    }
}

template <typename T>
inline bool ThreadPool<T>::append(T *request, int state)
{
    m_queueLocker.lock();
    if (m_workQueue.size() >= m_max_requests)
    {
        m_queueLocker.unlock();
        return false;
    }
    /*在本项目中，request通常是http数据类型，而m_state则表示该request是读事件还是写事件*/
    request->m_state = state;
    m_workQueue.push_back(request);
    m_queueLocker.unlock();
    m_queueStat.post(); // 添加完后就可以唤醒线程来取事件处理进行处理
    return true;
}

template <typename T>
inline bool ThreadPool<T>::append_p(T *request)
{
    m_queueLocker.lock();
    if (m_workQueue.size() >= m_max_requests)
    {
        m_queueLocker.unlock();
        return false;
    }
    m_workQueue.push_back(request);
    m_queueLocker.unlock();
    m_queueStat.post();
    return true;
}

template <typename T>
inline void *ThreadPool<T>::worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    pool->run();
    return pool;
}

template <typename T>
inline void ThreadPool<T>::run()
{
    while (true)
    {
        m_queueStat.wait();
        m_queueLocker.lock();
        if (m_workQueue.empty())
        {
            m_queueLocker.unlock();
            continue;
        }
        T *request = m_workQueue.front();
        m_workQueue.pop_front();
        m_queueLocker.unlock();
        if (!request)
        {
            continue;
        }
        if (1 == m_actor_model)
        {
            if (0 == request->m_state)
            {
                if (request->read_once())
                {
                    request->improv = 1;
                    ConnectionRAII mysqlcon(&request->mysql, m_connPool);
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else
            {

                if (request->write())
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else
        {
            ConnectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}

#endif // THREADPOOL_H