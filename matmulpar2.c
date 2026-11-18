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

void *init_child(void *buf)
{
    unsigned long row = (unsigned long)buf;

    for (int j = 0; j < SIZE; j++)
    {
        /* Simple initialization, which enables us to easy check
         * the correct answer. Each element in c will have the same
         * value as SIZE after the matmul operation.
         */
        a[row][j] = 1.0;
        b[row][j] = 1.0;
    }

    return NULL;
}

static void
init_matrix_par(void)
{
    pthread_t *children;
    children = malloc(SIZE * sizeof(pthread_t));

    unsigned long i;
    for (i = 0; i < SIZE; i++)
    {
        pthread_create(&(children[i - 1]), NULL, init_child, (void *)i);
    }

    for (i = 0; i < SIZE; i++)
    {
        pthread_join(children[i - 1], NULL);
    }
}

void *child(void *buf)
{
    unsigned long row = (unsigned long)buf;
    int j, k;

    for (j = 0; j < SIZE; j++)
    {
        c[row][j] = 0.0;
        for (k = 0; k < SIZE; k++)
            c[row][j] = c[row][j] + a[row][k] * b[k][j];
    }

    return NULL;
}

static void
matmul_par()
{
    pthread_t *children;
    children = malloc(SIZE * sizeof(pthread_t));

    unsigned long i = 0;
    for (i = 0; i < SIZE; i++)
    {
        pthread_create(&(children[i - 1]), NULL, child, (void *)i);
    }

    for (i = 0; i < SIZE; i++)
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

// before:
// real 0m41.899s user 4m40.989s sys 0m2.506s

// now:
// real 0m41.424s user 4m39.071s sys 0m2.007s
