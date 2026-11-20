/***************************************************************************
 *
 * Sequential version of Quick sort
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>

#include "qsortcommon.h"

struct thread_args
{
    int *v;
    unsigned low;
    unsigned high;
    unsigned depth;
};

#define KILO (1024)
#define MEGA (1024 * 1024)
#define MAX_ITEMS (64 * MEGA)
#define swap(v, a, b) \
    {                 \
        unsigned tmp; \
        tmp = v[a];   \
        v[a] = v[b];  \
        v[b] = tmp;   \
    }

static int *v;

#define MAX_THREADS 8192

static void
print_array(void)
{
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
        printf("%d ", v[i]);
    printf("\n");
}

static void
init_array(void)
{
    int i;
    v = (int *)malloc(MAX_ITEMS * sizeof(int));
    for (i = 0; i < MAX_ITEMS; i++)
        v[i] = rand();
}

static unsigned
partition(int *v, unsigned low, unsigned high, unsigned pivot_index)
{
    /* move pivot to the bottom of the vector */
    if (pivot_index != low)
        swap(v, low, pivot_index);

    pivot_index = low;
    low++;

    /* invariant:
     * v[i] for i less than low are less than or equal to pivot
     * v[i] for i greater than high are greater than pivot
     */

    /* move elements into place */
    while (low <= high)
    {
        if (v[low] <= v[pivot_index])
            low++;
        else if (v[high] > v[pivot_index])
            high--;
        else
            swap(v, low, high);
    }

    /* put pivot back between two groups */
    if (high != pivot_index)
        swap(v, pivot_index, high);
    return high;
}

static void
quick_sort(int *v, unsigned low, unsigned high)
{
    unsigned pivot_index;

    /* no need to sort a vector of zero or one element */
    if (low >= high)
        return;

    /* select the pivot value */
    pivot_index = (low + high) / 2;

    /* partition the vector */
    pivot_index = partition(v, low, high, pivot_index);

    /* sort the two sub arrays */
    if (low < pivot_index)
        quick_sort(v, low, pivot_index - 1);
    if (pivot_index < high)
        quick_sort(v, pivot_index + 1, high);
}

void *thread_quicksort(void *buf)
{
    struct thread_args *args = (struct thread_args *)buf;
    int *v = args->v;
    unsigned low = args->low;
    unsigned high = args->high;
    unsigned depth = args->depth;

    unsigned pivot_index;

    /* no need to sort a vector of zero or one element */
    if (low >= high)
        return NULL;

    /* select the pivot value */
    pivot_index = (low + high) / 2;

    /* partition the vector */
    pivot_index = partition(v, low, high, pivot_index);

    /* sort the two sub arrays */
    if (pow(2, depth + 1) - 1 + 2 < MAX_THREADS) // calculate if it can create two more threads without going over budget
    {
        pthread_t threads[2];
        struct thread_args args[2];

        // create threads
        if (low < pivot_index)
        {
            args[0].v = v;
            args[0].low = low;
            args[0].high = pivot_index - 1;
            args[0].depth = depth + 1;

            pthread_create(&(threads[0]), NULL, thread_quicksort, (void *)&args[0]);
        }

        if (pivot_index < high)
        {
            args[1].v = v;
            args[1].low = pivot_index + 1;
            args[1].high = high;
            args[1].depth = depth + 1;

            //thread_quicksort(&args[1]);
            pthread_create(&(threads[1]), NULL, thread_quicksort, (void *)&args[1]);
        }

        // join threads
        if (low < pivot_index)
        {
            pthread_join(threads[0], NULL);
        }
        if (pivot_index < high)
        {
            pthread_join(threads[1], NULL);
        }
        
    }
    else
    {
        // sequential instead
        if (low < pivot_index)
            quick_sort(v, low, pivot_index - 1);
        if (pivot_index < high)
            quick_sort(v, pivot_index + 1, high);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    init_array();
    // print_array();

    double startTime = get_time();
    struct thread_args args;
    args.v = v;
    args.low = 0;
    args.high = MAX_ITEMS - 1;
    args.depth = 0;
    thread_quicksort((void*)&args);
    double endTime = get_time();

    // calculate work time
    printf("Parallel real time: %f\n", endTime - startTime);

    if (check_sort(v, MAX_ITEMS) != 0)
    {
        printf("failed to sort\n");
    }
    else
    {
        printf("Sorted\n");
    }

    // print_array();
}
