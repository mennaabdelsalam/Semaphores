#include <iostream>
#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <mutex>

using namespace
std;
#define ll long long
const int mod = 1e9 + 7;
const int N = 2e5 + 5;

#define MaxItems 10
#define BufferSize 15 //15 slots

//We use two binary semaphores empty and full , empty will manage access to data and full will manage access to buffer


sem_t empty;
sem_t full;
sem_t check;
int in = 0;//index monitor
int out = 0;//index collector    w eletnen malnash da3wa beehom fel awel
queue<int> buffer[BufferSize];
pthread_mutex_t mutex1;
int mails = 0; // shared resource between mcounter and monitor
// Semaphore declaration
int item;
int data;

struct num {
    int i;
};


//counter receives his own number through the struct sent
void *counterfun(void *number) {
    num *number_ = (num *) number;
    int k = number_->i;
    while (1) {
        int random_time = rand() % 5; // random time 0-5 seconds
        sleep(random_time);
        cout << "counter thread " << k << ": received a message"
             << endl; //when he wakes up this corresponds to receiving a message and we prompt the use
        int value;
        sem_getvalue(&full,
                     &value); //Next we get the value of the semaphore if it happens to be 0 then this means that this   thread has to wait and we prompt the user
        if (value == 0)
            cout << "counter thread " << k << ": waiting to write" << endl;
        /*Then each counter tries to access the data variable to increment it , the first thread to reach
       semwait() would cause the value of full = 0 preventing any other threads from accessing data ,
      then the thread who gained access increments data , call sem_post() to increment full so that
      other semaphores can access data and the cycle repeats*/
        sem_wait(&full);
        data += 1;
        cout << "counter thread " << k << ": now adding to counter current value " << data << endl;
        sem_post(&full);
    }

}

/*Now the monitor wants to write messages into the buffer , the same methods used for
accessing data is used to access buffer but with semaphore full , using a different semaphore
allows data and buffer to be accessed independently which should be the case , once mmonitor
accesses the buffer it writes total messages , total is the variable we used to store the total
number of messages , if the buffer reaches its maximum capacity , mmonitor discards remaining
messages and prompts the user*/

// Producer Section /* fetch numIters items from the buffer and sum them */
void *produce(void *arg) {
    while (1) {
        item = data;
        ll randomnumb = rand() % 100 * 0.01; /*to test*/
        int checkempty;
        sleep(1);
        sem_getvalue(&check, &checkempty);
        if (checkempty == 0) cout << "Monitor thread: waiting to read mails" << endl;
        sem_wait(&empty); //wait until empty>0 ,decrement empty
        cout << "Monitor thread: reading a count value of " << item << endl;//mails
        data = 0;
        pthread_mutex_lock(&mutex1); //wait mutex ----- we could replace it with sem_post/sem_wait 
        //now we will add items to the buffer
        if (in < BufferSize) {
            buffer->push(item);
            in = (in + 1) % BufferSize;
            cout << "Monitor thread: writing to buffer at position " << in << endl;
        }
        pthread_mutex_unlock(&mutex1); //release the lock
        if (BufferSize == in + 1) {
            cout << "Monitor thread: Buffer full!!!!!!!!!!!!!!!!!!!" << endl;
        }
        //another method the check full buffer
        /*
         sem_getvalue(&empty,&chckemty);
         if(sem_getvalue(&empty,&chckemty<=0)cout<<"BUFFER FULL !!!!!!!!!!"<<endl;*/
        sem_post(&full);
        sleep(rand() % 100 * 0.01);
    }
}

// Consumer Section
void *consume(void *arg) {
    while (1) {
        ll randomnumb = rand() % 100 * 0.01;/*i wrote it for testing */
        sleep(1);
        //------------sleep---------//
        //------------now we will read from buffer---------mcollector sleeps for a constant time t1 (here t1 = 1   sec) ,       when he wakes up this corresponds to the time to read the messages found in the buffer*/
        sem_wait(&full);


        //now we read buffer Then each mcollector tries to access the buffer variable , when mcollector gains access it    reads each message stored in the buffer and empties the buffer so further messages can be added , if the buffer is empty the user is prompted*/
        pthread_mutex_lock(&mutex1);
        if (!buffer->empty()) {  /* setting condition not to get a 1- segmentation falt 2-not to read from -1*/
            buffer->pop();
            out = (out + 1) % BufferSize;
            cout << "Collector thread: reading from the buffer at position" << out << endl;
            pthread_mutex_unlock(&mutex1);
            --mails;//just testing sth
        } else {
            cout << "Collector thread: nothing is in the buffer!" << endl;
        }
        sem_post(&empty);
        sleep(randomnumb);/*optional*/
    }
}

int main(int argv, char *argc[]) {

    int N = BufferSize;//or any other number

    // Declaration of attribute......
    // pthread_attr_t initialization
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


    pthread_mutex_init(&mutex1, NULL);

    // semaphore initialization
    sem_init(&empty, 0, BufferSize);
    sem_init(&full, 0, 0);


    pthread_t Mmonitor, Mcollecotor, counter[BufferSize];
    // Creation of collector
    pthread_create(&Mcollecotor, &attr,
                   consume, &N);
    // Creation of Monitor
    pthread_create(&Mmonitor, &attr,
                   produce, &N);
    // create N mcounter threads
    for (int i = 0; i < N; i++) {
        struct num *number = (struct num *) malloc(sizeof(struct num));
        number->i = i + 1;
        pthread_create(&counter[i], &attr,
                       counterfun, number);
    }


    pthread_attr_destroy(&attr);

    // Joining all threads
    pthread_join(Mmonitor, NULL);
    pthread_join(Mcollecotor, NULL);
    for (int i = 0; i < N; i++) {
        pthread_join(counter[i], NULL);
    }

    pthread_mutex_destroy(&mutex1); //we stop the program manually so it's usless here 
    sem_destroy(&empty);
    sem_destroy(&full);

    pthread_exit(NULL);

    return 0;
}
