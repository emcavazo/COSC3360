/*
 Goal: we will take in two threads: threadStr, cmpStr, respectively.
 We will compare threadStr to cmpStr whether each character exist in cmpStr, 
    if so, print it, if not print * instead.
 Make one thread per character in threadStr
 */




#include <pthread.h>
#include <iostream>
#include <string>
#include <semaphore.h>

using namespace std;

struct InfoFromMain{
    int threadID;
    char currLetter;
    string cmpStr;
};

static pthread_mutex_t bsem; // Mutex sempahore
static pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER; //Condition variable to control the turn
static int turn = 0 ;

void *compareString(void *void_ptr){
    //mutex sempahores
        pthread_mutex_lock(&bsem); // Lock the mutex
            struct InfoFromMain *info = static_cast<struct InfoFromMain *>(void_ptr);
            while(turn!=info->threadID){
                pthread_cond_wait(&waitTurn, &bsem); // Wait for the turn to come
            }
            bool found = false;
            for(int i = 0; i < info->cmpStr.length(); i++){
                if(info->currLetter == info->cmpStr[i]){
                    found = true;
                    cout << info->currLetter;
                }
            }
            if(!found){
                cout << "*";
            }
            turn++; // Move to the next turn
            pthread_cond_broadcast(&waitTurn); // Notify all waiting threads that the turn has changed
        pthread_mutex_unlock(&bsem); // Unlock the mutex

        
    return nullptr;
}

int main(){
    std::string threadStr, cmpStr;
    std::cin >> threadStr >> cmpStr;

    int nThreads = threadStr.size(); // initalzie th enumber of threads variable;
    pthread_t *tid = new pthread_t[nThreads];
    InfoFromMain *arg = new InfoFromMain[nThreads];
    
    pthread_mutex_init(&bsem, nullptr); // initalizing bsem

    for(int i = 0; i < nThreads; i++){
        //instantiation
        arg[i].threadID = i; //determine what turn order the individual thread is at
        arg[i].currLetter = threadStr[i];
        arg[i].cmpStr = cmpStr; // used to compare whether the current letter exists in cmpStr

    }

    //pthread create
    for(int i = 0; i < nThreads; i++){
        if(pthread_create(&tid[i], nullptr, compareString, &arg[i]) != 0){
            cerr << "Failed to create thread " << i << endl;
        } else {
            cerr << "Created thread " << i << endl;
        }
    }

    for(int i = 0; i < nThreads; i++){
        pthread_join(tid[i], nullptr);
    }

    if(arg!=nullptr)
        delete[] arg;
    if(tid!=nullptr)
        delete[] tid;
    
    return 0;
}
