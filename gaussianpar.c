/***************************************************************************
 *
 * Sequential version of Gaussian elimination
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define MAX_SIZE 4096
#define THREAD_COUNT 8
#define ROWS_PER_CYCLE 64

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct thread_args
{
    unsigned int id;
    unsigned int k;
};

typedef double matrix[MAX_SIZE][MAX_SIZE];

int N;              /* matrix size		*/
int maxnum;         /* max number of element*/
char *Init;         /* matrix init type	*/
int PRINT;          /* print switch		*/
matrix A;           /* matrix A		*/
double b[MAX_SIZE]; /* vector b             */
double y[MAX_SIZE]; /* vector y             */

unsigned int global_rows_per_thread = MAX_SIZE / THREAD_COUNT;
unsigned int num_threads;

unsigned int init_num_threads;

/* forward declarations */
void work(void);
void Init_Matrix(void);
void Init_Matrix_Par(void);
void Print_Matrix(void);
void Init_Default(void);
int Read_Options(int, char **);


int main(int argc, char **argv)
{
    int i, timestart, timeend, iter;

    Init_Default();           /* Init default values	*/
    Read_Options(argc, argv); /* Read arguments	*/
    Init_Matrix_Par();            /* Init the matrix	*/
    work();
    if (PRINT == 1)
        Print_Matrix();
}

inline void thread_work_cyclic(const unsigned int id, const unsigned int k)
{
    // cyclic
    for (int i = k + 1 + id; i < N; i += THREAD_COUNT)
    {
        for (int j = k + 1; j < N; j++)
            A[i][j] = A[i][j] - A[i][k] * A[k][j]; /* Elimination step */
        b[i] = b[i] - A[i][k] * y[k];
        A[i][k] = 0.0;
    }
}

inline void thread_work_cyclic_block(const unsigned int id, const unsigned int k)
{
    // block-cyclic
    for (int i = k + 1 + id * ROWS_PER_CYCLE; i < N; i += THREAD_COUNT * ROWS_PER_CYCLE)
    {
        for (int offset = 0; offset < ROWS_PER_CYCLE && i + offset < N; offset++)
        {
            int row = i + offset;

            for (int j = k + 1; j < N; j++)
                A[row][j] = A[row][j] - A[row][k] * A[k][j]; /* Elimination step */
            b[row] = b[row] - A[row][k] * y[k];
            A[row][k] = 0.0;
        }
    }
}

inline void thread_work_block(const unsigned int id, const unsigned int k)
{
    // block-based

    int rows_left = N - k - 1;
    int rows_per_thread = (rows_left) / THREAD_COUNT;
    int start_row = (k + 1) + rows_per_thread * id;
    int end_row = id == THREAD_COUNT - 1 ? N : (k + 1) + rows_per_thread * (id + 1);

    for (int i = start_row; i < end_row; i++)
    {
        for (int j = k + 1; j < N; j++)
            A[i][j] = A[i][j] - A[i][k] * A[k][j]; /* Elimination step */
        b[i] = b[i] - A[i][k] * y[k];
        A[i][k] = 0.0;
    }
}

void* thread_work(void* buf)
{
    struct thread_args *args = (struct thread_args *)buf;
    unsigned int id = args->id;
    unsigned int k = args->k;

    //rows_per_thread = N / THREAD_COUNT;
    //thread_work_block(id, k);

    //thread_work_cyclic(id, k);

    thread_work_cyclic_block(id, k);

    return NULL;
}

void work(void)
{
    int i, j, k;
    unsigned long id;

    pthread_t *threads;
    threads = malloc(THREAD_COUNT * sizeof(pthread_t));

    struct thread_args* args;
    args = malloc(THREAD_COUNT * sizeof(struct thread_args));

    /* Gaussian elimination algorithm, Algo 8.4 from Grama */
    for (k = 0; k < N; k++)
    { /* Outer loop */
        for (j = k + 1; j < N; j++)
            A[k][j] = A[k][j] / A[k][k]; /* Division step */
        y[k] = b[k] / A[k][k];
        A[k][k] = 1.0;

        if(N - k > ROWS_PER_CYCLE * THREAD_COUNT / 4){
            // num_threads = THREAD_COUNT; // MIN(, (N - k) / 64);

            for (id = 0; id < THREAD_COUNT; id++)
            {
                args[id].id = id;
                args[id].k = k;
                pthread_create(&(threads[id]), NULL, thread_work, (void *)&args[id]);
            }

            for (id = 0; id < THREAD_COUNT; id++)
            {
                pthread_join(threads[id], NULL);
            }
        }
        else
        {
            // do sequential instead
            for (i = k + 1; i < N; i++)
            {
                for (j = k + 1; j < N; j++)
                    A[i][j] = A[i][j] - A[i][k] * A[k][j]; /* Elimination step */
                b[i] = b[i] - A[i][k] * y[k];
                A[i][k] = 0.0;
            }
        }

    }

    free(threads);
    free(args);
}

void *init_multithreaded_fast(void *buf)
{
    unsigned long row = (unsigned long)buf;

    assert(N % init_num_threads == 0);
    int per_thread = N / init_num_threads;

    for (int i = row * per_thread; i < (row +  1) * per_thread; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i == j) /* diagonal dominance */
                A[i][j] = 5.0;
            else
                A[i][j] = 2.0;
        }
    }

    return NULL;
}

void *init_multithreaded_rand(void *buf)
{
    unsigned long row = (unsigned long)buf;

    assert(N % init_num_threads == 0);
    int per_thread = N / init_num_threads;

    for (int i = row * per_thread; i < (row + 1) * per_thread; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i == j) /* diagonal dominance */
                A[i][j] = (double)(rand() % maxnum) + 5.0;
            else
                A[i][j] = (double)(rand() % maxnum) + 1.0;
        }
    }

    return NULL;
}

void Init_Matrix_Par()
{
    int i, j;

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init, "rand") == 0)
    {
        init_num_threads = N < 512 ? 1 : 2;
        pthread_t *threads;
        threads = malloc(init_num_threads * sizeof(pthread_t));

        unsigned long id;
        for (id = 0; id < init_num_threads; id++)
        {
            pthread_create(&(threads[id]), NULL, init_multithreaded_rand, (void *)id);
        }

        for (id = 0; id < init_num_threads; id++)
        {
            pthread_join(threads[id], NULL);
        }

        free(threads);
    }
    if (strcmp(Init, "fast") == 0)
    {
        init_num_threads = N < 512 ? 1 : 8;
        pthread_t *threads;
        threads = malloc(init_num_threads * sizeof(pthread_t));

        unsigned long id;
        for (id = 0; id < init_num_threads; id++)
        {
            pthread_create(&(threads[id]), NULL, init_multithreaded_fast, (void *)id);
        }

        for (id = 0; id < init_num_threads; id++)
        {
            pthread_join(threads[id], NULL);
        }

        free(threads);
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++)
    {
        b[i] = 2.0;
        y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
        Print_Matrix();
}

void Init_Matrix()
{
    int i, j;

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init, "rand") == 0)
    {
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (i == j) /* diagonal dominance */
                    A[i][j] = (double)(rand() % maxnum) + 5.0;
                else
                    A[i][j] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0)
    {
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (i == j) /* diagonal dominance */
                    A[i][j] = 5.0;
                else
                    A[i][j] = 2.0;
            }
        }
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++)
    {
        b[i] = 2.0;
        y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
        Print_Matrix();
}

void Print_Matrix()
{
    int i, j;

    printf("Matrix A:\n");
    for (i = 0; i < N; i++)
    {
        printf("[");
        for (j = 0; j < N; j++)
            printf(" %5.2f,", A[i][j]);
        printf("]\n");
    }
    printf("Vector b:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", b[j]);
    printf("]\n");
    printf("Vector y:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", y[j]);
    printf("]\n");
    printf("\n\n");
}

void Init_Default()
{
    N = 2048;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
}

int Read_Options(int argc, char **argv)
{
    char *prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++*argv)
            {
            case 'n':
                --argc;
                N = atoi(*++argv);
                break;
            case 'h':
                printf("\nHELP: try sor -u \n\n");
                exit(0);
                break;
            case 'u':
                printf("\nUsage: gaussian [-n problemsize]\n");
                printf("           [-D] show default values \n");
                printf("           [-h] help \n");
                printf("           [-I init_type] fast/rand \n");
                printf("           [-m maxnum] max random no \n");
                printf("           [-P print_switch] 0/1 \n");
                exit(0);
                break;
            case 'D':
                printf("\nDefault:  n         = %d ", N);
                printf("\n          Init      = rand");
                printf("\n          maxnum    = 5 ");
                printf("\n          P         = 0 \n\n");
                exit(0);
                break;
            case 'I':
                --argc;
                Init = *++argv;
                break;
            case 'm':
                --argc;
                maxnum = atoi(*++argv);
                break;
            case 'P':
                --argc;
                PRINT = atoi(*++argv);
                break;
            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -u \n\n", prog);
                break;
            }
}
