/*
    The Hybrid Merge Sort to use for Operating Systems Assignment 1 2021
    written by Robert Sheehan

    Modified by: Jeff Peng
    UPI: zpen741

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/times.h>
#include <math.h>
#include<pthread.h>

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
    if (block.size >= (1024 * 8)) {
        struct block sub_block[8];
        int thread_creation;
        pthread_t t[7];
        for (int i = 0; i < 7; i++) {
            sub_block[i].size = block.size / 8;
            sub_block[i].data = block.data + i * sub_block[i].size;
            
            thread_creation = pthread_create(&t[i], NULL, (void*)&merge_sort, &sub_block[i]);
            if (thread_creation) {
                printf("ERROR; reutrn code from pthread_create() is %d\n", thread_creation);
                exit(EXIT_FAILURE);
            }
            

        }
        sub_block[7].size = block.size / 8;
        sub_block[7].data = block.data + 7 * sub_block[7].size;
        merge_sort(&sub_block[7]);
        for (int i = 0; i < 7; i++) {
            pthread_join(t[i], NULL);
            
        }
         for (int i = 1; i <= 7; i++) {
             merge(&sub_block[0], &sub_block[i]);
              sub_block[0].size += sub_block[i].size;
  //
          }

    }
    else {
        merge_sort(&block);
    }
    gettimeofday(&finish_wall_time, NULL);
    times(&finish_times);
    timersub(&finish_wall_time, &start_wall_time, &wall_time);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    printf("wall time %ld secs and %ld microseconds\n", wall_time.tv_sec, wall_time.tv_usec);

    if (block.size < 1025)
        print_data(&block);

    printf(is_sorted(&block) ? "sorted\n" : "not sorted\n");
    free(block.data);
    exit(EXIT_SUCCESS);
}
