/*
Goal:
Given an input integer, print in order whether each digit of the input is even or odd

e.g. 
1234

Example output:

1: Odd
2: Even
3: Odd
4. Even


*/
#include <pthread.h>
#include <iostream>
#include <string>
#include <semaphore.h>

struct InfoFromMain{
    int threadID; // whose turn is it?
    int digit;
};

static pthread_mutex_t bsem; // Mutex sempahore
static pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER;
static int turn = 0; determines which thread goes first;

void *printDigit(void * void_ptr){
    //do work

    pthread_mutex_lock(&bsem)
        InfoFromMain* data = static_cast<InfoFromMain*> (void_ptr);
        while(data->threadID != turn){
            pthread_cond_wait(&waitTurn, &bsem);
        }
    pthread_mutex_unlock(&bsem);

    if(data->threadID % 2 == 0){
        std::cout << data->threadID << ": Even" << std::endl;
    }
    else{
        std::cout << data->threadID << ": Odd" << std::endl;
    }


    pthread_mutex_lock(&bsem);
        turn++;
        pthread_cond_broadcast(&waitTurn);

    pthread_mutex_unlock(&bsem);

    return nullptr;
}


int main(){
    std::string input;
    std::cin >> input;

    int nThreads= input.length();
    pthread_t *tid = new pthread_t[nThreads];
    InfoFromMain *arg = new InfoFromMain[nThreads];

    pthread_mutex_init(&bsem, nullptr); // Initializing a binary sempahore

    for(int i = 0; i < nThreads; i++){
        //instantiate the argument for the thread
        infoFromMain info;
        info-> threadID = i; // initalize thread ID
        info->digit = input[i]; // initialize digit number - 1 (in the example of 1234)
        arg[i] = *info;

        pthread_create(&tid[i],nullptr, InfoFromMain, &arg[i]);
    }

    for(int i = 0; i < nThreads; i++){
        pthread_join(tid[i], nullptr);
    }
}