#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h> //�������ڲ�������δ֪ʱ��ȡ�����еĲ���
#include <pthread.h>
#include "block_queue.h"
using namespace std;

class Log 
{
public:
    //C++11�Ժ�,ʹ�þֲ������������ü���
    static Log* get_instance() //����ģʽ����֤һ�������һ��ʵ�������ṩһ����������ȫ�ַ��ʵ㣬��ʵ�������г���ģ�鹲��
    {
        static Log instance; //����ʵ���ڵ�һ�α�ʹ��ʱ�Ž��г�ʼ���������ӳٳ�ʼ��������static�޶�����ȫ�ֱ���
        return &instance;
    }

    static void* flush_log_thread(void* args) //�����߳���־
    {
        Log::get_instance()->async_write_log(); //����ʵ����ͬ��д��־����
    }

    //��ѡ��Ĳ�������־�ļ�����־��������С����������Լ����־������
    bool init(const char* file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char* format, ...);

    void flush(void);

private:
    Log(); //˽�л����Ĺ��캯�����Է�ֹ��紴��������Ķ���
    virtual ~Log();
    void* async_write_log()
    {
        string single_log;
        //������������ȡ��һ����־string��д���ļ�
        while (m_log_queue->pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp); //fputs��һ���������ַ������ڶ����������ļ�ָ��
            m_mutex.unlock();
        }
    }

private:
    char dir_name[128]; //·����
    char log_name[128]; //log�ļ���
    int m_split_lines;  //��־�������
    int m_log_buf_size; //��־��������С
    long long m_count;  //��־������¼
    int m_today;        //��Ϊ�������,��¼��ǰʱ������һ��
    FILE* m_fp;         //��log���ļ�ָ�룬FILE������<stdio.h>��
    char* m_buf;
    block_queue<string>* m_log_queue; //��������
    bool m_is_async;                  //�Ƿ�ͬ����־λ
    locker m_mutex;
    int m_close_log; //�ر���־
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}