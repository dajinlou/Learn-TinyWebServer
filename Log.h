#ifndef LOG_H
#define LOG_H
#include "BlockQueue.h"

#include <stdio.h>
#include <stdarg.h>


using namespace std;

class Log
{
public:
    /*日志单例模式2：创建一个共有静态方法获得实例，并用指针返回*/
    static Log *get_instance()
    {
        static Log instance; // C++11以后懒汉模式无需加锁，编译器会保证局部静态变量的线程安全
        return &instance;
    }

    static void *flush_log_thread(void *args){
        Log::get_instance()->async_write_log();
    }

    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char* file_name, int close_log,int log_buf_size = 8192,int split_lines = 5000000,int max_queue_size = 0);

    void write_log(int level,const char* format,...);

    void flush(void);

private:
    /*日志单例模式1：私有化构造函数，确保外界无法创建新实例*/
    Log();
    virtual ~Log();
    void *async_write_log()
    {
        string single_log;
        while(m_log_queue->pop(single_log)){
            m_mutex.lock();
            fputs(single_log.c_str(),m_fp);
            m_mutex.lock();
        }
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    FILE *m_fp;            // 打开log的文件指针
    long long m_count = 0; // 日志行数记录
    bool m_is_async;       // 是否是异步
    int m_today;  //因为按天分类，记录当前时间是那一天
    char *m_buf;
    BlockQueue<string> *m_log_queue; //阻塞队列
    Locker m_mutex;
    int m_close_log;   //关闭日志
};

//##__VA_ARGS__ 这是 GNU 扩展，用于在宏中处理可变参数## 操作符用于在宏展开时连接参数，如果参数为空，它将什么也不做。这避免了在空宏参数列表的情况下生成多余的逗号，这在某些编译器中可能导致编译错误。
#define LOG_DEBUG(format,...) if(0 == m_close_log){Log::get_instance()->write_log(0,format,##__VA_ARGS__);Log::get_instance()->flush();}
#define LOG_INFO(format,...) if(0 == m_close_log){Log::get_instance()->write_log(1,format,##__VA_ARGS__);Log::get_instance()->flush();}
#define LOG_WARN(format,...) if(0 == m_close_log){Log::get_instance()->write_log(2,format,##__VA_ARGS__);Log::get_instance()->flush();}
#define LOG_ERROR(format,...) if(0 == m_close_log){Log::get_instance()->write_log(3,format,##__VA_ARGS__);Log::get_instance()->flush();}

#endif