#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <iostream>
#include <stdexcept> // Include for try-block statement
#include <climits>   // Include for INT_MAX
#include <algorithm>
#include <array>
using namespace std;

// initialize semophores
sem_t empty_milk;
sem_t empty_cheese;
sem_t mutex_milk;
sem_t mutex_cheese;
sem_t full_milk;
sem_t full_cheese;

const int mMAX = 9; // number of entries in the milk buffer
const int chMAX = 4; // number of entries in the cheese buffer

array<int, mMAX> buffer_milk = {0}; // milk buffer initialized to 0 for all entries
array<int, chMAX> buffer_cheese = {0}; // cheese buffer initialized to 0 for all entries

int milk_write_idx = 0; // write index for the milk buffer
int milk_read_idx = 0; // read index for the milk buffer
int cheese_write_idx = 0; // write index for the cheese buffer
int cheese_read_idx = 0; // read index for the cheese buffer


int milk = 0, cheese = 0, cheeseburgers = 0; // current count of milk, cheese and cheeseburgers

// a structure for thread arguments to pass into the thread runner function
struct thread_args
{
    int pid;    // producer id
    int total;  // number of items to produce
};

/********************************************************************
*** FUNCTION milk_producer                                        ***
*********************************************************************
*** DESCRIPTION : Runner function that produces milk to the milk  ***
*** buffer.                                                       ***
*********************************************************************
*** INPUT ARGS  : args                                            ***
*** OUTPUT ARGS :                                                 ***
*** IN/OUT ARGS : milk, milk_write_idx                            ***
*** RETURN      :                                                 ***
********************************************************************/

void *milk_producer(void* args)
{
    // cast arguments struct to struct pointer
    thread_args *milk_args = new thread_args;
    *milk_args = *(thread_args *)args;

    // loop until the milk count meets the total milk needed
    for(int i = 0; i < milk_args->total; i++)
    {
        // if there isn't an available space in the milk buffer to write to, wait
        sem_wait(&empty_milk);
        // if another thread is accessing the milk buffer, wait
        sem_wait(&mutex_milk);
        //critical section
        // if so, assign the thread's producer id to the milk buffer at the writing index
        // thereby producing milk to that spot
        buffer_milk[milk_write_idx] = milk_args->pid;
        // increment the milk counter
        milk++;
        // increment the milk buffer writing index
        milk_write_idx++;
        // if the index goes beyond the bounds of the array, loop it back to the beginning
        if (milk_write_idx == mMAX)
        {
            milk_write_idx %= mMAX;
        }
        // unlock the milk mutex semaphore
        sem_post(&mutex_milk);
        // if there are enough milk bottles to produce cheese, signal the full milk semaphore
        if (milk % 3 == 0)
        {
            sem_post(&full_milk);
        }      
       
    }
    // delete struct pointer
    delete milk_args;
}

/********************************************************************
*** FUNCTION cheese_producer                                      ***
*********************************************************************
*** DESCRIPTION : Runner function that consumes 3 milk bottles    ***
*** from the milk buffer and produces cheese to the cheese buffer.***
*********************************************************************
*** INPUT ARGS  : args                                            ***
*** OUTPUT ARGS :                                                 ***
*** IN/OUT ARGS : cheese, cheese_write_idx, milk_read_idx         ***
*** RETURN      :                                                 ***
********************************************************************/

void *cheese_producer(void* args)
{
    int mid[3] = {0}; // an array of milk producer id's, initialized to 0
    int cheese_id; // the cheese producer id to be sent to the cheese buffer

    // cast arguments struct to struct pointer
    thread_args *cheese_args = new thread_args; 
    *cheese_args = *(thread_args *)args;

    // loop through program if the amount of cheese produced is less than what is needed
    for(int j = 0; j < cheese_args->total; j++)
    {
        // if there aren't enough milk bottles in the milk buffer to read, wait
        sem_wait(&full_milk);
        // if another thread is accessing the milk buffer, wait
        sem_wait(&mutex_milk);
        // critical section
        // take the next three entries in the milk buffer
        for (int i = 0; i < 3; i++)
        {
            // add the producer id's from each milk buffer entry to the milk id array
            mid[i] = buffer_milk[milk_read_idx];
            // increment the milk read index
            milk_read_idx++;
            // if the milk read index exceeds the bounds of the array, loop back to the beginning
            if (milk_read_idx == mMAX)
            {
                milk_read_idx %= mMAX;
            }
        }
        // return the milk mutex lock
        sem_post(&mutex_milk);
        // signal the empty milk semaphor 3 times to account for the 3 milk bottles consumed
        for (int i = 0; i < 3; i++)
            sem_post(&empty_milk);

        // assemble cheese id from milk id array and the current thread's producer id
        cheese_id = mid[0]*1000 + mid[1]*100 + mid[2]*10 + cheese_args->pid;
        // if the cheese count hit the total while the thread was waiting for the mutex lock
        // reset the mutex and full milk semaphores
        
        // if there isn't an available space in the cheese buffer to write to, wait
        sem_wait(&empty_cheese);
        // if another thread is accessing the cheese buffer, wait
        sem_wait(&mutex_cheese);
        // critical section
        // write the cheese id to the cheese buffer at the writing index
        buffer_cheese[cheese_write_idx] = cheese_id;
        // increment the cheese counter
        cheese++;
        // increment the cheese buffer writing index
        cheese_write_idx++;
        // if the writing index exceeds the bounds of the array, loop back to the beginning
        if (cheese_write_idx == chMAX)
        {
            cheese_write_idx %= chMAX;
        }
        // release the cheese mutex lock
        sem_post(&mutex_cheese);
        // if there is enough cheese to make a cheeseburger, signal the full cheese semaphore
        if (cheese % 2 == 0)
        {
            sem_post(&full_cheese);
        }
    }
    // delete pointer
    delete cheese_args;
}

/********************************************************************
*** FUNCTION cheeseburger_producer                                ***
*********************************************************************
*** DESCRIPTION : Runner function that consumes 2 cheese slices   ***
*** from the cheese buffer, produces a cheeseburger, and outputs  ***
*** the cheeseburger id to the screen.                            ***
*********************************************************************
*** INPUT ARGS  : arg                                             ***
*** OUTPUT ARGS :                                                 ***
*** IN/OUT ARGS : cheeseburgers, cheese_read_idx                  ***
*** RETURN      :                                                 ***
********************************************************************/

void *cheeseburger_producer(void* arg)
{
    int chid[2] = {0}; // array that contains the cheese ids
    int burger_id; // the complete burger id

    // loop until the amount of cheeseburgers reaches the total needed
    while (cheeseburgers < *(int*)arg)
    {
        // if there is not enough cheese to make a cheeseburger, wait
        sem_wait(&full_cheese);
        // if another thread is accessing the cheese buffer, wait
        sem_wait(&mutex_cheese);
        //critical section
        // take the next two entries in the cheese buffer
        for (int i = 0; i < 2; i++)
        {
            // assign the cheese id at the cheese read index to the cheese id array 
            chid[i] = buffer_cheese[cheese_read_idx];
            // increment the cheese read index
            cheese_read_idx++;
            // if the read index goes over the bounds of the array, loop back to the beginning
            if (cheese_read_idx == chMAX)
            {
                cheese_read_idx %= chMAX;
            }
        }
        // release cheese mutex lock
        sem_post(&mutex_cheese);
        // signal the empty milk semaphor 2 times to account for the 2 cheese slices consumed
        for (int i = 0; i < 2; i++)
            sem_post(&empty_cheese);
        // assemble burger id from cheese id's
        burger_id = chid[0]*10000 + chid[1];
        // increment cheeseburger counter
        cheeseburgers++;
        // output cheeseburger id to the screen
        cout << burger_id << endl;

    }
}

/********************************************************************
*** FUNCTION removeWhiteSpace                                     ***
*********************************************************************
*** DESCRIPTION : Removes any character that counts as white      ***
*** space.                                                        ***
*********************************************************************
*** INPUT ARGS  :                                                 ***
*** OUTPUT ARGS :                                                 ***
*** IN/OUT ARGS : str                                             ***
*** RETURN      : string str                                      ***
********************************************************************/

string removeWhiteSpace(string str)
{
    // remove all of the spaces
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
     // remove any carriage returns
    str.erase(remove(str.begin(), str.end(), '\r'), str.end());
    // remove any tabs
    str.erase(remove(str.begin(), str.end(), '\t'), str.end());
    // remove any newline characters
    str.erase(remove(str.begin(), str.end(), '\n'), str.end());
    // remove any vertical tabs
    str.erase(remove(str.begin(), str.end(), '\v'), str.end());
    // remove any form feed characters
    str.erase(remove(str.begin(), str.end(), '\f'), str.end());
    return str;
}

/********************************************************************
*** FUNCTION intConvert                                           ***
*********************************************************************
*** DESCRIPTION : Translate the value string to an integer.       ***
*********************************************************************
*** INPUT ARGS  : str                                             ***
*** OUTPUT ARGS : int value                                       ***
*** IN/OUT ARGS :                                                 ***
*** RETURN      : int value or int representing translation error ***
********************************************************************/

int intConvert(string str)
{
    // attempt to convert the value string to an int
    try
    {
        int i = stoi(str);
    }
    // if an exception is found, display error message
    catch (invalid_argument)
    {
       cout << "Please enter a whole number greater than 0." << endl;
        return INT_MAX;
    }
    catch (out_of_range)
    {
        cout << "Out of Range error while converting " << str << " to integer." << endl;
        return INT_MAX;
    }
    catch (exception)
    {
        cout << "Exception error while converting " << str << " to integer." << endl;
        return INT_MAX;
    }
    // convert the string to an int
    return stoi(str);
}

int main()
{
    // initialize semaphores with their values
    sem_init(&empty_milk, 0, mMAX); // a semphore that will block access if the milk buffer is full
    sem_init(&empty_cheese, 0, chMAX); // a semphore that will block access if the cheese buffer is full
    sem_init(&mutex_milk, 0, 1); // mutex lock for accessing the milk buffer
    sem_init(&mutex_cheese, 0, 1); // mutex lock for accessing the cheese buffer
    sem_init(&full_milk, 0, 0); // a semphore that will block access if the milk buffer doesn't have enough entries for the consumer
    sem_init(&full_cheese, 0, 0); // a semphore that will block access if the cheese buffer doesn't have enough entries for the consumer

    // initialize threads
    pthread_t t_m1, t_m2, t_m3;
    pthread_t t_ch1, t_ch2;
    pthread_t t_chb;

    string cheeseburgers_string; // a string of the input
    int total_cheeseburgers; // total cheeseburgers

    while(true)
    {
        // write prompt for input
        cout << "How many cheeseburgers do you want to produce? Enter 'exit' to quit program." << endl;
        // read in string
        cin >> cheeseburgers_string;
        // remove white space
        cheeseburgers_string = removeWhiteSpace(cheeseburgers_string);
        // check to see if 'exit' was written
        if (cheeseburgers_string == "exit")
        {
            // if so, exit
            break;
        }
        // check to see if the input string is a positive integer
        else if ((total_cheeseburgers = intConvert(cheeseburgers_string)) != INT_MAX)
        {
            if(total_cheeseburgers > 0)
            {
                // if so, find the total amount of cheese and milk required
                int total_cheese = total_cheeseburgers;
                int total_milk = 2 * total_cheese;

                // initialize struct of runner function arguments with producer ids and total amount needed
                thread_args args[5] = {{1, total_milk},{2, total_milk},{3, total_milk},{4, total_cheese},{5, total_cheese}};

                // create 3 milk producer threads, 2 cheese producer threads, and 1 cheeseburger producer thread
                pthread_create(&t_m1, NULL, milk_producer, (void*)&args[0]);
                pthread_create(&t_m2, NULL, milk_producer, (void*)&args[1]);
                pthread_create(&t_m3, NULL, milk_producer, (void*)&args[2]);
                pthread_create(&t_ch1, NULL, cheese_producer, (void*)&args[3]);
                pthread_create(&t_ch2, NULL, cheese_producer, (void*)&args[4]);
                pthread_create(&t_chb, NULL, cheeseburger_producer, (void*)&total_cheeseburgers);

                // wait for threads to complete
                pthread_join(t_m1, NULL);
                pthread_join(t_m2, NULL);
                pthread_join(t_m3, NULL);
                pthread_join(t_ch1, NULL);
                pthread_join(t_ch2, NULL);
                pthread_join(t_chb, NULL);

                // wait for 1 second
                sleep(1);

                /*/ delete and reinitialize semaphores
                sem_destroy(&empty_milk);
                sem_destroy(&empty_cheese);
                sem_destroy(&mutex_milk);
                sem_destroy(&mutex_cheese);
                sem_destroy(&full_milk);
                sem_destroy(&full_cheese);

                sem_init(&empty_milk, 0, mMAX);
                sem_init(&empty_cheese, 0, chMAX);
                sem_init(&mutex_milk, 0, 1);
                sem_init(&mutex_cheese, 0, 1);
                sem_init(&full_milk, 0, 0);
                sem_init(&full_cheese, 0, 0);*/

                // reset milk and cheese buffers
                buffer_milk.fill(0);
                buffer_cheese.fill(0);
                
                // reset read and write indexes
                milk_write_idx = 0, milk_read_idx = 0, cheese_write_idx = 0, cheese_read_idx = 0;
                // reset milk, cheese, and cheeseburger counters
                milk = 0, cheese = 0, cheeseburgers = 0;
            }
            // if the input was not a positive integer, display error message
            else
            {
                cout << "Please enter a whole number greater than 0." << endl;
            }
        }
    }
    // delete semaphores
    sem_destroy(&empty_milk);
    sem_destroy(&empty_cheese);
    sem_destroy(&mutex_milk);
    sem_destroy(&mutex_cheese);
    sem_destroy(&full_milk);
    sem_destroy(&full_cheese);
    
    
    return 0;
    
}