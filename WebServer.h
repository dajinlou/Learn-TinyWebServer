#ifndef WEBSERVER_H
#define WEBSERVER_H
#include "SqlConnectionPool.h"
#include "HttpConn.h"
#include "ThreadPool.h"
#include "Utils.h"

#include <sys/epoll.h>

const int MAX_FD = 65536;           // 最大文件描述符
const int MAX_EVENT_NUMBER = 10000; // 最大事件数
const int TIMESLOT = 5;             // 最小超时单位

// 登录信息
struct Stu_sqlInfo
{
    string user;
    string password;
    string databaseName;
};

class WebServer
{
private:
    /* data */
public:
    WebServer(/* args */);
    ~WebServer();

    void init(int port, Stu_sqlInfo &sqlInfo, int log_write, int opt_linger, int trigmode, int sql_num, int thread_num, int close_log, int actor_model);

    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(UtilTimer *timer);
    void deal_timer(UtilTimer *timer, int sockfd);
    bool dealclientdata();
    bool dealwithsignal(bool &timeout, bool &stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

public:
    // 服务器本身
    int m_port;
    char *m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_pipefd[2];
    int m_epollfd;
    HttpConn *users;

    // 数据库相关
    SqlConnectionPool *m_connPool;
    string m_user;
    string m_password;
    string m_dbName;
    int m_sql_num;

    // 线程池相关
    ThreadPool<HttpConn> *m_pool;
    int m_thread_num;

    // epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_OPT_LINGER; // 是否开启长连接
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    // 定时器相关
    client_data *users_timer;
    Utils utils;
};

#endif // WEBSERVER_H