#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "RingBuffer.h"


class Test {
public:
    Test(int id_ = 0, int value_ = 0) {
        id = id_;
        value = value_;
        sprintf(name, "id = %d, value = %d", id, value);
    }

    void display() {
        printf("%s\n", name);
    }

private:
    int id;
    int value;
    char name[128];

};


double gettimeofday_s(struct timeval* start, struct timeval* end) {
    return (end->tv_sec - start->tv_sec) + (end->tv_usec - start->tv_usec) * 1.0 / 1000000;
}

RingBuffer<Test> ring_buffer(1 << 12);

#define N (10 * (1 << 20))

void produce() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    long long i = 0;
    while(i < N) {
        if(ring_buffer.enqueue(Test(i % 1024, i))) {
            i++;
        }
    }

    gettimeofday(&end, NULL);
    printf("producer tid = %lu time: %0.2f s\n", pthread_self(), gettimeofday_s(&start, &end));
}

void consume() {
    sleep(1);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    Test t;
    long long i = 0;
    while(i < N) {
        if(ring_buffer.dequeue(t)) {
            //t.display();
            i++;
        }
    }

    gettimeofday(&end, NULL);
    printf("consumer tid = %lu time: %0.2f s\n", pthread_self(), gettimeofday_s(&start, &end));
}

int main() {
    std::thread producer(produce);
    std::thread consumer(consume);

    producer.join();
    consumer.join();

    printf("main thread exit\n");
    return 0;
}


/*#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "RingBuffer.h"
 
 
class Test
{
public:
   Test(int id = 0, int value = 0)
   {
        this->id = id;
        this->value = value;
        sprintf(data, "id = %d, value = %d\n", this->id, this->value);
   }
 
   void display()
   {
      printf("%s", data);
   }
private:
   int id;
   int value;
   char data[128];
};
 
double getdetlatimeofday(struct timeval *begin, struct timeval *end)
{
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
           (begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}
 
RingBuffer<Test> queue(1 << 12);//2u000
 
#define N (10 * (1 << 20))
 
void produce()
{
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    unsigned int i = 0;
    while(i < N)
    {
        if(queue.push(Test(i % 1024, i)))
        {
           i++;
        }
    }
 
    gettimeofday(&end, NULL);
    double tm = getdetlatimeofday(&begin, &end);
    printf("producer tid=%lu %f MB/s %f msg/s elapsed= %f size= %u\n", pthread_self(), N * sizeof(Test) * 1.0 / (tm * 1024 * 1024), N * 1.0 / tm, tm, i);
}
 
void consume()
{
    sleep(1);
    Test test;
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    unsigned int i = 0;
    while(i < N)
    {
        if(queue.pop(test))
        {
           // test.display();
           i++;
        }
    }
    gettimeofday(&end, NULL);
    double tm = getdetlatimeofday(&begin, &end);
    printf("consumer tid=%lu %f MB/s %f msg/s elapsed= %f, size=%u \n", pthread_self(), N * sizeof(Test) * 1.0 / (tm * 1024 * 1024), N * 1.0 / tm, tm, i);
}
 
int main(int argc, char const *argv[])
{
    std::thread producer1(produce);
    std::thread consumer(consume);
    producer1.join();
    consumer.join();
    return 0;
}*/