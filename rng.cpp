/* From Project Instruction */

#include <time.h>	
#include <stdlib.h>
#include <math.h>

#define round(X) (((X)>= 0)?(int)((X)+0.5):(int)((X)-0.5))

void InitRandom(long l_seed)
    /*
     * initialize the random number generator
     * if l_seed is 0, will seed the random number generator using the current clock
     * if l_seed is NOT 0, your simulation should be repeatable if you use the same l_seed
     * call this function at the beginning of main() and only once
     */
{
    if (l_seed == 0L) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        srand48(ts.tv_nsec);
    } else {
        srand48(l_seed);
    }
}

int ExponentialInterval(double rate)
    /*
     * returns an exponentially distributed random interval (in milliseconds) with mean 1/rate
     * rate is either lambda or mu (in packets/second)
     * according to the spec: w = ln(1-r) / (-rate)
     */
{
    double dval;
    do {
        dval = drand48();
        /* if dval is too small or too large, try again */
    } while (dval < 0.000001 || dval > 0.999999);

    dval = ((double)1.0) - dval;
    dval = -log(dval);
    dval = ((dval / rate) * ((double)1000));

    return round(dval);
}

