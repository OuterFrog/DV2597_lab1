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
    init_matrix();
    matmul_par();
    //print_matrix();
}
