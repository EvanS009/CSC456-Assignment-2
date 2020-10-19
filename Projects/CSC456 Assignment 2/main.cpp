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


sem_t empty_milk;
sem_t empty_cheese;
sem_t mutex_milk;
sem_t mutex_cheese;
sem_t full_milk;
sem_t full_cheese;

const int mMAX = 9;
const int chMAX = 4;

int buffer_milk[mMAX] = {0};
int buffer_cheese[chMAX] = {0};

int milk_write_idx = 0;
int milk_read_idx = 0;
int cheese_write_idx = 0;
int cheese_read_idx = 0;


int milk = 0, cheese = 0, cheeseburgers = 0;

struct thread_args
{
    int pid;
    int total;
};

void *milk_producer(void* args)
{
    thread_args *margs = new thread_args;
    *margs = *(thread_args *)args;
    while (milk < margs->total)
    {
        sem_wait(&empty_milk);
        sem_wait(&mutex_milk);
        // critical section
        buffer_milk[milk_write_idx] = margs->pid;
        milk++;
        milk_write_idx++;
        if (milk_write_idx == mMAX)
        {
            milk_write_idx %= mMAX;
        }
        sem_post(&mutex_milk);
        if (milk % 3 == 0)
        {
            sem_post(&full_milk);
        }
        
    }
    delete margs;
}

void *cheese_producer(void* args)
{
    int mid[3] = {0};
    int cheese_id;
    thread_args *chargs = new thread_args;
    *chargs = *(thread_args *)args;

    while (cheese < chargs->total)
    {

        sem_wait(&full_milk);
        sem_wait(&mutex_milk);
        for (int i = 0; i < 3; i++)
        {
            mid[i] = buffer_milk[milk_read_idx];
            milk_read_idx++;
            if (milk_read_idx == mMAX)
            {
                milk_read_idx %= mMAX;
            }
        }
        sem_post(&mutex_milk);
        for (int i = 0; i < 3; i++)
            sem_post(&empty_milk);

        cheese_id = mid[0]*1000 + mid[1]*100 + mid[2]*10 + chargs->pid;

        sem_wait(&empty_cheese);
        sem_wait(&mutex_cheese);
        // critical section
        buffer_cheese[cheese_write_idx] = cheese_id;
        cheese++;
        cheese_write_idx++;
        if (cheese_write_idx == chMAX)
        {
            cheese_write_idx %= chMAX;
        }
        sem_post(&mutex_cheese);
        if (cheese % 2 == 0)
        {
            sem_post(&full_cheese);
        }
        
    }
    delete chargs;
}

void *cheeseburger_producer(void* arg)
{
    int chid[2] = {0};
    int burger_id;

    while (cheeseburgers < *(int*)arg)
    {
        sem_wait(&full_cheese);
        sem_wait(&mutex_cheese);
        //critical section
        for (int i = 0; i < 2; i++)
        {
            chid[i] = buffer_cheese[cheese_read_idx];
            cheese_read_idx++;
            if (cheese_read_idx == chMAX)
            {
                cheese_read_idx %= chMAX;
            }
        }
        sem_post(&mutex_cheese);
        for (int i = 0; i < 2; i++)
            sem_post(&empty_cheese);
        burger_id = chid[0]*10000 + chid[1];
        cheeseburgers++;
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
*** FUNCTION intConvert                                         ***
*********************************************************************
*** DESCRIPTION : Translate the value string to an integer.       ***
*********************************************************************
*** INPUT ARGS  : str                                        ***
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
    sem_init(&empty_milk, 0, mMAX);
    sem_init(&empty_cheese, 0, chMAX);
    sem_init(&mutex_milk, 0, 1);
    sem_init(&mutex_cheese, 0, 1);
    sem_init(&full_milk, 0, 0);
    sem_init(&full_cheese, 0, 0);

    pthread_t t_m1, t_m2, t_m3;
    pthread_t t_ch1, t_ch2;
    pthread_t t_chb;

    string cheeseburgers_string;
    int total_cheeseburgers;

    while(true)
    {
        cout << "How many cheeseburgers do you want to produce? Enter 'exit' to quit program." << endl;
        cin >> cheeseburgers_string;
        cheeseburgers_string = removeWhiteSpace(cheeseburgers_string);
        if (cheeseburgers_string == "exit")
        {
            break;
        }
        else if ((total_cheeseburgers = intConvert(cheeseburgers_string)) != INT_MAX)
        {
            if(total_cheeseburgers > 0)
            {
                int total_cheese = 2 * total_cheeseburgers;
                int total_milk = 3 * total_cheese;

                thread_args args[5] = {{1, total_milk},{2, total_milk},{3, total_milk},{4, total_cheese},{5, total_cheese}};

            

                pthread_create(&t_m1, NULL, milk_producer, (void*)&args[1]);
                pthread_create(&t_m2, NULL, milk_producer, (void*)&args[2]);
                pthread_create(&t_m3, NULL, milk_producer, (void*)&args[3]);
                pthread_create(&t_ch1, NULL, cheese_producer, (void*)&args[4]);
                pthread_create(&t_ch2, NULL, cheese_producer, (void*)&args[5]);
                pthread_create(&t_chb, NULL, cheeseburger_producer, (void*)&total_cheeseburgers);

                pthread_join(t_m1, NULL);
                pthread_join(t_m2, NULL);
                pthread_join(t_m3, NULL);
                pthread_join(t_ch1, NULL);
                pthread_join(t_ch2, NULL);
                pthread_join(t_chb, NULL);

                sem_destroy(&empty_milk);
                sem_destroy(&empty_cheese);
                sem_destroy(&mutex_milk);
                sem_destroy(&mutex_cheese);
                sem_destroy(&full_milk);
                sem_destroy(&full_cheese);

                fill (buffer_milk, buffer_milk + mMAX, 0);
                fill (buffer_cheese, buffer_cheese + chMAX, 0);
                
                milk_write_idx = 0, milk_read_idx = 0, cheese_write_idx = 0, cheese_read_idx = 0;
                milk = 0, cheese = 0, cheeseburgers = 0;
            }
            else
            {
                cout << "Please enter a whole number greater than 0." << endl;
            }
        }
    }

    
    
    return 0;
    
}