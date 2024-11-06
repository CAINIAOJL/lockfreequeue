#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#define SHM_NAME_LEN 128
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define IS_POT(x) ((x) && !((x) & ((x)-1)))
#define MEMORY_BARRIER __sync_synchronize()

template<class T>
class LockFreeQueue {
protected:
    typedef struct {
        int m_lock;
        inline void spinlock_init() {
            m_lock = 0;
        }

        inline void spinlock_lock() {
            while(! __sync_bool_compare_and_swap(&m_lock, 0, 1)) {

            }
        }

        inline void spinlock_unlock() {
            __sync_lock_release(&m_lock);
        }
    } spinlock_t;

public:
    LockFreeQueue(unsigned int sz, const char* name_ = NULL) {
        memset(shm_name, 0, sizeof(shm_name));
        createQueue(name_, sz);
    }

    ~LockFreeQueue() {
        if(shm_name[0] == 0) {
            delete [] m_buffer;
            m_buffer = NULL; // 防止野指针
        } else {
            if(munmap(m_buffer, m_size * sizeof(T)) == -1) {
                perror("munmap");
            }
            if(shm_unlink(shm_name) == -1) {
                perror("shm_unlink");
            }
        }
    }
    
    bool isFull() const {
#ifdef USE_POT
        return m_head == (m_rear + 1) & (m_size - 1);
#else 
        return m_head == (m_rear + 1) % m_size;
#endif
    }

    bool isEmpty() const {
        return m_head == m_rear;
    }

    unsigned int front()const
    {
        return m_head;
    }
 
    unsigned int rear()const
    {
        return m_rear;
    }

    bool enqueue(const T& data) {
#ifdef USE_LOCK
        m_spinlock.spinlock_lock();
#endif
        if(isFull()) {
#ifdef USE_LOCK
            m_spinlock.spinlock_unlock();
#endif
            return false;
        }
        memcpy(&m_buffer[m_rear], &data, sizeof(T));
#ifdef USE_MB
        MEMORY_BARRIER;
#endif

#ifdef USE_POT
        m_rear = (m_rear + 1) & (m_size - 1);
#else
        m_rear = (m_rear + 1) % m_size;
#endif

#ifdef USE_LOCK
        m_spinlock.spinlock_unlock();
#endif
        return true;
    }

    bool dequeue(T& data) {
#ifdef USE_LOCK
        m_spinlock.spinlock_lock();
#endif
        if(isEmpty()) {
#ifdef USE_LOCK
            m_spinlock.spinlock_unlock();
#endif
            return false;
        }
        data = m_buffer[m_head];
#ifdef USE_MB
        MEMORY_BARRIER;
#endif

#ifdef USE_POT
        m_head = (m_head + 1) & (m_size - 1);
#else
        m_head = (m_head + 1) % m_size;
#endif

#ifdef USE_LOCK
        m_spinlock.spinlock_unlock();
#endif
        return true;
    }

protected:
    inline unsigned int roundup_pow_of_two(size_t size) {
        size |= size >> 1;
        size |= size >> 2;
        size |= size >> 4;
        size |= size >> 8;
        size |= size >> 16;
        size |= size >> 32;

        return size + 1;
    }
    
    virtual void createQueue(const char* name, unsigned int size) {
#ifdef USE_POT
        if(!IS_POT(size)) {
            size = roundup_pow_of_two(size);
        }
#endif
        m_size = size;
        m_head = m_rear = 0;
        if(name == NULL) {
            m_buffer = new T[m_size];
        } else {
            //创建共享内存
            int shm_fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if(shm_fd == -1) {
                perror("shm_open");
            }
            //设置共享内存大小
            if(ftruncate(shm_fd, m_size * sizeof(T)) < 0) {
                perror("ftruncate");
                close(shm_fd);
            }
            //创建映射
            void* addr = mmap(0, m_size * sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if(addr == MAP_FAILED) {
                perror("mmap");
                close(shm_fd);
            }
            //映射完成后，关闭共享内存文件
            if(close(shm_fd) == -1) {
                perror("close");
            }

            m_buffer = static_cast<T*>(addr);
            memcpy(shm_name, name, SHM_NAME_LEN - 1);
        }
#ifdef USE_LOCK
    spinlock_init(m_lock);
#endif
    }


protected:
    char shm_name[SHM_NAME_LEN]; //hared memorry name
    volatile unsigned int m_head;
    volatile unsigned int m_rear;
    unsigned int m_size;

#ifdef USE_LOCK
    spinlock_t m_spinlock;
#endif
    T* m_buffer;
};

#define USE_LOCK //开启spinlock锁，多生产者多消费者场景
#define USE_POT  //开启队列大小的2的幂对齐
#define USE_MB   //开启Memory Barrier


/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
 
#define SHM_NAME_LEN 128
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define IS_POT(x) ((x) && !((x) & ((x)-1)))
#define MEMORY_BARRIER __sync_synchronize()
 
template <class T>
class LockFreeQueue
{
protected:
    typedef struct
    {
        int m_lock;
        inline void spinlock_init()
        {
            m_lock = 0;
        }
 
        inline void spinlock_lock()
        {
            while(!__sync_bool_compare_and_swap(&m_lock, 0, 1)) {}
        }
 
        inline void spinlock_unlock()
        {
            __sync_lock_release(&m_lock);
        }
    } spinlock_t;
 
public:
    // size:队列大小
    // name:共享内存key的路径名称，默认为NULL，使用数组作为底层缓冲区。
    LockFreeQueue(unsigned int size, const char* name = NULL)
    {
        memset(shm_name, 0, sizeof(shm_name));
        createQueue(name, size);
    }
 
    ~LockFreeQueue()
    {
        if(shm_name[0] == 0)
        {
            delete [] m_buffer;
            m_buffer = NULL;
        }
        else
        {
            if (munmap(m_buffer, m_size * sizeof(T)) == -1) {
                perror("munmap");
            }
            if (shm_unlink(shm_name) == -1) {
                perror("shm_unlink");
            }
        }
    }
 
    bool isFull()const
    {
#ifdef USE_POT
        return m_head == (m_tail + 1) & (m_size - 1);
#else
        return m_head == (m_tail + 1) % m_size;
#endif
    }
 
    bool isEmpty()const
    {
        return m_head == m_tail;
    }
 
    unsigned int front()const
    {
        return m_head;
    }
 
    unsigned int tail()const
    {
        return m_tail;
    }
 
    bool push(const T& value)
    {
#ifdef USE_LOCK
        m_spinLock.spinlock_lock();
#endif
        if(isFull())
        {
#ifdef USE_LOCK
            m_spinLock.spinlock_unlock();
#endif
            return false;
        }
        memcpy(m_buffer + m_tail, &value, sizeof(T));
#ifdef USE_MB
        MEMORY_BARRIER;
#endif
 
#ifdef USE_POT
        m_tail = (m_tail + 1) & (m_size - 1);
#else
        m_tail = (m_tail + 1) % m_size;
#endif
 
#ifdef USE_LOCK
        m_spinLock.spinlock_unlock();
#endif
        return true;
    }
 
    bool pop(T& value)
    {
#ifdef USE_LOCK
        m_spinLock.spinlock_lock();
#endif
        if (isEmpty())
        {
#ifdef USE_LOCK
            m_spinLock.spinlock_unlock();
#endif
            return false;
        }
        memcpy(&value, m_buffer + m_head, sizeof(T));
#ifdef USE_MB
        MEMORY_BARRIER;
#endif
 
#ifdef USE_POT
        m_head = (m_head + 1) & (m_size - 1);
#else
        m_head = (m_head + 1) % m_size;
#endif
 
#ifdef USE_LOCK
        m_spinLock.spinlock_unlock();
#endif
        return true;
    }
 
protected:
    virtual void createQueue(const char* name, unsigned int size)
    {
#ifdef USE_POT
        if (!IS_POT(size))
        {
            size = roundup_pow_of_two(size);
        }
#endif
        m_size = size;
        m_head = m_tail = 0;
        if(name == NULL)
        {
            m_buffer = new T[m_size];
        }
        else
        {
            int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
            if (shm_fd < 0)
            {
                perror("shm_open");
            }
 
            if (ftruncate(shm_fd, m_size * sizeof(T)) < 0)
            {
                perror("ftruncate");
                close(shm_fd);
            }
 
            void *addr = mmap(0, m_size * sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (addr == MAP_FAILED)
            {
                perror("mmap");
                close(shm_fd);
            }
            if (close(shm_fd) == -1)
            {
                perror("close");
                exit(1);
            }
 
            m_buffer = static_cast<T*>(addr);
            memcpy(shm_name, name, SHM_NAME_LEN - 1);
        }
#ifdef USE_LOCK
    spinlock_init(m_lock);
#endif
    }
    inline unsigned int roundup_pow_of_two(size_t size)
    {
        size |= size >> 1;
        size |= size >> 2;
        size |= size >> 4;
        size |= size >> 8;
        size |= size >> 16;
        size |= size >> 32;
        return size + 1;
    }
protected:
    char shm_name[SHM_NAME_LEN];
    volatile unsigned int m_head;
    volatile unsigned int m_tail;
    unsigned int m_size;
#ifdef USE_LOCK
    spinlock_t m_spinLock;
#endif
    T* m_buffer;
};*/