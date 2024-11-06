#include "LockFreeQueue.h"
#include <thread>



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

LockFreeQueue<Test> queue(1 << 20, "/shm");

#define N (1 << 20) 

void produce() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    long long i = 0;
    while(i < N) {
        if(queue.enqueue(Test(i >> 10, i))) {
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
        if(queue.dequeue(t)) {
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