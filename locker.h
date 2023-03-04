#pragma once

#include <exception>
#include <pthread.h>
#include <semaphore.h>

//���ļ�Ӧ����hpp�ļ���ʵ���˸�����

class sem //�ź�����
{
public:
    sem() //���캯��
    {
        if (sem_init(&m_sem, 0, 0) != 0) //���������ֱ��Ƕ����ź���ָ�롢�ź������͡��ź�����ֵ���ɹ�ʱ����0
        {
            throw std::exception(); //����ѡ�����׳��쳣����������Ϊ���캯��û�з���ֵ
        }
    }
    sem(int num) //�вι���
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem() //����
    {
        sem_destroy(&m_sem); //�������������ͷ��ź���
    }
    bool wait() //�÷�������Ϊ������⵽�ź���ֵ��Ϊ0���򷵻��棬������������
    {
        return sem_wait(&m_sem) == 0; //��ԭ�Ӳ����������ź���ֵ-1����ֵ��Ϊ0������̹���
    }
    bool post() 
    {
        return sem_post(&m_sem) == 0; //�����ź���ֵ+1����wait��Ӧ
    }

private:
    sem_t m_sem; //ÿ���ź�������ֻ��װ��һ���ź������䷽��
};

class locker //��������
{
public:
    locker() //���캯��
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0) //��ʼ������������Ϊ������ָ��������ͣ��ɹ�����0
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex); //�������������ͷ���
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0; //��ԭ�Ӳ�������
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0; //��ԭ�Ӳ�������
    }
    pthread_mutex_t* get() //�����ṩ�˻����ָ���get����
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex; //ͬ����ÿ������ֻ��װһ�������䷽��
};

class cond //���������࣬�����ϲ�ͬ����û�н�Ӧ�ú���������һ��ʹ�õĻ�������װ�����У���������Ҫ����ʱ��(wait&timewait)�ٵ���һ����
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0) //��ʼ��������������Ŀ����������ָ�뼰������
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
        ret = pthread_cond_wait(&m_cond, m_mutex); //������������mutex������Ϊ�˱�������������ԭ���ԣ��ɹ�ʱ����0
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t* m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t); //ǰ����������waitһ�������˸�timespec�ṹ��ָ�룬�������������������
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0; //����һ���ȴ�Ŀ�������������߳�
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0; //�㲥�������еȴ�Ŀ�������������߳�
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond; //����װ�������������������������δ��װ
};