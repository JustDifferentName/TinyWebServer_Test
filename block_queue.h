#pragma once

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "locker.h"
using namespace std;

//���������з�װ��һ����Դ���У�T��ĳ����Դ

template <class T>
class block_queue {
private:
    locker m_mutex; //�����������������ݣ������һ���߳���Ҫ�ȴ�һ�����������ͷţ����߳�ͨ����Ҫ��ѯ�û������Ƿ��ѱ��ͷ�
    cond m_cond;  //��������ʹ�á�֪ͨ�����ѡ�ģ�ͣ�������������һ�����ݺ�֪ͨ������ʹ�ã���������δ�ӵ�֪ͨǰ��������״̬��ԼCPU��Դ��
    //���������յ�֪ͨ�󣬸Ͻ�������״̬���������������ݣ�ʹ�����¼�����ģ�ͣ��ڱ�֤�����¶�������¾����ܼ������ù����Ͷ���Դ�����ġ�

    T* m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;

public:
    block_queue(int max_size = 1000) //�вδ�Ĭ��ֵ���캯�����޲����ÿ���ΪĬ�Ϲ��캯��
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        m_max_size = max_size;
        m_array = new T[max_size]; //���������˴洢T�������ݵĶ���
        m_size = 0; //δ����Ԫ�أ���ʱ��СΪ0
        m_front = -1;
        m_back = -1; //δ����Ԫ�أ���ʱͷβָ�붼ָ��ǰ�ڱ���
    }

    void clear() //��ն��У���������ǰ������
    {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    ~block_queue() //���������ͷŶ��У��ǵ�������
    {
        m_mutex.lock();
        if (m_array != NULL)
            delete[] m_array;

        m_mutex.unlock();
    }

    //�ж϶����Ƿ�����
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

    //�ж϶����Ƿ�Ϊ��
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

    //�ô�������value�����ն���Ԫ��
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

    //�ô�������value�����ն�βԪ��
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

    int size() //���ش�С
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_size;

        m_mutex.unlock();
        return tmp;
    }

    int max_size() //�����ݻ�
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_max_size;

        m_mutex.unlock();
        return tmp;
    }

    bool push(const T& item) //���ô��������ٸ����ռ�����ģ���const�޶�ֻ����push�����������ߴ�������Դ
    {
        m_mutex.lock();
        if (m_size >= m_max_size) //�������磬������Դ���˷�ʱ
        {
            m_cond.broadcast(); 
            m_mutex.unlock();
            return false;
        }

        m_back = (m_back + 1) % m_max_size; //ѭ�����
        m_array[m_back] = item;

        m_size++;

        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    //popʱ,�����ǰ����û��Ԫ��,����ȴ���������
    bool pop(T& item) //��֮ǰһ����ʹ�ô������������ճ�����Դ
    {

        m_mutex.lock();
        while (m_size <= 0) //�������磬������Դʱ
        {

            if (!m_cond.wait(m_mutex.get())) //�õ�ǰ�߳�����
            {
                m_mutex.unlock();
                return false;
            }
        }

        m_front = (m_front + 1) % m_max_size; //ѭ������
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    bool pop(T& item, int ms_timeout) //��ʱ������pop
    {
        struct timespec t = { 0, 0 }; //��ȷ������
        struct timeval now = { 0, 0 }; //��ȷ��΢��
        gettimeofday(&now, NULL);
        m_mutex.lock();
        if (m_size <= 0) //����ʱ�ſ��ܳ�ʱ�����������ᳬʱ
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t)) //��ʱ���
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