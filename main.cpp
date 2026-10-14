#include <iostream>
#include <unistd.h>
#include <pthread.h> //for threads and semaphores
#include <semaphore.h>

using namespace std;

//each thread will print its number 5 times in this function
void* runner(void*);

//holds an array of semaphores on the heap
sem_t* runnersSync;

//data to pass into the thread function
struct RunnerInfo
{
    int myNumber; //number to print
    int totalRunners; //total number of runners

    RunnerInfo(int n, int t) {
        myNumber = n;
        totalRunners = t;
    }
};

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cout<<"Wrong number of command line arguments"<<endl;
        exit(0);
    }

    //get the total number of runners
    int numRunners = atoi(argv[1]);

    runnersSync = new sem_t[numRunners];
    sem_init(&runnersSync[0], 0, 1);
    for (int i = 1; i < numRunners; i++) {
        sem_init(&runnersSync[i], 0, 0);
    }

    //create the array of threads and start them in the runner function
    pthread_t* threads = new pthread_t[numRunners];
    for (int i = 0; i < numRunners; i++) {
        pthread_create(&threads[i], nullptr, runner, new RunnerInfo(i, numRunners));
    }

    for (int i = 0; i < numRunners; i++) {
        pthread_join(threads[i], 0);
    }

    delete[] runnersSync;
    delete[] threads;

    return 0;
}
//--
void* runner(void* p_data)
{
    RunnerInfo* p_runnerInfo = (RunnerInfo*)p_data;


    for (int i = 0; i < 5; i++) {
        sem_wait(&runnersSync[p_runnerInfo->myNumber]);
        string delim = p_runnerInfo->myNumber == p_runnerInfo->totalRunners - 1 ? "\n" : " -> ";
        cout << p_runnerInfo->myNumber << delim;
        sem_post(&runnersSync[(p_runnerInfo->myNumber + 1) % p_runnerInfo->totalRunners]);
    }

    delete p_runnerInfo;

    return 0;
}