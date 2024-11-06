#ifndef RINGBUFFER_H
#define RINGBUFFER_H
template <class T>
class RingBuffer {
public:
    RingBuffer(unsigned int sz): m_size(sz), m_font(0), m_rear(0) {
        m_buffer = new T[sz];//初始化
    }
    ~RingBuffer() {
        delete [] m_buffer;
        m_buffer = nullptr; //防止野指针
    }

    inline bool isEmpty() const {
        return m_font == m_rear;
    }

    inline bool isFull() const {
        return m_font == (m_rear + 1) % m_size;
    }
            
    //&
    bool enqueue(const T& data) {
        if(isFull()) {
            return false;
        }

        m_buffer[m_rear] = data;
        m_rear = (m_rear + 1) % m_size;
        return true;
    }
            
    //*
    bool enqueue(const T* data) {
        if(isEmpty()) {
            return false;
        }

        m_buffer[m_rear] = *data;
        m_rear = (m_rear + 1) % m_size;
        return true;
    }

    inline bool dequeue(T& data) {
        if(isEmpty()) {
            return false;
        }

        data = m_buffer[m_font];
        m_font = (m_font + 1) % m_size;
        return true;
    }

    inline bool dequeue(T* data) {
        if(isEmpty()) {
            return false;
        }

        *data = m_buffer[m_font];
        m_font = (m_font + 1) % m_size;
        return true;
    }

    unsigned int size() const {
         return m_size;
    }

    unsigned int font() const {
        return m_font;
    }

    unsigned int rear() const {
        return m_rear;
    }

private:
    unsigned int m_size;//环形队列的大小
    int m_font; //环形队列的前端
    int m_rear; //环形队列的尾端
    T* m_buffer; //环形队列的缓冲区
};

#endif // RINGBUFFER_H 

/*#pragma once
 
template <class T>
class RingBuffer
{
public:
    RingBuffer(unsigned size): m_size(size), m_front(0), m_rear(0)
    {
        m_data = new T[size];
    }
 
    ~RingBuffer()
    {
        delete [] m_data;
        m_data = nullptr;
    }
 
    inline bool isEmpty() const
    {
        return m_front == m_rear;
    }
 
    inline bool isFull() const
    {
        return m_front == (m_rear + 1) % m_size;
    }
 
    bool push(const T& value)
    {
        if(isFull())
        {
            return false;
        }
        m_data[m_rear] = value;
        m_rear = (m_rear + 1) % m_size;
        return true;
    }
 
    bool push(const T* value)
    {
        if(isFull())
        {
            return false;
        }
        m_data[m_rear] = *value;
        m_rear = (m_rear + 1) % m_size;
        return true;
    }
 
    inline bool pop(T& value)
    {
        if(isEmpty())
        {
            return false;
        }
        value = m_data[m_front];
        m_front = (m_front + 1) % m_size;
        return true;
    }
 
    inline unsigned int front()const
    {
        return m_front;
    }
 
    inline unsigned int rear()const
    {
        return m_rear;
    }
 
    inline unsigned int size()const 
    {
        return m_size;
    }
private:
    unsigned int m_size;// 队列长度
    int m_front;// 队列头部索引
    int m_rear;// 队列尾部索引
    T* m_data;// 数据缓冲区
};*/