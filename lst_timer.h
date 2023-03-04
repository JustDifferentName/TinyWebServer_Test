#pragma once

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h> //C库提供调试方法
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> //提供了共享内存的管理方法
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h> //多路IO

#include <time.h>
#include "log.h"

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer* timer;
};

class util_timer //类似于双向链表
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire; //超时时间

    void (*cb_func)(client_data*);
    client_data* user_data;
    util_timer* prev;
    util_timer* next;
};

class sort_timer_lst //排序定时器链表
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer* timer);
    void adjust_timer(util_timer* timer);
    void del_timer(util_timer* timer);
    void tick(); //心搏时间检查非活动连接

private:
    void add_timer(util_timer* timer, util_timer* lst_head);

    util_timer* head;
    util_timer* tail;
};

class Utils //数据处理工具类
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot); //心搏时间

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char* info);

public:
    static int* u_pipefd; //全局的管道文件符指针
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data* user_data);