#ifndef UTILTIMER_H
#define UTILTIMER_H

#include<time.h>
#include <arpa/inet.h>

class UtilTimer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    UtilTimer *timer;
};

class UtilTimer
{
public:
    UtilTimer(/* args */);
    ~UtilTimer();


public:
    /* data */
    time_t expire;
    void (* cb_func)(client_data *);  //函数指针
    client_data *user_data;
    UtilTimer *prev;
    UtilTimer *next;
};





#endif //UTILTIMER_H