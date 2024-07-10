#include "Utils.h"

#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
// #include <stdlib.h>
#include <string.h>
// #include <unistd.h>
// #include <stdio.h>

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

Utils::Utils()
{
}

Utils::~Utils()
{
}

void Utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

// 对文件描述符设置非阻塞
int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);      // 用于操作文件描述符 获取文件状态标志
    int new_option = old_option | O_NONBLOCK; // 非阻塞模式。
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
    {
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    }
    else
    {
        event.events = EPOLLIN | EPOLLRDHUP; // 表示对“对端半关闭”（即对端调用了 shutdown() 函数）的事件感兴趣。当对端关闭连接时，此事件会被触发。
    }
    if (one_shot)
    {
        event.events |= EPOLLONESHOT; // 它控制的是 epoll 的行为，而不是描述感兴趣的 I/O 事件类型。这意味着 sockfd 只会在有读事件发生时被报告一次，之后它将不会再次触发事件，直到我们再次显式地将它添加到 epoll 实例中。
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

// 信号处理函数
void Utils::sig_handler(int sig)
{
    // 为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
}

// 设置信号函数
// 当定义一个函数模板或一个接受函数作为参数的函数时，
// 使用 void(handler)(int) 是一种语法糖，用来指定参数 handler 可以是任何接受 int 并返回 void 的函数或函数对象。在这种情况下，
// 不需要在 handler 前加星号，因为这不是变量声明，而是类型说明。
void Utils::addsig(int sig, void(handler)(int), bool restart)
// void Utils::addsig(int sig, std::function<void(int)> handler, bool restart)  //std::function 是 C++11 中引入的一种通用封装，它并不是一个函数指针，而是一个更复杂的模板类
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART; // 系统会在信号处理函数执行前后自动重启被信号中断的系统调用。
    }
    sigfillset(&sa.sa_mask);                 // 设置信号掩码，指定在信号处理期间需要屏蔽的信号
    assert(sigaction(sig, &sa, NULL) != -1); // 使用sigaction系统调用来设置信号处理
}

// 定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler()
{
    m_timerLst.tick();
    alarm(m_TIMESLOT); // 古老而简单
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}
