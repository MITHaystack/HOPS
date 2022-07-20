/*
 * Test code to simulate difx2mark4 usage of root_id()
 * Pulled from difx2mark4.c:newScan()
 */
#include <time.h>

char *make_id(void)
{
    static int first = 1;
    static int year, day, hour, min, sec;
    time_t now;
    char *rcode;
    
    if (first) {
        now = time ((time_t *) NULL);
        t = localtime (&now);
        year = t->tm_year;
        day = t->tm_yday+1;
        hour = t->tm_hour;
        min = t->tm_min;
        sec = t->tm_sec;
        first = 0;
    } else {
        sec += 4;
    }

    rcode = root_id (year, day, hour, min, sec);
    return(rcode);
}

/*
 * eof
 */
