#pragma once

#include <exception>
#include <pthread.h>
#include <semaphore.h>

//本文件应算作hpp文件，实现了各方法

class sem //信号量类
{
public:
    sem() //构造函数
    {
        if (sem_init(&m_sem, 0, 0) != 0) //三个参数分别是对象信号量指针、信号量类型、信号量初值，成功时返回0
        {
            throw std::exception(); //这里选择用抛出异常来处理，是因为构造函数没有返回值
        }
    }
    sem(int num) //有参构造
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem() //析构
    {
        sem_destroy(&m_sem); //在析构函数中释放信号量
    }
    bool wait() //该方法作用为：若检测到信号量值已为0，则返回真，用于阻塞进程
    {
        return sem_wait(&m_sem) == 0; //以原子操作将对象信号量值-1，若值已为0，则进程挂起
    }
    bool post() 
    {
        return sem_post(&m_sem) == 0; //对象信号量值+1，与wait对应
    }

private:
    sem_t m_sem; //每个信号量类中只封装了一个信号量及其方法
};

class locker //互斥锁类
{
public:
    locker() //构造函数
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0) //初始化的两个参数为对象锁指针和其类型，成功返回0
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex); //在析构函数中释放锁
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0; //以原子操作上锁
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0; //以原子操作解锁
    }
    pthread_mutex_t* get() //额外提供了获得锁指针的get方法
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex; //同样是每个锁类只封装一个锁及其方法
};

class cond //条件变量类，和书上不同的是没有将应该和条件变量一起使用的互斥锁封装在类中，而是在需要锁的时候(wait&timewait)再调用一个锁
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0) //初始化的两个参数是目标条件变量指针及其类型
        {
            //pthread_mutex_destroy(&m_mutex); 
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t* m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex); //调用已上锁的mutex参数是为了保护条件变量的原子性，成功时返回0
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t* m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t); //前两个参数和wait一样，多了个timespec结构体指针，包含两个变量秒和纳秒
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0; //唤醒一个等待目标条件变量的线程
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0; //广播唤醒所有等待目标条件变量的线程
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond; //仅封装了条件变量，辅助锁即其操作未封装
};