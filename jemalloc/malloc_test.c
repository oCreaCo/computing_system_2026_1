#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MEMORY_BLOCK_SIZE       (16)
#define NUM_BLOCK               (67108864) // 64M

int block_per_thread;

void *thread_func(void *arg)
{
    for (int i = 0; i < block_per_thread; i++)
        malloc(MEMORY_BLOCK_SIZE);

    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    int num_threads = atoi(argv[1]);

    block_per_thread = NUM_BLOCK / num_threads;

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], 0, thread_func, NULL);

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    free(threads);

    return 0;
}
