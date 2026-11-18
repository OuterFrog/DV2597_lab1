/***************************************************************************
 *
 * false_sharing.c
 *
 * A simple program to show the performance impact of false sharing
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define LOOPS (2000 * 1000000)

volatile static int a, b, c;

static void *
inc_a(void *arg)
{
    int i;
    int a;
    printf("Create inc_a\n");
    while (i++ < LOOPS)
        a++;
    pthread_exit(0);
}

static void *
inc_b(void *arg)
{
    int i;
    int b;
    printf("Create inc_b\n");
    while (i++ < LOOPS)
        b++;
    pthread_exit(0);
}

static void *
inc_c(void *arg)
{
    int i;
    int c;
    printf("Create inc_c\n");
    while (i++ < LOOPS)
        c++;
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int rc, t;
    pthread_t tid_a, tid_b, tid_c;

    rc = pthread_create(&tid_a, NULL, inc_a, (void *)t);
    rc = pthread_create(&tid_b, NULL, inc_b, (void *)t);
    rc = pthread_create(&tid_c, NULL, inc_c, (void *)t);
    pthread_join(tid_a, NULL);
    pthread_join(tid_b, NULL);
    pthread_join(tid_c, NULL);
}

// time all global: real    0m29.384s
// time a local: real    0m12.238s
// time a, b local: real    0m4.133s
// time all local: real    0m6.549s

// a and b were probably located in the same memory block
// c didn't change it
// however, this was only the last test i did
// for the first tests i did, c did make a significant difference times were more like 24 -> 15 -> 9 -> 4