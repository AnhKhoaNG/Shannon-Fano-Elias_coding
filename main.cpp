#include <iostream>
#include <pthread.h>
#include <vector>
#include <map>
#include <cmath>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
using namespace std;

struct Value {                           //Struct to store all the local variable
    pthread_mutex_t *print;              //mutex semaphore
    pthread_mutex_t *write;
    pthread_cond_t *cond;                //condition to control the turn 
    int turn;                           //Control the turn of threads
    int threadNum;                      //Control the print order
    string symbol;
    double prob;
    double fx;
}value;

void* cal_algo(void* struct_obj) {
    Value* obj = (Value*) struct_obj;
    double prob = obj->prob;
    string symbol = obj->symbol;
    obj->fx += obj->prob;                       
    double fx = obj->fx;
    int myTurn = obj->threadNum;
    double fbar_x;         
    double L_x;
    pthread_mutex_unlock(obj->write);                                //Local variables, create to store the information from struct
    
    fbar_x = fx - prob + 0.5 * prob;            
    L_x = ceil(log2(1/prob)+1);                                  
    string codex = "";

    while (L_x--)                                               
    {
        fbar_x *=2;
        int fractbit =fbar_x;
        if (fractbit ==1)
        {
            fbar_x -= fractbit;
            codex.push_back('1');    
        } else
        codex.push_back('0');  
    }
    pthread_mutex_lock(obj->print);                                //Mutex semaphore to control the run turn of thread
    while (myTurn != obj->turn){
        pthread_cond_wait(obj->cond,obj->print);                  //Condition variable to print in order
    }
    pthread_mutex_unlock(obj->print);
    
    
    std::cout << "Symbol " << symbol << ", Code: " <<codex << endl;        
    pthread_mutex_lock(obj->print);            //Mutex semaphore to secure print order
    obj->turn++;    
    pthread_cond_broadcast(obj->cond);//Broadcast for the condition that is waiting
    pthread_mutex_unlock(obj->print);
    return nullptr;
}


int main() {
    string input;
    getline(cin,input);                              //Read the input
    
    map <char, double> frequency;              //Using map to sort the symbol and calculate their frequencies
    for(const char &a: input)
        frequency[a]++;
    
    int size= frequency.size();                 //Number of symbol
    static pthread_mutex_t print;
    static pthread_mutex_t write;
    static pthread_cond_t cond;
    pthread_mutex_init(&print,NULL);             //Initilize the mutex
    pthread_mutex_init(&write,NULL); 
    cond = PTHREAD_COND_INITIALIZER;  
    value.print = &print;
    value.write = &write;
    value.cond = &cond;
    value.turn = 0; 
    
    vector <double> prob_store;                 //Create vectors to store the prob and symbol calculated from map
    vector <char> sym_store;
    for (auto &pair: frequency) {               //Store the symbol and the probability of each symbol in the vector of struct
        sym_store.push_back(pair.first);
        prob_store.push_back(pair.second/input.size());                                                     
    }
    cout << "SHANNON-FANO-ELIAS Codes:" <<endl <<endl;
   pthread_t* tid = new pthread_t[size];                           
    for(int i = 0; i < size; i++){
        pthread_mutex_lock(value.write);         //Mutex semaphore to secure the write to the data
        value.symbol = sym_store[i];              //Assign each thread only the data that it only works with
        value.prob = prob_store[i];
        value.threadNum = i;
        pthread_create(&tid[i], nullptr, &cal_algo,  &value);
    }
    for(int i = 0; i < size; i++)
        pthread_join(tid[i], nullptr);

    delete[] tid;
    return 0;
}