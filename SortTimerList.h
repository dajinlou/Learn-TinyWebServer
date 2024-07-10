#ifndef SORTTIMERLIST_H
#define SORTTIMERLIST_H
#include "UtilTimer.h"
class SortTimerList
{

public:
    SortTimerList(/* args */);
    ~SortTimerList();

    void add_timer(UtilTimer *timer);
    void adjust_timer(UtilTimer *timer);
    void del_timer(UtilTimer *timer);
    void tick();

private:
    void add_timer(UtilTimer *timer, UtilTimer *lst_head);

private:
    /* data */
    UtilTimer *head;
    UtilTimer *tail;
};

void cb_func(client_data *user_data);
#endif // SORTTIMERLIST_H