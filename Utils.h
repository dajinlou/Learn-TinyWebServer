#ifndef UTILS_H
#define UTILS_H
#include "SortTimerList.h"

#include <functional>

class Utils
{

public:
    Utils(/* args */);
    ~Utils();

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd,int fd,bool one_shot, int TRIGMode);

    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig,void(handler)(int),bool restart = true);
    // void addsig(int sig,std::function<void(int)> handler, bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd,const char *info);

public:
    static int *u_pipefd;
    static int u_epollfd;
    SortTimerList m_timerLst;
    int m_TIMESLOT;
};

#endif // UTILS_H