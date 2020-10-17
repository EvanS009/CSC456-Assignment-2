#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>


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
    
    while (milk < *(int*)args->total)
    {
        sem_wait(&empty_milk);
        sem_wait(&mutex_milk);
        // critical section
        buffer_milk[milk_write_idx] = *(int*)args->pid;
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
}

void *cheese_producer(void* args)
{
    int[3] mid = {0};
    int cheese_id;

    while (cheese < *(int*)args->total)
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
        sem_wait(&mutex_milk);
        sem_wait(&empty_milk);

        cheese_id = mid[0]*1000 + mid[1]*100 + mid[2]*10 + *(int*)args->pid;

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
}

void *cheeseburger_producer(void* arg)
{
    int[2] chid = {0};
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

        burger_id = chid[0]*10000 + chid[1];
        cheeseburgers++;
        cout << burger_id << endl;
    }
}


int main(int argc, char** argv)
{
    sem_init(&empty_milk, 0, mMAX);
    sem_init(&empty_cheese, 0, chMAX);
    sem_init(&mutex_milk, 0, 1);
    sem_init(&mutex_cheese, 0, 1);
    sem_init(&full_milk, 0, 0);
    sem_init(&full_cheese, 0, 0);

    

    int total_cheeseburgers = argv[1];
    int total_cheese = 2 * total_cheeseburgers;
    int total_milk = 3 * total_cheese;

    thread_args[5] = {{1, total_milk},{2, total_milk},{3, total_milk},{4, total_cheese},{5, total_cheese}}

    pthread_t t_m1, t_m2, t_m3;
    pthread_t t_ch1, t_ch2;
    pthread_t t_chb;

    pthread_create(&t_m1, NULL, milk_producer, (void*)&thread_args[1]);
    pthread_create(&t_m2, NULL, milk_producer, (void*)&thread_args[2]);
    pthread_create(&t_m3, NULL, milk_producer, (void*)&thread_args[3]);
    pthread_create(&t_ch1, NULL, cheese_producer, (void*)&thread_args[4]);
    pthread_create(&t_ch2, NULL, cheese_producer, (void*)&thread_args[5]);
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
    sem_destroy(&full_cheese;

    return 0;
    
}