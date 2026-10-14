#include <iostream>
#include <unistd.h>
#include <pthread.h> //for threads and semaphores
#include <semaphore.h>

using namespace std;

//producer and consumer thread functions
void* prod(void*);
void* cons(void*);

//buffer management functions
void append(int n);
int take();

//semaphores
sem_t not_full;
sem_t not_empty;
sem_t bufMutex;
sem_t printMutex;

//shared variables
//max number of Fibonacci numbers to produce and consume
const int MAX_FIBS = 30;

//used to represent the nth Fibonacci number to be produced
int nthFibIndex = 0;

//holds the number of fibs consumed by a consumer
int consumed = 0;

//circular buffer and indices
int* p_buffer;
int in = 0;
int out = -1;
int bufferSize;

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cout<<"Wrong number of command line arguments"<<endl;
        exit(0);
    }
    //read the command line arguments and turn them into integers
    int numProducers = atoi(argv[1]);
    int numConsumers = atoi(argv[2]);
    bufferSize = atoi(argv[3]);
    
    //create the thread objects
    pthread_t* p_producers = new pthread_t[numProducers];
    pthread_t* p_consumers = new pthread_t[numConsumers];
    
    //create the circular buffer
    p_buffer = new int[bufferSize];
    
    // TODO: Initialize semaphores with appropriate values
    // Hint: What should not_full start at? What about not_empty?
    sem_init(&not_full, 0, bufferSize);
    sem_init(&not_empty, 0, 0);
    sem_init(&bufMutex, 0, 1);
    sem_init(&printMutex, 0, 1);

    for (int i = 0; i < numProducers; i++) {
        pthread_create(p_producers + i, 0, prod, 0);
    }
    for (int i = 0; i < numConsumers; i++) {
        pthread_create(p_consumers + i, 0, cons, 0);
    }

    for (int i = 0; i < numProducers; i++) {
        pthread_join(p_producers[i], 0);
    }
    for (int i = 0; i < numConsumers; i++) {
        pthread_join(p_consumers[i], 0);
    }

    //free up the memory previously allocated
    delete[] p_producers;
    delete[] p_consumers;
    delete[] p_buffer;

    return 0;
}
//--
void* prod(void*)
{
    while (true) {
        sem_wait(&bufMutex);
        if (nthFibIndex >= MAX_FIBS) {
            sem_post(&bufMutex);
            break; 
        }

        int currentFib = nthFibIndex;
        
        nthFibIndex++; 
        sem_post(&bufMutex);

        append(currentFib);
    }
    return 0;
}
//--
void* cons(void*)
{
    while (true) {
        sem_wait(&printMutex);
        if (consumed >= MAX_FIBS) {
            sem_post(&printMutex);
            break; 
        }
        consumed++;
        sem_post(&printMutex);

        take();
    }
    return 0;
}
//--
void append(int n)
{
    int f = 0, s = 1;
    // s (second) is functioning as return value
    if (n == 0) {
        s = f;
    }
    for (int i = 2; i <= n; i++) {
        int temp = s;
        s = f + s;
        f = temp;
    }
    
    sem_wait(&not_full);
    sem_wait(&bufMutex);
    p_buffer[in] = s;
    cout << "Producing: " << s << endl;
    in++;
    in %= bufferSize;
    sem_post(&not_empty);
    sem_post(&bufMutex);
}
//--
int take()
{
    int val = 0;
    sem_wait(&not_empty);
    sem_wait(&printMutex);
    out++;
    out %= bufferSize;
    val = p_buffer[out];
    cout << "\tConsuming: " << val << endl;
    sem_post(&not_full);
    sem_post(&printMutex);

    return val;
}