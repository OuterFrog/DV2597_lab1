#ifndef QSORTCOMMON_H_
#define QSORTCOMMON_H_

#include <time.h>

static double get_time()
{
    struct timespec time_spec;
    int success = clock_gettime(CLOCK_REALTIME, &time_spec);
    return time_spec.tv_sec + time_spec.tv_nsec / 1000000000.0; // convert to seconds
}
#endif // QSORTCOMMON_H_