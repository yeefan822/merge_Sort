/*
    The Hybrid Merge Sort to use for Operating Systems Assignment 1 2021
    written by Robert Sheehan

    Modified by: Jeff Peng
    UPI: zpen741

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

 //#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/times.h>
#include <math.h>
#include </usr/include/linux/fcntl.h>
#define _GNU_SOURCE

#define SIZE    24
#define MAX     1000
#define SPLIT   16


struct block {
    int size;
    int* data;
};

void print_data(struct block* block) {
    for (int i = 0; i < block->size; ++i)
        printf("%d ", block->data[i]);
    printf("\n");
}

/* The insertion sort for smaller halves. */
void insertion_sort(struct block* block) {
    for (int i = 1; i < block->size; ++i) {
        for (int j = i; j > 0; --j) {
            if (block->data[j - 1] > block->data[j]) {
                int temp;
                temp = block->data[j - 1];
                block->data[j - 1] = block->data[j];
                block->data[j] = temp;
            }
        }
    }
}

/* Combine the two halves back together. */
void merge(struct block* left, struct block* right) {
    int* combined = calloc(left->size + right->size, sizeof(int));
    if (combined == NULL) {
        perror("Allocating space for merge.\n");
        exit(EXIT_FAILURE);
    }
    int dest = 0, l = 0, r = 0;
    while (l < left->size && r < right->size) {
        if (left->data[l] < right->data[r])
            combined[dest++] = left->data[l++];
        else
            combined[dest++] = right->data[r++];
    }
    while (l < left->size)
        combined[dest++] = left->data[l++];
    while (r < right->size)
        combined[dest++] = right->data[r++];
    memmove(left->data, combined, (left->size + right->size) * sizeof(int));
    free(combined);
}

/* Merge sort the data. */
void merge_sort(struct block* block) {
    if (block->size > SPLIT) {
        struct block left_block;
        struct block right_block;
        left_block.size = block->size / 2;
        left_block.data = block->data;
        right_block.size = block->size - left_block.size; // left_block.size + (block->size % 2);
        right_block.data = block->data + left_block.size;
        merge_sort(&left_block);
        merge_sort(&right_block);
        merge(&left_block, &right_block);
    }
    else {
        insertion_sort(block);
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block* block) {
    bool sorted = true;
    for (int i = 0; i < block->size - 1; i++) {
        if (block->data[i] > block->data[i + 1])
            sorted = false;
    }
    return sorted;
}

bool array_is_sorted(int* data, int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block* block) {
    srand(1); // the same random data seed every time
    for (int i = 0; i < block->size; i++) {
        block->data[i] = rand() % MAX;
    }
}

int main(int argc, char* argv[]) {
    long size;

    if (argc < 2) {
        size = SIZE;
    }
    else {
        size = atol(argv[1]);
    }
    struct block block;
    struct block first_block;
    struct block second_block, temp_block;
    block.size = (int)pow(2, size);
    block.data = (int*)calloc(block.size, sizeof(int));
    if (block.data == NULL) {
        perror("Unable to allocate space for data.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(&block);



    struct timeval start_wall_time, finish_wall_time, wall_time;
    struct tms start_times, finish_times;
    gettimeofday(&start_wall_time, NULL);
    times(&start_times);
    //Beginning by peng
    if (block.size > SPLIT) {

        first_block.size = block.size / 2;
        first_block.data = block.data;
        second_block.size = block.size - first_block.size; //
        second_block.data = block.data + second_block.size;

        int number = 0;

        int my_pipe[2];

        if (pipe(my_pipe) == -1) {
            perror("Creating pipe for data exchanging!");
            exit(EXIT_FAILURE);
        }

        pid_t cpid;
        if ((cpid = fork()) < 0) {
            perror("Fail to create process!");
            exit(EXIT_FAILURE);
        }
        else if (cpid == 0) {
            close(my_pipe[0]);

            merge_sort(&second_block);
           // printf(is_sorted(&second_block) ? "sorted\n" : "not sorted\n");

            if (second_block.size > 32767)
            {
               // printf("hallo\n");
                int chunk_size = 16384;
                int* chunk = NULL;
                int chunk_number = second_block.size / 16384;
                for (int i = 0; i < chunk_number; i++)
                {

                    chunk = second_block.data + i * chunk_size;
                    int number = write(my_pipe[1], chunk, chunk_size * sizeof(int));
                    // printf(array_is_sorted(chunk,chunk_size) ? "array is sorted\n" : "array is not sorted\n");

                     //usleep(100);

                }

            }
            else {
                int number = write(my_pipe[1], second_block.data, second_block.size * sizeof(int));

            }
            close(my_pipe[1]);
            exit(1);

        }
        else {
            merge_sort(&first_block);
            close(my_pipe[1]);
            if (second_block.size > 32767)
            {
                //printf("second block size is %d\n",second_block.size);
                int chunk_number = second_block.size / 16384;
                int chunk_size = 16384;
                int* chunk;
                for (int i = 0; i < chunk_number; i++)
                {
                    chunk = second_block.data + i * chunk_size;
                    int number = read(my_pipe[0], chunk, chunk_size * sizeof(int));
                }
                second_block.size = chunk_number * chunk_size;
                //printf("second block size is %d\n",second_block.size);
                //printf(is_sorted(&second_block) ? "sorted1\n" : "not sorted1\n");

            }
            else {
                int number = read(my_pipe[0], second_block.data, second_block.size * sizeof(int));
            }

            close(my_pipe[0]);

            //wait(0);

            merge(&first_block, &second_block);
            if (waitpid(cpid, 0, NULL) != -1) {
                printf("wait success\n");
            }
        }

    }
    else {
        insertion_sort(&block);
    }
    //ending by peng
    //merge_sort(&block);

    gettimeofday(&finish_wall_time, NULL);
    times(&finish_times);
    timersub(&finish_wall_time, &start_wall_time, &wall_time);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    printf("wall time %ld secs and %ld microseconds\n", wall_time.tv_sec, wall_time.tv_usec);

    if (block.size < 1025)
        print_data(&block);

    printf(is_sorted(&second_block) ? "sorted\n" : "not sorted\n");
    free(block.data);
    exit(EXIT_SUCCESS);
}
