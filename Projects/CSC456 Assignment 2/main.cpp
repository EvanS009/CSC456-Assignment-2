#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

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
int milk_read_idk = 0;
int cheese_write_idx = 0;
int cheese_read_idx = 0;


int total_cheeseburgers = 0;

int main(int argc, char** argv)
{
    sem_init(&empty_milk, 0, mMAX);
    sem_init(&empty_cheese, 0, chMAX);
    sem_init(&mutex_milk, 0, 1);
    sem_init(&mutex_cheese, 0, 1);
    sem_init(&full_milk, 0, 0);
    sem_init(&full_cheese, 0, 0);

    int pid[5] = {1, 2, 3, 4, 5};

    pthread_t t_m1, t_m2, t_m3;
    pthread_t t_ch1, t_ch2;
    pthread_t t_chb;

    pthread_create(&t_m1, NULL, milk_producer, (void*)&pid[0]);
    pthread_create(&t_m2, NULL, milk_producer, (void*)&pid[1]);
    pthread_create(&t_m3, NULL, milk_producer, (void*)&pid[2]);
    pthread_create(&t_ch1, NULL, cheese_producer, (void*)&pid[3]);
    pthread_create(&t_ch2, NULL, cheese_producer, (void*)&pid[4]);
    pthread_create(&t_chb, NULL, cheeseburger_producer, (void*)&argv[1]);

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