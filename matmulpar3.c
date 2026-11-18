/***************************************************************************
 *
 * Parallel version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 2048

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

#define CORES 8

static void
init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
        {
            /* Simple initialization, which enables us to easy check
             * the correct answer. Each element in c will have the same
             * value as SIZE after the matmul operation.
             */
            a[i][j] = 1.0;
            b[i][j] = 1.0;
        }
}

void *init_child(void *buf)
{
    unsigned long start_row = (unsigned long)buf;
    int i, j, k;

    int threads_todo = SIZE / CORES;
    for (i = start_row; i < start_row + threads_todo; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            /* Simple initialization, which enables us to easy check
             * the correct answer. Each element in c will have the same
             * value as SIZE after the matmul operation.
             */
            a[i][j] = 1.0;
            b[i][j] = 1.0;
        }
    }

    return NULL;
}

static void
init_matrix_par(void)
{
    pthread_t *children;
    children = malloc(CORES * sizeof(pthread_t));

    unsigned long i;
    for (i = 0; i < CORES; i++)
    {
        pthread_create(&(children[i - 1]), NULL, init_child, (void *)i);
    }

    for (i = 0; i < CORES; i++)
    {
        pthread_join(children[i - 1], NULL);
    }
}

void *child(void *buf)
{
    unsigned long start_row = (unsigned long)buf;
    int i, j, k;

    int threads_todo = SIZE / CORES;
    for (i = start_row; i < start_row + threads_todo; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }


    return NULL;
}

static void
matmul_par()
{
    pthread_t *children;
    children = malloc(CORES * sizeof(pthread_t));

    unsigned long i = 0;
    for (i = 0; i < CORES; i++)
    {
        pthread_create(&(children[i - 1]), NULL, child, (void *)i);
    }

    for (i = 0; i < CORES; i++)
    {
        pthread_join(children[i - 1], NULL);
    }
}

static void
print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
            printf(" %7.2f", c[i][j]);
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    init_matrix_par();
    matmul_par();
    // print_matrix();
}

// execution time with 8 cores and not init parallelized: real    0m40.902s
// exection time with 8 cores and init parallelized: real    0m7.024s
