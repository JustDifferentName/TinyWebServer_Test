#pragma once

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "locker.h"
using namespace std;

//阻塞队列中封装了一个资源队列，T是某种资源

template <class T>
class block_queue {
private:
    locker m_mutex; //互斥锁保护共享数据，但如果一个线程需要等待一个互斥锁的释放，该线程通常需要轮询该互斥锁是否已被释放
    cond m_cond;  //条件变量使用“通知—唤醒”模型，生产者生产出一个数据后通知消费者使用，消费者在未接到通知前处于休眠状态节约CPU资源；
    //当消费者收到通知后，赶紧从休眠状态被唤醒来处理数据，使用了事件驱动模型，在保证不误事儿的情况下尽可能减少无用功降低对资源的消耗。

    T* m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;

public:
    block_queue(int max_size = 1000) //有参带默认值构造函数，无参引用可作为默认构造函数
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        m_max_size = max_size;
        m_array = new T[max_size]; //堆区开辟了存储T类型数据的队列
        m_size = 0; //未加入元素，此时大小为0
        m_front = -1;
        m_back = -1; //未加入元素，此时头尾指针都指向前哨兵点
    }

    void clear() //清空队列，操作数据前先上锁
    {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    ~block_queue() //析构函数释放队列，记得锁操作
    {
        m_mutex.lock();
        if (m_array != NULL)
            delete[] m_array;

        m_mutex.unlock();
    }

    //判断队列是否满了
    bool full()
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    //判断队列是否为空
    bool empty()
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    //用传出参数value来接收队首元素
    bool front(T& value)
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    //用传出参数value来接收队尾元素
    bool back(T& value)
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    int size() //返回大小
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_size;

        m_mutex.unlock();
        return tmp;
    }

    int max_size() //返回容积
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_max_size;

        m_mutex.unlock();
        return tmp;
    }

    bool push(const T& item) //引用传入来减少辅助空间的消耗，加const限定只读，push类似于生产者存入了资源
    {
        m_mutex.lock();
        if (m_size >= m_max_size) //队列上溢，即有资源将浪费时
        {
            m_cond.broadcast(); 
            m_mutex.unlock();
            return false;
        }

        m_back = (m_back + 1) % m_max_size; //循环入队
        m_array[m_back] = item;

        m_size++;

        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    //pop时,如果当前队列没有元素,将会等待条件变量
    bool pop(T& item) //和之前一样，使用传出参数来接收出队资源
    {

        m_mutex.lock();
        while (m_size <= 0) //队列下溢，即无资源时
        {

            if (!m_cond.wait(m_mutex.get())) //让当前线程阻塞
            {
                m_mutex.unlock();
                return false;
            }
        }

        m_front = (m_front + 1) % m_max_size; //循环出队
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    bool pop(T& item, int ms_timeout) //超时处理版的pop
    {
        struct timespec t = { 0, 0 }; //精确到纳秒
        struct timeval now = { 0, 0 }; //精确到微秒
        gettimeofday(&now, NULL);
        m_mutex.lock();
        if (m_size <= 0) //阻塞时才可能超时，不阻塞不会超时
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t)) //超时检测
            {
                m_mutex.unlock();
                return false;
            }
        }

        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }
};